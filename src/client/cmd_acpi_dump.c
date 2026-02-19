#include "../log.h"
#include "../macros.h"
#include "../file_utils.h"
#include "../acpi_analysis.h"
#include "../nxjson_write.h"

#include "check_root.h"
#include "client_global.h"

#include <stdio.h>  // printf
#include <string.h> // strcmp

const struct cli99_Option acpi_dump_options[] = {
  cli99_Options_Include(&main_options),
  {"command",   Option_Acpi_Dump_Command, cli99_NormalPositional},
  {"-f|--file", Option_Acpi_Dump_File,    cli99_RequiredArgument},
  {"-j|--json", Option_Acpi_Dump_Json,    cli99_NoArgument      },
  cli99_Options_End()
};

enum NBFC_PACKED_ENUM AcpiDump_Action {
  AcpiDump_Action_None,
  AcpiDump_Action_Registers,
  AcpiDump_Action_ECRegisters,
  AcpiDump_Action_Methods,
  AcpiDump_Action_DSL,
};

struct {
  enum AcpiDump_Action action;
  bool json;
  const char* file;
} Acpi_Dump_Options = {
  AcpiDump_Action_None,
  false,
  ACPI_ANALYSIS_ACPI_DSDT
};

enum AcpiDump_Action AcpiDump_CommandFromString(const char* s) {
  if (! strcmp(s, "registers"))    return AcpiDump_Action_Registers;
  if (! strcmp(s, "ec-registers")) return AcpiDump_Action_ECRegisters;
  if (! strcmp(s, "methods"))      return AcpiDump_Action_Methods;
  if (! strcmp(s, "dsl"))          return AcpiDump_Action_DSL;
  return AcpiDump_Action_None;
}

/*
 * Dumps the disassembled DSDT to stdout.
 */
static int AcpiDump_DSL(const char* dsdt_file) {
  Error e;
  char* out;

  e = Acpi_Analysis_Is_IASL_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  e = Acpi_Analysis_Get_DSL(dsdt_file, &out);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  printf("%s", out);
  Mem_Free(out);
  return NBFC_EXIT_SUCCESS;
}

/*
 * Dumps the methods of a DSDT to stdout.
 */
static int AcpiDump_Methods(const char* dsdt_file, bool json) {
  Error e;
  AcpiInfo acpi_info = {0};

  e = Acpi_Analysis_Is_AcpiExec_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  e = Acpi_Analysis_Get_Info(dsdt_file, &acpi_info);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (json) {
    nx_json root = {0};
    nx_json* array = create_json_array(NULL, &root);
    for_each_array(AcpiMethod*, method, acpi_info.methods)
      AcpiMethod_ToJson(method, NULL, array);
    nxjson_write_to_fd(array, STDOUT_FILENO);
    nx_json_free(array);
  }
  else {
    for_each_array(AcpiMethod*, method, acpi_info.methods) {
      printf("%s args=%d\n", method->name, method->length);
    }
  }

  AcpiInfo_Free(&acpi_info);
  return NBFC_EXIT_SUCCESS;
}

/*
 * Check if `name` is a region contained in `ec_regions`.
 */
static bool AcpiDump_ContainsRegion(array_of(AcpiOperationRegionName)* ec_regions, const char* name) {
  for_each_array(AcpiOperationRegionName*, region, *ec_regions) {
    if (! strcmp(*region, name))
      return true;
  }

  return false;
}

/*
 * Dump all available registers of a DSDT to stdout.
 *
 * If `only_ec` is true, only output registers that are available
 * through the embedded controller.
 */
static int AcpiDump_Registers(const char* dsdt_file, bool json, bool only_ec) {
  Error e;
  AcpiInfo acpi_info = {0};

  // ==========================================================================
  // Check if apcica-tools are installed
  // ==========================================================================

  e = Acpi_Analysis_Is_AcpiExec_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Get ACPI info
  // ==========================================================================

  e = Acpi_Analysis_Get_Info(dsdt_file, &acpi_info);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Output
  // ==========================================================================

  if (json) {
    nx_json root = {0};
    nx_json* array = create_json_array(NULL, &root);

    for_each_array(AcpiRegister*, register_, acpi_info.registers) {
      if (only_ec && !AcpiDump_ContainsRegion(&acpi_info.ec_region_names, register_->region))
        continue;

      AcpiRegister_ToJson(register_, NULL, array);
    }

    nxjson_write_to_fd(array, STDOUT_FILENO);
    nx_json_free(array);
  }
  else {
    for_each_array(AcpiRegister*, register_, acpi_info.registers) {
      if (only_ec && !AcpiDump_ContainsRegion(&acpi_info.ec_region_names, register_->region))
        continue;

      printf("%s [%s] byte=%d byte_hex=0x%X bit=%d total_bit=%d len=%d acc=%d\n",
        register_->name,
        register_->region,
        register_->bit_offset / 8,
        register_->bit_offset / 8,
        register_->bit_offset % 8,
        register_->bit_offset,
        register_->bit_length,
        register_->access_byte_width);
    }
  }

  // ==========================================================================
  // Free data
  // ==========================================================================

  AcpiInfo_Free(&acpi_info);
  return NBFC_EXIT_SUCCESS;
}

int AcpiDump() {
  const char* const dsdt_file = Acpi_Dump_Options.file;
  const bool json = Acpi_Dump_Options.json;

  if (Acpi_Dump_Options.action == AcpiDump_Action_None) {
    Log_Error("Missing command");
    return NBFC_EXIT_CMDLINE;
  }

  if (! file_exists(dsdt_file)) {
    Log_Error("%s: %s", dsdt_file, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }

  if (! file_is_readable(dsdt_file)) {
    Log_Error("%s: %s (do you need root priviledges?)", dsdt_file, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }

  switch (Acpi_Dump_Options.action) {
    case AcpiDump_Action_DSL:         return AcpiDump_DSL(dsdt_file);
    case AcpiDump_Action_Methods:     return AcpiDump_Methods(dsdt_file, json);
    case AcpiDump_Action_Registers:   return AcpiDump_Registers(dsdt_file, json, false);
    case AcpiDump_Action_ECRegisters: return AcpiDump_Registers(dsdt_file, json, true);
    default:                          return NBFC_EXIT_FAILURE;
  }
}
