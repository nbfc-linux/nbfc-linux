#ifndef NBFC_ACPI_ANALYSIS_H_
#define NBFC_ACPI_ANALYSIS_H_

#include "error.h"
#include "macros.h"
#include "nxjson.h"

#include <stdbool.h>

#define ACPI_ANALYSIS_IASL      "iasl"
#define ACPI_ANALYSIS_ACPIEXEC  "acpiexec"
#define ACPI_ANALYSIS_ACPI_DSDT "/sys/firmware/acpi/tables/DSDT"

/*
 * Stores a register name.
 */
typedef char AcpiRegisterName[5];
declare_array_of(AcpiRegisterName);

/*
 * Stores a operation region name.
 */
typedef char AcpiOperationRegion[5];
declare_array_of(AcpiOperationRegion);

/*
 * Stores information about an ACPI method.
 *
 * name:
 *   The full name of an ACPI method including its path.
 *
 * length:
 *   The number of arguments.
 */
struct AcpiMethod {
  char* name;
  int   length;
};
typedef struct AcpiMethod AcpiMethod;
declare_array_of(AcpiMethod);

/*
 * Stores information about an ACPI register.
 *
 * name:
 *   The full name of an ACPI register including its path.
 *
 * region:
 *   The operation region where the register is stored.
 *
 * bit_offset:
 *   The bit offset of the register.
 *
 * bit_length:
 *   The bit length of the register.
 *
 * access_byte_width:
 *   The access byte width of the register.
 */
struct AcpiRegister {
  char* name;
  AcpiOperationRegion region;
  int   bit_offset;
  int   bit_length;
  int   access_byte_width;
};
typedef struct AcpiRegister AcpiRegister;
declare_array_of(AcpiRegister);

Error Acpi_Analysis_Is_IASL_Installed();
Error Acpi_Analysis_Is_AcpiExec_Installed();

Error Acpi_Analysis_Get_DSL(const char*, char**);
Error Acpi_Analysis_Get_Methods(const char*, array_of(AcpiMethod)*);
Error Acpi_Analysis_Get_Registers(const char*, array_of(AcpiRegister)*);
Error Acpi_Analysis_Get_EC_OperationRegions(const char*, array_of(AcpiOperationRegion)*);

bool  Acpi_Analysis_Path_Equals(const char*, const char*);
const char* Acpi_Analysis_Get_Register_Basename(const char*);

nx_json* AcpiMethod_ToJson(AcpiMethod*, const char*, nx_json*);
nx_json* AcpiRegister_ToJson(AcpiRegister*, const char*, nx_json*);

void AcpiMethod_Free(AcpiMethod*);
void AcpiRegister_Free(AcpiRegister*);

#endif
