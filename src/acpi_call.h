#ifndef NBFC_ACPI_CALL_H_
#define NBFC_ACPI_CALL_H_

#include <stdint.h> // uint64_t

#include "error.h"

Error AcpiCall_Open(void);
Error AcpiCall_Call(const char*, uint64_t, uint64_t*);

#endif
