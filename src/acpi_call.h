#ifndef NBFC_ACPI_CALL_H_
#define NBFC_ACPI_CALL_H_

#include <stdint.h>

#include "error.h"

Error* AcpiCall_Open();
Error* AcpiCall_Call(const char*, ssize_t, uint64_t*);
Error* AcpiCall_CallTemplate(const char*, uint64_t, uint64_t*);

#endif
