#include "server.h"

#include "nbfc.h"
#include "nxjson_utils.h"
#include "reverse_nxjson.c"
#include "error.h"
#include "service.h"
#include "log.h"

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

static const char* Json_EscapeString(char*, const size_t, const char*);
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
      if (c->type != NX_JSON_INTEGER) {
        return err_string(0, "fan: not an integer");
      }

      fan = c->val.i;

      if (fan < 0) {
        return err_string(0, "fan: cannot be negative");
      }
      else if (fan >= fancount) {
        return err_string(0, "fan: no such fan available");
      }
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

      if (speed < 0.0 || speed > 100.0) {
        return err_string(0, "speed: Invalid value");
      }
    }
		else {
      return err_string(0, "Unknown arguments");
    }
  }

  if (speed == -1 && auto_mode == Boolean_Unset) {
    return err_string(0, "Missing argument: speed");
  }

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
  nx_json *o = create_json(NX_JSON_OBJECT, NULL, &root);

  nx_json *o1 = create_json(NX_JSON_STRING, "status", o);
  o1->val.text = "OK";

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

  static const char Bool_ToStr[2][6] = {"false", "true"};
  char buf[256];
  char result[NBFC_MAX_FILE_SIZE];
  StringBuf  S = {result, 0, sizeof(result) - 1};
  StringBuf* s = &S;

  StringBuf_Printf(s, "{\n"
    "\t\"pid\":         %d,\n"
    "\t\"config\":      \"%s\",\n"
    "\t\"read-only\":   %s,\n"
    "\t\"temperature\": %.2f,\n"
    "\t\"fans\": [\n",
    getpid(),
    Json_EscapeString(buf, sizeof(buf), Service_Model_Config.NotebookModel),
    Bool_ToStr[options.read_only],
    Service_Temperature);

  size_t i = 0;
  for_each_array(Fan*, fan, Service_Fans) {
    StringBuf_Printf(s, "\t\t{\n"
      "\t\t\t\"name\":          \"%s\",\n"
      "\t\t\t\"automode\":      %s,\n"
      "\t\t\t\"critical\":      %s,\n"
      "\t\t\t\"current_speed\": %.2f,\n"
      "\t\t\t\"target_speed\":  %.2f,\n"
      "\t\t\t\"speed_steps\":   %d\n"
      "\t\t}%s\n",
      Json_EscapeString(buf, sizeof(buf), fan->fanConfig->FanDisplayName),
      Bool_ToStr[fan->mode == Fan_ModeAuto],
      Bool_ToStr[fan->isCritical],
      Fan_GetCurrentSpeed(fan),
      Fan_GetTargetSpeed(fan),
      Fan_GetSpeedSteps(fan),
      (++i != Service_Fans.size ? "," : "")
    );
  }

  StringBuf_Printf(s, "\t\t]\n}\n");
  if (s->size + 1 == s->capacity)
    return (errno = ENOBUFS), err_stdlib(0, NULL);

  e = Protocol_Send(socket, s->s, s->size, 0);
  if (e)
    return e;

  e = Protocol_Send_End(socket);
  if (e)
    return e;

  return err_success();
}

static void *Server_Handle_Client(void *arg) {
  int socket = *((int *)arg);
  free(arg);

  const nx_json* json = NULL;
  char* buf = NULL;

  Error* e = Protocol_Receive_Json(socket, &buf, &json);
  if (e) {
    Protocol_Send_Error(socket, err_print_all(e));
    goto END;
  }

  if (json->type != NX_JSON_OBJECT) {
    Protocol_Send_Error(socket, "Not a JSON object");
    goto END;
  }

  const nx_json* command = nx_json_get(json, "command");
  if (! command) {
    Protocol_Send_Error(socket, "Missing 'command' field");
    goto END;
  }

  if (command->type != NX_JSON_STRING) {
    Protocol_Send_Error(socket, "command: not a string");
    goto END;
  }

  pthread_mutex_lock(&Service_Lock);

  if (!strcmp(command->val.text, "set-fan-speed")) {
    Error* e = Server_Command_Set_Fan(socket, json);
    if (e) {
      Protocol_Send_Error(socket, err_print_all(e));
    }
  }
  else if (!strcmp(command->val.text, "status")) {
    Error* e = Server_Command_Status(socket, json);
    if (e) {
      Protocol_Send_Error(socket, err_print_all(e));
    }
  }
  else
  {
    Protocol_Send_Error(socket, "Invalid command");
  }

  pthread_mutex_unlock(&Service_Lock);

END:
  if (buf)
    free(buf);

  if (json)
    nx_json_free(json);

  close(socket);
  return NULL;
}

Error* Server_Init(int port) {
  // Create a TCP/IP socket
  if ((Server_FD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    return err_stdlib(0, "socket()");
  }

  // Bind the socket to an address
  Server_Address.sin_family = AF_INET;
  Server_Address.sin_addr.s_addr = INADDR_ANY;
  Server_Address.sin_port = htons(port);

  if (bind(Server_FD, (struct sockaddr *)&Server_Address, sizeof(Server_Address)) < 0) {
    return err_stdlib(0, "bind()");
  }

  // Listeon for incoming connections
  if (listen(Server_FD, 3) < 0) {
    return err_stdlib(0, "listen()");
  }

  Log_Info("Connected on port %d\n", ntohs(Server_Address.sin_port));
  return err_success();
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
  int* new_socket = malloc(sizeof(int));

  if ((*new_socket = accept(Server_FD, (struct sockaddr*)&Server_Address, (socklen_t*)&addrlen)) < 0) {
    free(new_socket);
    return err_stdlib(0, "accept()");
  }

  pthread_t thread_id;
  if (pthread_create(&thread_id, NULL, Server_Handle_Client, new_socket) != 0) {
    free(new_socket);
    return err_stdlib(0, "pthread_create()");
  }

  pthread_detach(thread_id);

  return err_success();
}

static void* Server_Run(void* arg) {
  while (1) {
    Error* e = Server_Loop();
    e_warn();
  }

  return NULL;
}

void Server_Close() {
  if (Server_FD != -1)
    close(Server_FD);
  Server_FD = -1;
}

// ", \, and control codes (anything less than U+0020).
static const char* Json_EscapeString(char* buf, const size_t n, const char* s) {
  size_t i = 0;
  for (; *s && i < n - 6 - 1; ++s)
    if (*s == '"' || *s == '\\' || *s < 0x20) {
      snprintf(&buf[i], 7, "\\u%.4X", *s);
      i += 6;
    }
    else
      buf[i++] = *s;
  buf[i] = '\0';
  return buf;
}

