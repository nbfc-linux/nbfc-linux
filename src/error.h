#ifndef NBFC_ERROR_H_
#define NBFC_ERROR_H_

#include "log.h"

#include <errno.h>
#include <stddef.h>

enum ErrorSystem {
  ErrorSystem_String,
  ErrorSystem_Integer,
  ErrorSystem_Stdlib,
  ErrorSystem_NxJson,
#ifdef HAVE_SENSORS
  ErrorSystem_Sensors,
#endif
};

typedef struct Error Error;
struct Error {
  enum ErrorSystem system;
  union {
    int code;
    const char* message;
  } value;
};

extern Error error_stack[16];

#define e_warn()      do { if (e) { Log_Warn("%s\n", err_print_all(e)); } } while (0)
#define e_die()       do { if (e) { Log_Error("%s\n", err_print_all(e)); exit(EXIT_FAILURE); } } while(0)
#define e_check()     do { if (e) return e;         } while(0)
#define e_goto(LABEL) do { if (e) goto LABEL;       } while(0)
#define err_success() NULL
#define die(E, ...)   do { fprintf(stderr, __VA_ARGS__); exit(E); } while(0)

Error* err_integer(Error*, int i);
Error* err_string(Error*,  const char* message);
Error* err_stdlib(Error*,  const char* message);
Error* err_nxjson(Error*,  const char* message);
const char* err_print_all(Error*);

static inline Error* err_allocate(Error* e) {
  return e ? ++e : error_stack;
}

#endif
