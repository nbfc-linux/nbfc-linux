#include "server.h"

#include "nbfc.h"
#include "macros.h"
#include "nxjson_utils.h"
#include "reverse_nxjson.h"
#include "error.h"
#include "fan_temperature_control.h"
#include "service.h"
#include "service_config.h"
#include "log.h"
#include "protocol.h"
#include "memory.h"
#include "stack_memory.h"

#include <errno.h>      // errno, EWOULDBLOCK, EAGAIN, EFBIG, EINTR
#include <stdio.h>      // snprintf
#include <string.h>     // strcmp, memset
#include <unistd.h>     // read, close, unlink
#include <sys/stat.h>   // chmod
#include <sys/socket.h> // socket, bind, listen, accept
#include <sys/un.h>     // sockaddr_un
#include <fcntl.h>      // fcntl
#include <poll.h>       // poll, POLLIN

#define SERVER_MAX_MESSAGE_SIZE 256 // Max size for incoming messages

struct Client {
  int fd;
  bool active;
  char buf[SERVER_MAX_MESSAGE_SIZE];
  size_t bufsz;
};
typedef struct Client Client;
declare_array_of(Client);

static int                Server_FD = -1;
static struct sockaddr_un Server_Address;
static array_of(Client)   Server_Clients = {0};
static struct pollfd*     Server_PollFDs = NULL;
static size_t             Server_PollFDSize = 0;

/* Command "set-fan-speed"
 *
 * Examples of incoming JSON:
 *
 * {"Command": "set-fan-speed", "Speed": <SPEED>}
 * {"Command": "set-fan-speed", "Fan": <NUMBER>, "Speed": <SPEED>}
 * {"Command": "set-fan-speed", "Fan": <NUMBER>, "Speed": "auto"}
 *
 * Note: We don't use StackMemory_Init() here, because that has already
 * been called in Server_HandleClient().
 */
static Error* Server_Command_Set_Fan(int socket, const nx_json* json) {
  int fan = -1;
  float speed = -2;
  const int fancount = Service_Model_Config.FanConfigurations.size;

  nx_json_for_each(c, json) {
    if (!strcmp(c->key, "Command"))
      continue;
    else if (!strcmp(c->key, "Fan")) {
      if (c->type != NX_JSON_INTEGER)
        return err_string(0, "Fan: Not an integer");

      fan = c->val.i;

      if (fan < 0)
        return err_string(0, "Fan: Cannot be negative");
      else if (fan >= fancount)
        return err_string(0, "Fan: No such fan available");
    }
    else if (!strcmp(c->key, "Speed")) {
      if (c->type == NX_JSON_STRING && !strcmp(c->val.text, "auto")) {
        speed = -1;
        continue;
      }
      else if (c->type == NX_JSON_DOUBLE)
        speed = c->val.dbl;
      else if (c->type == NX_JSON_INTEGER)
        speed = c->val.i;
      else {
        return err_string(0, "Speed: Invalid type. Either float or 'auto'");
      }

      if (speed < 0.0 || speed > 100.0)
        return err_string(0, "Speed: Invalid value");
    }
    else {
      return err_string(0, "Unknown arguments");
    }
  }

  if (speed == -2)
    return err_string(0, "Missing argument: Speed");

  for_enumerate_array(int, i, Service_Fans) {
    if (fan == -1 || fan == i) {
      if (speed == -1)
        Fan_SetAutoSpeed(&Service_Fans.data[i].Fan);
      else
        Fan_SetFixedSpeed(&Service_Fans.data[i].Fan, speed);

      if (! options.read_only)
        Fan_ECFlush(&Service_Fans.data[i].Fan);
    }
  }

  Service_WriteTargetFanSpeedsToState();

  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);
  create_json_string("Status", o, "OK");

  Error* e = Protocol_Send_Json(socket, o);
  nx_json_free(o);
  return e;
}

/* Command "status"
 *
 * Examples of incoming JSON:
 *
 * {"Command": "status"}
 *
 * Note: We don't use StackMemory_Init() here, because that has already
 * been called in Server_HandleClient().
 */
