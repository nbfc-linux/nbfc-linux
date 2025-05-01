#include "error.h"

#include "stringbuf.h"
#include "nxjson.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static Error error_stack[16];

static inline Error* err_allocate(Error* e) {
  return e ? ++e : error_stack;
}

static void err_print(const Error* e, StringBuf* s) {
  if (! e)
    return;

  switch (e->system) {
  case ErrorSystem_Stdlib:  StringBuf_Printf(s, "%s", strerror(e->value.code)); break;
  case ErrorSystem_NxJson:  StringBuf_Printf(s, "%s", NX_JSON_MSGS[e->value.code]); break;
  case ErrorSystem_String:  StringBuf_Printf(s, "%s", e->value.message); break;
  }
}

const char* err_print_all(const Error* e) {
  static char buf[4096];
  StringBuf s = { buf, 0, sizeof(buf) - 1 };

  buf[0] = '\0';

  if (! e)
    return buf;

  for (; e > error_stack; --e) {
    err_print(e, &s);
    StringBuf_AddCh(&s, ':');
    StringBuf_AddCh(&s, ' ');
  }

  err_print(e, &s);

  return buf;
}

Error* err_string(Error* e, const char* message) {
  e = err_allocate(e);
  e->system = ErrorSystem_String;
  snprintf(e->value.message, sizeof(e->value.message), "%s", message);
  return e;
}

Error* err_stringf(Error* e, const char* message, ...) {
  e = err_allocate(e);
  e->system = ErrorSystem_String;

  va_list args;
  va_start(args, message);
  vsnprintf(e->value.message, sizeof(e->value.message), message, args);
  va_end(args);

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
