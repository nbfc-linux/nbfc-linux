#include "server.h"

#include "nbfc.h"
#include "nxjson_utils.h"
#include "reverse_nxjson.h"
#include "error.h"
#include "service.h"
#include "log.h"
#include "quit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

static int                Server_FD = -1;
static struct sockaddr_in Server_Address;
static pthread_t          Server_Thread_ID;
static const int          Server_Max_Failures = 100;

static void* Server_Run(void*);

static Error* Server_Command_Set_Fan(int socket, const nx_json* json) {
  int fan = -1;
  float speed = -1;
  Boolean auto_mode = Boolean_Unset;
  const int fancount = Service_Model_Config.FanConfigurations.size;

  nx_json_for_each(c, json) {
    if (!strcmp(c->key, "command"))
      continue;
    else if (!strcmp(c->key, "fan")) {
      if (c->type != NX_JSON_INTEGER)
        return err_string(0, "fan: not an integer");

      fan = c->val.i;

      if (fan < 0)
        return err_string(0, "fan: cannot be negative");
      else if (fan >= fancount)
        return err_string(0, "fan: no such fan available");
    }
    else if (!strcmp(c->key, "speed")) {
      if (c->type == NX_JSON_STRING && !strcmp(c->val.text, "auto")) {
        auto_mode = true;
        continue;
      }
      else if (c->type == NX_JSON_DOUBLE)
        speed = c->val.dbl;
      else if (c->type == NX_JSON_INTEGER)
        speed = c->val.i;
      else {
        return err_string(0, "speed: Invalid type. Either float or 'auto'");
      }

      if (speed < 0.0 || speed > 100.0)
        return err_string(0, "speed: Invalid value");
    }
    else {
      return err_string(0, "Unknown arguments");
    }
  }

  if (speed == -1 && auto_mode == Boolean_Unset)
    return err_string(0, "Missing argument: speed");

  float* speeds = Mem_Malloc(sizeof(float) * fancount);

  for (int i = 0; i < fancount; ++i)
    speeds[i] = -1;
  for (int i = 0; i < min(service_config.TargetFanSpeeds.size, fancount); ++i)
    speeds[i] = service_config.TargetFanSpeeds.data[i];

  if (fan == -1) {
    for (int i = 0; i < fancount; ++i)
      speeds[i] = (auto_mode == true) ? -1 : speed;
  }
  else {
    speeds[fan] = (auto_mode == true) ? -1 : speed;
  }

  free(service_config.TargetFanSpeeds.data);
  service_config.TargetFanSpeeds.data = speeds;
  service_config.TargetFanSpeeds.size = fancount;

  Service_UpdateFanSpeedsByTargetFanSpeeds();
  kill(getpid(), SIGUSR1);

  Error* e = ServiceConfig_Write();
  if (e)
    return e;

  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);
  create_json_string("status", o, "OK");

  e = Protocol_Send_Json(socket, o);
  nx_json_free(o);
  return e;
}

static Error* Server_Command_Status(int socket, const nx_json* json) {
  Error* e;

  nx_json_for_each(c, json) {
    if (!strcmp(c->key, "command"))
      continue;
    else
      return err_string(0, "Unknown arguments");
  }

  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);
  create_json_integer("pid", o, getpid());
  create_json_string("config", o, Service_Model_Config.NotebookModel);
  create_json_bool("read-only", o, options.read_only);
  create_json_double("temperature", o, Service_Temperature);
  nx_json* fans = create_json_array("fans", o);

  for_each_array(Fan*, fan, Service_Fans) {
    nx_json* fan_json = create_json_object(NULL, fans);
    create_json_string("name", fan_json, fan->fanConfig->FanDisplayName);
    create_json_bool("automode", fan_json, (fan->mode == Fan_ModeAuto));
    create_json_bool("critical", fan_json, fan->isCritical);
    create_json_double("current_speed", fan_json, Fan_GetCurrentSpeed(fan));
    create_json_double("target_speed", fan_json, Fan_GetTargetSpeed(fan));
    create_json_integer("speed_steps", fan_json, Fan_GetSpeedSteps(fan));
  }

  e = Protocol_Send_Json(socket, o);
  nx_json_free(o);
  return e;
}

