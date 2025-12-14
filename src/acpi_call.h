#ifndef NBFC_ACPI_CALL_H_
#define NBFC_ACPI_CALL_H_

#include <stdint.h>    // uint64_t
#include <sys/types.h> // ssize_t
#include <string.h>    // strlen

#include "error.h"

Error* AcpiCall_Open();
Error* AcpiCall_Call(const char*, ssize_t, uint64_t*);
Error* AcpiCall_CallTemplate(const char*, uint64_t, uint64_t*);

static inline Error* AcpiCall_Call_Str(const char* s, uint64_t* out) {
  return AcpiCall_Call(s, strlen(s), out);
}

#endif
