#ifndef NBFC_ERROR_H_
#define NBFC_ERROR_H_

#include "log.h"

#include <stddef.h>

enum ErrorSystem {
  ErrorSystem_String,
  ErrorSystem_Stdlib,
  ErrorSystem_NxJson,
};
typedef enum ErrorSystem ErrorSystem;

struct Error {
  ErrorSystem system;
  union {
    int code;
    char message[1024];
  } value;
};
typedef struct Error Error;

#define e_warn()      do { if (e) { Log_Warn("%s\n", err_print_all(e)); } } while (0)
#define e_die()       do { if (e) { Log_Error("%s\n", err_print_all(e)); exit(EXIT_FAILURE); } } while(0)
#define e_check()     do { if (e) return e;         } while(0)
#define e_goto(LABEL) do { if (e) goto LABEL;       } while(0)
#define err_success() NULL

Error* err_string(Error*,  const char* message);
Error* err_stringf(Error*, const char* message, ...);
Error* err_stdlib(Error*,  const char* message);
Error* err_nxjson(Error*,  const char* message);
const char* err_print_all(const Error*);

#endif