static void *Server_Handle_Client(void *arg) {
  const int socket = (int) (intptr_t) arg;
  const nx_json* json = NULL;
  char* buf = NULL;
  Error* e;

  e = Protocol_Receive_Json(socket, &buf, &json);
  if (e)
    goto error;

  if (json->type != NX_JSON_OBJECT) {
    e = err_string(0, "Not a JSON object");
    goto error;
  }

  const nx_json* command = nx_json_get(json, "command");
  if (! command) {
    e = err_string(0, "Missing 'command' field");
    goto error;
  }

  if (command->type != NX_JSON_STRING) {
    e = err_string(0, "command: not a string");
    goto error;
  }

  pthread_mutex_lock(&Service_Lock);
  {
    if (!strcmp(command->val.text, "set-fan-speed"))
      e = Server_Command_Set_Fan(socket, json);
    else if (!strcmp(command->val.text, "status"))
      e = Server_Command_Status(socket, json);
    else
      e = err_string(0, "Invalid command");
  }
  pthread_mutex_unlock(&Service_Lock);

error:
  if (e)
    Protocol_Send_Error(socket, err_print_all(e));

  if (buf)
    free(buf);

  if (json)
    nx_json_free(json);

  close(socket);
  return NULL;
}

Error* Server_Init(int port) {
  Error* e = NULL;
  int opt = 1;

  // Create a TCP/IP socket
  if ((Server_FD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    e = err_stdlib(0, "socket()");
    goto error;
  }

  if (setsockopt(Server_FD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    e = err_stdlib(0, "setsockopt()");
    goto error;
  }

  // Bind the socket to an address
  Server_Address.sin_family = AF_INET;
  Server_Address.sin_addr.s_addr = INADDR_ANY;
  Server_Address.sin_port = htons(port);

  if (bind(Server_FD, (struct sockaddr *)&Server_Address, sizeof(Server_Address)) < 0) {
    e = err_stdlib(0, "bind()");
    goto error;
  }

  // Listeon for incoming connections
  if (listen(Server_FD, 3) < 0) {
    e = err_stdlib(0, "listen()");
    goto error;
  }

  Log_Info("Connected on port %d\n", ntohs(Server_Address.sin_port));

error:
  if (e)
    Server_Close();

  return e;
}

Error* Server_Start() {
  if (pthread_create(&Server_Thread_ID, NULL, Server_Run, NULL) != 0) {
    return err_stdlib(0, "pthread_create()");
  }

  pthread_detach(Server_Thread_ID);
  return err_success();
}

void Server_Stop() {
  pthread_kill(Server_Thread_ID, SIGTERM);
}

Error* Server_Loop() {
  int addrlen = sizeof(Server_Address);
  int new_socket;
  pthread_t thread_id;

  if ((new_socket = accept(Server_FD, (struct sockaddr*)&Server_Address, (socklen_t*)&addrlen)) < 0)
    return err_stdlib(0, "accept()");

  if (pthread_create(&thread_id, NULL, Server_Handle_Client, (void*) (intptr_t) new_socket) != 0)
    return err_stdlib(0, "pthread_create()");

  pthread_detach(thread_id);

  return err_success();
}

static void* Server_Run(void* arg) {
  int failures = 0;

  while (! quit) {
    Error* e = Server_Loop();

    // When the server stops, accept() from Server_Loop() returns
    // EBADF. So don't handle this as an error.
    if (! quit)
      e_warn();

    if (e) {
      if (++failures > Server_Max_Failures)
        return NULL;
    }
    else
      failures = 0;
  }

  return NULL;
}

void Server_Close() {
  if (Server_FD != -1)
    close(Server_FD);
  Server_FD = -1;
}
