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
typedef char AcpiOperationRegionName[5];
declare_array_of(AcpiOperationRegionName);

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
  AcpiOperationRegionName region;
  int   bit_offset;
  int   bit_length;
  int   access_byte_width;
};
typedef struct AcpiRegister AcpiRegister;
declare_array_of(AcpiRegister);

/*
 * Stores information about an ACPI operation region.
 *
 * name:
 *   The full name of the ACPI operation region including its path.
 *
 * type:
 *   The type of the operation region. ("EmbeddedControl", "SystemMemory", ...)
 */
struct AcpiOperationRegion {
  char* name;
  char* type;
};
typedef struct AcpiOperationRegion AcpiOperationRegion;
declare_array_of(AcpiOperationRegion);

/*
 * Stores overall ACPI information.
 *
 * registers:
 *   Holds information about ACPI registers.
 *
 * methods:
 *   Holds information about ACPI methods.
 *
 * regions:
 *   Holds information about ACPI operation regions.
 *
 * ec_region_names:
 *   Holds the names of EC operation regions that expose registers
 *   accessible via the embedded controller.
 */
struct AcpiInfo {
  array_of(AcpiRegister)            registers;
  array_of(AcpiMethod)              methods;
  array_of(AcpiOperationRegion)     regions;
  array_of(AcpiOperationRegionName) ec_region_names;
};
typedef struct AcpiInfo AcpiInfo;

Error Acpi_Analysis_Is_IASL_Installed();
Error Acpi_Analysis_Is_AcpiExec_Installed();

Error Acpi_Analysis_Get_DSL(const char*, char**);
Error Acpi_Analysis_Get_Info(const char*, AcpiInfo*);

bool  Acpi_Analysis_Path_Equals(const char*, const char*);
const char* Acpi_Analysis_Get_Register_Basename(const char*);

nx_json* AcpiMethod_ToJson(AcpiMethod*, const char*, nx_json*);
nx_json* AcpiRegister_ToJson(AcpiRegister*, const char*, nx_json*);

void AcpiInfo_Free(AcpiInfo*);
void AcpiMethod_Free(AcpiMethod*);
void AcpiRegister_Free(AcpiRegister*);
void AcpiRegion_Free(AcpiOperationRegion*);

#endif