static Error* Server_Command_Status(int socket, const nx_json* json) {
  if (json->val.children.length > 1)
      return err_string(0, "Unknown arguments");

  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);
  create_json_integer("PID", o, getpid());
  create_json_string("SelectedConfigId", o, service_config.SelectedConfigId);
  create_json_bool("ReadOnly", o, options.read_only);
  nx_json* fans = create_json_array("Fans", o);

  for_each_array(FanTemperatureControl*, ftc, Service_Fans) {
    const Fan* fan = &ftc->Fan;
    nx_json* fan_json = create_json_object(NULL, fans);
    create_json_string("Name", fan_json, fan->fanConfig->FanDisplayName);
    create_json_double("Temperature", fan_json, ftc->Temperature);
    create_json_bool("AutoMode", fan_json, (fan->mode == Fan_ModeAuto));
    create_json_bool("Critical", fan_json, fan->isCritical);
    create_json_double("CurrentSpeed", fan_json, Fan_GetCurrentSpeed(fan));
    create_json_double("TargetSpeed", fan_json, Fan_GetTargetSpeed(fan));
    create_json_double("RequestedSpeed", fan_json, Fan_GetRequestedSpeed(fan));
    create_json_integer("SpeedSteps", fan_json, Fan_GetSpeedSteps(fan));
  }

  Error* e = Protocol_Send_Json(socket, o);
  nx_json_free(o);
  return e;
}

/* Initialize server.
 *
 * Call socket(), bind() and listen().
 *
 * Also change the mode of the socket file to 0666.
 */
