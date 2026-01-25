#include "../log.h"
#include "../file_utils.h"
#include "../acpi_analysis.h"
#include "../nxjson_write.h"

#include "check_root.h"
#include "client_global.h"

#include <stdio.h>  // printf
#include <string.h> // strcmp

const cli99_option acpi_dump_options[] = {
  cli99_include_options(&main_options),
  {"command",   Option_Acpi_Dump_Command, 1},
  {"-f|--file", Option_Acpi_Dump_File,    1},
  {"-j|--json", Option_Acpi_Dump_Json,    0},
  cli99_options_end()
};

enum AcpiDump_Action {
  AcpiDump_Action_None,
  AcpiDump_Action_Registers,
  AcpiDump_Action_ECRegisters,
  AcpiDump_Action_Methods,
  AcpiDump_Action_DSL,
};

struct {
  enum AcpiDump_Action action;
  const char* file;
  bool json;
} Acpi_Dump_Options = {
  AcpiDump_Action_None,
  ACPI_ANALYSIS_ACPI_DSDT,
  false
};

enum AcpiDump_Action AcpiDump_CommandFromString(const char* s) {
  if (! strcmp(s, "registers"))    return AcpiDump_Action_Registers;
  if (! strcmp(s, "ec-registers")) return AcpiDump_Action_ECRegisters;
  if (! strcmp(s, "methods"))      return AcpiDump_Action_Methods;
  if (! strcmp(s, "dsl"))          return AcpiDump_Action_DSL;
  return AcpiDump_Action_None;
}

/*
 * Dumps a JSON object to STDOUT.
 */
static inline void AcpiDump_WriteJson(const nx_json* json) {
  NX_JSON_Write write_obj = NX_JSON_Write_Init(STDOUT_FILENO, WriteMode_Write);
  nx_json_write(&write_obj, json, 0);
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
  array_of(AcpiMethod) methods = {0};

  e = Acpi_Analysis_Is_AcpiExec_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  e = Acpi_Analysis_Get_Methods(dsdt_file, &methods);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (json) {
    nx_json root = {0};
    nx_json* array = create_json_array(NULL, &root);
    for_each_array(AcpiMethod*, method, methods)
      AcpiMethod_ToJson(method, NULL, array);
    AcpiDump_WriteJson(array);
    nx_json_free(array);
  }
  else {
    for_each_array(AcpiMethod*, method, methods) {
      printf("%s args=%d\n", method->name, method->length);
    }
  }

  for_each_array(AcpiMethod*, method, methods) {
    AcpiMethod_Free(method);
  }
  Mem_Free(methods.data);
  return NBFC_EXIT_SUCCESS;
}

/*
 * Check if `name` is a region contained in `ec_regions`.
 */
static bool AcpiDump_ContainsRegion(array_of(AcpiOperationRegion)* ec_regions, const char* name) {
  for_each_array(AcpiOperationRegion*, region, *ec_regions) {
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
  array_of(AcpiRegister) registers = {0};
  array_of(AcpiOperationRegion) ec_regions = {0};

  // ==========================================================================
  // Check if apcica-tools are installed
  // ==========================================================================

  e = Acpi_Analysis_Is_AcpiExec_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (only_ec) {
    e = Acpi_Analysis_Is_IASL_Installed();
    if (e) {
      Log_Error("%s", err_print_all(e));
      return NBFC_EXIT_FAILURE;
    }
  }

  // ==========================================================================
  // Get registers and EC operation regions
  // ==========================================================================

  e = Acpi_Analysis_Get_Registers(dsdt_file, &registers);
  if (e) {
    Log_Error("%s", err_print_all(e));
    goto end;
  }

  if (only_ec) {
    e = Acpi_Analysis_Get_EC_OperationRegions(dsdt_file, &ec_regions);
    if (e) {
      Log_Error("%s", err_print_all(e));
      goto end;
    }
  }

  // ==========================================================================
  // Output
  // ==========================================================================

  if (json) {
    nx_json root = {0};
    nx_json* array = create_json_array(NULL, &root);

    for_each_array(AcpiRegister*, register_, registers) {
      if (only_ec && !AcpiDump_ContainsRegion(&ec_regions, register_->region))
        continue;

      AcpiRegister_ToJson(register_, NULL, array);
    }

    AcpiDump_WriteJson(array);
    nx_json_free(array);
  }
  else {
    for_each_array(AcpiRegister*, register_, registers) {
      if (only_ec && !AcpiDump_ContainsRegion(&ec_regions, register_->region))
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

end:
  for_each_array(AcpiRegister*, register_, registers) {
    AcpiRegister_Free(register_);
  }
  Mem_Free(registers.data);
  Mem_Free(ec_regions.data);

  if (e)
    return NBFC_EXIT_FAILURE;
  else
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
