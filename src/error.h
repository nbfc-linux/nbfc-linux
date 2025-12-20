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

struct ErrorImpl {
  ErrorSystem system;
  union {
    int code;
    char message[1024];
  } value;
};
typedef struct ErrorImpl ErrorImpl;

typedef struct ErrorImpl* Error;

#define e_warn()      do { if (e) { Log_Warn("%s", err_print_all(e)); } } while (0)
#define e_die()       do { if (e) { Log_Error("%s", err_print_all(e)); exit(EXIT_FAILURE); } } while(0)
#define e_check()     do { if (e) return e;         } while(0)
#define e_goto(LABEL) do { if (e) goto LABEL;       } while(0)
#define err_success() NULL

#define err_string(MESSAGE)      err_chain_string(0, MESSAGE)
#define err_stringf(FORMAT, ...) err_chain_stringf(0, FORMAT, __VA_ARGS__)
#define err_stdlib(MESSAGE)      err_chain_stdlib(0, MESSAGE)
#define err_nxjson(MESSAGE)      err_chain_nxjson(0, MESSAGE)

Error err_chain_string(Error,  const char* message);
Error err_chain_stringf(Error, const char* message, ...);
Error err_chain_stdlib(Error,  const char* message);
Error err_chain_nxjson(Error,  const char* message);
const char* err_print_all(Error);

#endif