Error* Server_Init() {
  Error* e = NULL;

  memset(&Server_Address, 0, sizeof(Server_Address));
  Server_Address.sun_family = AF_UNIX;
  snprintf(Server_Address.sun_path, sizeof(Server_Address.sun_path), NBFC_SOCKET_PATH);

  if ((Server_FD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    e = err_stdlib(0, "socket()");
    goto error;
  }

  if (bind(Server_FD, (struct sockaddr *)&Server_Address, sizeof(Server_Address)) < 0) {
    e = err_stdlib(err_string(0, NBFC_SOCKET_PATH), "bind()");
    goto error;
  }

  if (chmod(NBFC_SOCKET_PATH, 0666) < 0) {
    e = err_stdlib(err_string(0, NBFC_SOCKET_PATH), "chmod()");
    goto error;
  }

  if (listen(Server_FD, 3) < 0) {
    e = err_stdlib(0, "listen()");
    goto error;
  }

error:
  if (e)
    Server_Close();

  return e;
}

// Return a new `Client` structure
static Client* Server_AllocateClient() {
  // Try to use an existing client that is inactive
  for_each_array(Client*, client, Server_Clients)
    if (! client->active)
      return client;

  // Allocate space for new client
  const size_t idx = Server_Clients.size;
  Server_Clients.data = Mem_Realloc(Server_Clients.data, (idx + 1) * sizeof(Client));
  Server_Clients.size = idx + 1;
  return &Server_Clients.data[idx];
}

/* Initialize a client
 *
 * - Make it active
 * - Set its file descriptor
 * - Reset the buffer
 * - Make the file descriptor non blocking
 */
static Error* Server_UseClient(Client* client, int fd) {
  int flags = fcntl(fd, F_GETFL, 0);

  if (flags == -1)
    return err_stdlib(0, "fcntl()");

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    return err_stdlib(0, "fcntl()");

  client->active = true;
  client->fd = fd;
  client->bufsz = 0;
  client->buf[0] = '\0';
  return err_success();
}

// Find an active client whose file descriptor matches `fd`
static Client* Server_FindClientByFileDescriptor(int fd) {
  for_each_array(Client*, client, Server_Clients)
    if (client->active && client->fd == fd)
      return client;

  return NULL;
}

// Get the number of active clients
static size_t Server_GetNumberOfActiveClients() {
  size_t num_clients = 0;
  for_each_array(Client*, client, Server_Clients)
    num_clients += client->active;
  return num_clients;
}

// Accept a new connection and add setup client
static Error* Server_AcceptClient() {
  Error* e;
  int addrlen = sizeof(Server_Address);
  int new_socket;

  if ((new_socket = accept(Server_FD, (struct sockaddr*)&Server_Address, (socklen_t*)&addrlen)) < 0)
    return err_stdlib(0, "accept()");

  Client* client = Server_AllocateClient();

  e = Server_UseClient(client, new_socket);
  if (e)
    close(new_socket);

  return e;
}

/* Receive the incoming message for the client
 *
 * Return `0` if the message has been fully received, else `-1`.
 */
static int Server_ReceiveMessage(Client* client) {
  size_t space_left;
  size_t size_to_read;
  int    nread;

  do {
    space_left = sizeof(client->buf) - client->bufsz - 1;

    if (space_left <= 0) {
      errno = EFBIG;
      return -1;
    }

    if (space_left > PROTOCOL_BUFFER_SIZE)
      size_to_read = PROTOCOL_BUFFER_SIZE;
    else
      size_to_read = space_left;

    Log_Debug("read(%d, ..., %d)\n", client->fd, size_to_read);

    nread = read(client->fd, client->buf + client->bufsz, size_to_read);

    if (nread < 0)
      return -1;

    client->bufsz += nread;
    client->buf[client->bufsz] = '\0';

    char* end_marker_pos = strstr(client->buf, PROTOCOL_END_MARKER);
    if (end_marker_pos) {
      *end_marker_pos = '\0';
      break;
    }
  } while (nread > 0);

  return 0;
}

/* Process a client connection.
 *
 * - Read the message from the client file descriptor
 * - Parse it as JSON
 * - Call the command functions
 */
static void Server_HandleClient(Client* client) {
  Error* e = NULL;
  const nx_json* json = NULL;

  Log_Debug("Server_HandleClient(fd=%d)\n", client->fd);

  if (Server_ReceiveMessage(client) == -1) {
    switch (errno) {
      case EAGAIN:
#if EAGAIN != EWOULDBLOCK
      case EWOULDBLOCK:
#endif
        return;

      case EFBIG:
        e = err_string(0, "Message too large");
        goto end;

      default:
        Log_Warn("Client %d read failed: %s\n", client->fd, strerror(errno));
        close(client->fd);
        client->active = false;
        return;
    }
  }

  // The functions `Server_Command_Set_Fan()` and `Server_Command_Status()`
  // are also allocating using this stack, so keep this large
  char nxjson_memory[NBFC_MAX_FILE_SIZE];

  StackMemory_Init(nxjson_memory, sizeof(nxjson_memory));

  json = nx_json_parse_utf8(client->buf);

  if (! json) {
    e = err_nxjson(0, "Invalid JSON");
    goto end;
  }

  if (json->type != NX_JSON_OBJECT) {
    e = err_string(0, "Not a JSON object");
    goto end;
  }

  const nx_json* command = nx_json_get(json, "Command");
  if (! command) {
    e = err_string(0, "Missing 'Command' field");
    goto end;
  }

  if (command->type != NX_JSON_STRING) {
    e = err_string(0, "Command: Not a string");
    goto end;
  }

  if (!strcmp(command->val.text, "set-fan-speed"))
    e = Server_Command_Set_Fan(client->fd, json);
  else if (!strcmp(command->val.text, "status"))
    e = Server_Command_Status(client->fd, json);
  else
    e = err_string(0, "Invalid command");

end:
  nx_json_free(json);
  StackMemory_Destroy();
  if (e)
    Protocol_Send_Error(client->fd, err_print_all(e));
  close(client->fd);
  client->active = false;
}

// Hadle incoming connections and process clients
Error* Server_Loop(int timeout) {
  const size_t num_clients = Server_GetNumberOfActiveClients();
  const size_t needed_fdsize = num_clients + 1;

  Log_Debug("Server_Loop(timeout=%d): num clients: %d\n", timeout, num_clients);

  // Allocate pollfd array if needed
  if (needed_fdsize > Server_PollFDSize) {
    Server_PollFDs = Mem_Realloc(Server_PollFDs, needed_fdsize * sizeof(struct pollfd));
    Server_PollFDSize = needed_fdsize;
  }

  // Add server file descriptor to Server_PollFDs
  Server_PollFDs[0].fd = Server_FD;
  Server_PollFDs[0].events = POLLIN;

  // Add clients to Server_PollFDs
  size_t idx = 1;
  for_each_array(Client*, client, Server_Clients) {
    if (client->active) {
      Server_PollFDs[idx].fd = client->fd;
      Server_PollFDs[idx].events = POLLIN;
      ++idx;
    }
  }

  // Call poll() ...
  int poll_count = poll(Server_PollFDs, needed_fdsize, timeout);
  if (poll_count < 0) {
    if (errno != EINTR)
      return err_stdlib(0, "poll()");
    else
      return err_success();
  }

  // We have an incoming connection ...
  if (Server_PollFDs[0].revents & POLLIN) {
    Error* e = Server_AcceptClient();
    if (e)
      return e;
  }

  // Check for activity on client file descriptors ...
  for (idx = 1; idx < needed_fdsize; ++idx) {
    if (Server_PollFDs[idx].revents & POLLIN) {
      Client* client = Server_FindClientByFileDescriptor(Server_PollFDs[idx].fd);
      if (client == NULL)
        Log_Warn("No client with fd=%d found\n", Server_PollFDs[idx].fd);
      else {
        Server_HandleClient(client);
      }
    }
  }

  return err_success();
}

void Server_Close() {
  if (Server_FD != -1) {
    close(Server_FD);
    unlink(NBFC_SOCKET_PATH);
    Server_FD = -1;
  }
}
