#include "error.h"

#include "stringbuf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SENSORS
#include <sensors/error.h>
#endif

Error error_stack[16];

static void err_print(Error* e, StringBuf* s) {
  if (! e)
    return;

  switch (e->system) {
  case ErrorSystem_Stdlib:  StringBuf_Printf(s, "%s", strerror(e->value.code)); break;
  case ErrorSystem_NxJson:  StringBuf_Printf(s, "%s", NX_JSON_MSGS[e->value.code]); break;
#ifdef HAVE_SENSORS
  case ErrorSystem_Sensors: StringBuf_Printf(s, "%s", sensors_strerror(e->value.code)); break;
#endif
  case ErrorSystem_String:  StringBuf_Printf(s, "%s", e->value.message); break;
  case ErrorSystem_Integer: StringBuf_Printf(s, "%d", e->value.code);    break;
  }
}

static void err_print_all_buf(Error* e, StringBuf* s) {
  if (! e)
    return;

  for (; e > error_stack; --e) {
    err_print(e, s);
    StringBuf_AddCh(s, ':');
    StringBuf_AddCh(s, ' ');
  }

  err_print(e, s);
  StringBuf_AddCh(s, '\n');
}

void err_print_all(Error* e) {
  char buf[4096];
  StringBuf s = { buf, 0, sizeof(buf) - 1 };
  err_print_all_buf(e, &s);
  fprintf(stderr, "%s", s.s);
}

Error* err_integer(Error* e, int i) {
  e = err_allocate(e);
  e->system = ErrorSystem_Integer;
  e->value.code = i;
  return e;
}

Error* err_string(Error* e, const char* message) {
  e = err_allocate(e);
  e->system = ErrorSystem_String;
  e->value.message = message;
  return e;
}

Error* err_stdlib(Error* e, const char* message) {
  e = err_allocate(e);
  e->system = ErrorSystem_Stdlib;
  e->value.code = errno;
  if (message)
    return err_string(e, message);
  return e;
}

Error* err_nxjson(Error* e, const char* message) {
  e = err_allocate(e);
  e->system = ErrorSystem_NxJson;
  e->value.code = NX_JSON_ERROR;
  if (message)
    return err_string(e, message);
  return e;
}

