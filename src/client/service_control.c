#include "service_control.h"

#include <errno.h>  // errno, ENOENT
#include <stdio.h>  // snprintf
#include <stdlib.h> // exit, system, WEXITSTATUS
#include <string.h> // strcat, strerror, strcspn, memset
#include <signal.h> // kill, SIGINT
#include <unistd.h> // access, F_OK, unlink
#include <limits.h> // INT_MAX

#include <sys/types.h>
#include <sys/socket.h> // connect, socket
#include <sys/un.h>     // sockaddr_un

#include "../log.h"
#include "../sleep.h"
#include "../nbfc.h"
#include "../log.h"
#include "../memory.h"
#include "../parse_number.h"
#include "../file_utils.h"
#include "../protocol.h"
#include "../nxjson_utils.h"
#include "../service_config.h"

int Service_Get_PID() {
  const char* err;
  char buf[32];
  if (slurp_file(buf, sizeof(buf), NBFC_PID_FILE) == -1) {
    if (errno == ENOENT)
      return -1;
    else {
      err = strerror(errno);
      goto error;
    }
  }

  // trim the newline
  buf[strcspn(buf, "\n")] = '\0';

  int pid = parse_number(buf, 0, INT_MAX, &err);
  if (err) {
error:
    Log_Error("Failed to read the pid file: " NBFC_PID_FILE ": %s\n", err);
    exit(NBFC_EXIT_FAILURE);
  }

  return pid;
}

Error* Client_Communicate(const nx_json* in, char** buf, const nx_json** out) {
  int sock;
  struct sockaddr_un serv_addr;
  Error* e = NULL;

  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    return err_stdlib(0, "socket()");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  snprintf(serv_addr.sun_path, sizeof(serv_addr.sun_path), NBFC_SOCKET_PATH);

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    e = err_string(0, NBFC_SOCKET_PATH);
    e = err_stdlib(e, "connect()");
    goto error;
  }

  e = Protocol_Send_Json(sock, in);
  if (e)
    goto error;

  e = Protocol_Receive_Json(sock, buf, out);
  if (e)
    goto error;

error:
  close(sock);
  return e;
}

void ServiceConfig_Load() {
  if (access(NBFC_SERVICE_CONFIG, F_OK) != 0) {
    memset(&service_config, 0, sizeof(service_config)); // Clear values
    return;
  }

  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;
  Error* e = nx_json_parse_file(&js, buf, sizeof(buf), NBFC_SERVICE_CONFIG);
  if (e)
    goto error;

  e = ServiceConfig_FromJson(&service_config, js);
  nx_json_free(js);

  if (e) {
error:
    e = err_string(e, NBFC_SERVICE_CONFIG);
    e_die();
  }
}

Error* ServiceInfo_TryLoad(ServiceInfo* service_info) {
  Error* e;
  nx_json root = {0};
  nx_json* in = create_json_object(NULL, &root);
  create_json_string("Command", in, "status");

  char* buf = NULL;
  const nx_json* out = NULL;
  e = Client_Communicate(in, &buf, &out);
  if (e)
    goto error;

  if (out->type != NX_JSON_OBJECT) {
    e = err_string(0, "Not a JSON object");
    goto error;
  }

  const nx_json* err = nx_json_get(out, "Error");
  if (err) {
    if (err->type != NX_JSON_STRING) {
      e = err_string(0, "'Error' is not a string");
      goto error;
    }

    e = err_string(0, err->val.text);
    goto error;
  }

  e = ServiceInfo_FromJson(service_info, out);
  if (e)
    goto error;

  e = ServiceInfo_ValidateFields(service_info);
  if (e)
    goto error;

  for_each_array(FanInfo*, f, service_info->Fans) {
    e = FanInfo_ValidateFields(f);
    if (e)
      goto error;
  }

error:
  nx_json_free(in);
  nx_json_free(out);
  Mem_Free(buf);

  return e;
}

void Service_LoadAllConfigFiles(ModelConfig* model_config) {
  Error* e;
  Trace trace = {0};
  char path[PATH_MAX];

  e = ServiceConfig_Init(NBFC_SERVICE_CONFIG);
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    Log_Error("This command needs a valid and configured `%s`\n", NBFC_SERVICE_CONFIG);
    exit(NBFC_EXIT_FAILURE);
  }

  e = ModelConfig_FindAndLoad(model_config, path, service_config.SelectedConfigId);
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    Log_Error("This command needs a valid model configuration (%s)\n", path);
    exit(NBFC_EXIT_FAILURE);
  }

  Trace_Push(&trace, path);
  e = ModelConfig_Validate(&trace, model_config);
  if (e) {
    Log_Error("%s: %s\n", trace.buf, err_print_all(e));
    Log_Error("This command needs a valid model configuration (%s)\n", path);
    exit(NBFC_EXIT_FAILURE);
  }
}

int Service_Start(bool read_only) {
  int pid = Service_Get_PID();
  if (pid != -1) {
    Log_Info("Service already running (pid: %d)\n", pid);
    return NBFC_EXIT_SUCCESS;
  }

  char cmd[64] = "nbfc_service -f";
  if (read_only)
    strcat(cmd, " -r");

  int ret = system(cmd);
  if (ret == -1) {
    Log_Error("Failed to start process: %s\n", strerror(errno));
    return NBFC_EXIT_FAILURE;
  }
  if (WEXITSTATUS(ret) == 127) {
    Log_Error("Can't run nbfc_service, make sure the binary is installed\n");
    return NBFC_EXIT_FAILURE;
  }

  return WEXITSTATUS(ret);
}

int Service_Stop() {
  int pid = Service_Get_PID();
  if (pid == -1) {
    Log_Error("Service not running\n");
    return NBFC_EXIT_SUCCESS;
  }

  Log_Info("Killing nbfc_service (%d)\n", pid);
  if (kill(pid, SIGINT) == -1) {
    Log_Error("Failed to kill nbfc_service process (%d): %s\n", pid, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }

  unlink(NBFC_PID_FILE);
  return NBFC_EXIT_SUCCESS;
}

int Service_Restart(bool read_only) {
  Service_Stop();
  sleep_ms(1000);
  return Service_Start(read_only);
}
