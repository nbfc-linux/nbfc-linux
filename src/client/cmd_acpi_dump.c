#include "../log.h"
#include "../macros.h"
#include "../memory.h"
#include "../file_utils.h"
#include "../acpi_analysis.h"
#include "../nxjson_write.h"

#include "check_root.h"
#include "client_global.h"

#include <stdio.h>  // printf
#include <string.h> // strcmp

#define ACPI_DUMP_MAX_AML_FILES 64

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
  const char* files[ACPI_DUMP_MAX_AML_FILES];
  size_t files_size;
} Acpi_Dump_Options = {
  AcpiDump_Action_None,
  false,
  {0},
  0,
};

enum AcpiDump_Action AcpiDump_CommandFromString(const char* s) {
  if (! strcmp(s, "registers"))    return AcpiDump_Action_Registers;
  if (! strcmp(s, "ec-registers")) return AcpiDump_Action_ECRegisters;
  if (! strcmp(s, "methods"))      return AcpiDump_Action_Methods;
  if (! strcmp(s, "dsl"))          return AcpiDump_Action_DSL;
  return AcpiDump_Action_None;
}

/*
 * Disassembles the AML files and writes them to stdout.
 */
static int AcpiDump_DSL(array_of(str)* aml_files) {
  Error e;
  char* out;

  e = Acpi_Analysis_Is_IASL_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  for_each_array(str*, file, *aml_files) {
    e = Acpi_Analysis_Get_DSL(*file, &out);
    if (e) {
      Log_Error("%s", err_print_all(e));
      return NBFC_EXIT_FAILURE;
    }

    printf("%s", out);
    Mem_Free(out);
  }

  return NBFC_EXIT_SUCCESS;
}

/*
 * Dumps the methods from the given AML files to stdout.
 */
static int AcpiDump_Methods(array_of(str)* aml_files, bool json) {
  Error e;
  AcpiInfo acpi_info = {0};

  e = Acpi_Analysis_Is_AcpiExec_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  e = Acpi_Analysis_Get_Info(aml_files, &acpi_info);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (json) {
    nx_json root = {0};
    nx_json* array = create_json_array(NULL, &root);
    for_each_array(AcpiMethod*, method, acpi_info.methods)
      AcpiMethod_ToJson(method, NULL, array);
    nxjson_write_to_fd(array, STDOUT_FILENO, 2);
#if STRICT_CLEANUP
    nx_json_free(array);
#endif
  }
  else {
    for_each_array(AcpiMethod*, method, acpi_info.methods) {
      printf("%s args=%u\n", method->name, method->length);
    }
  }

#if STRICT_CLEANUP
  AcpiInfo_Free(&acpi_info);
#endif

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
 * Dump all available registers from the given AML files to stdout.
 *
 * If `only_ec` is true, only output registers that are available
 * through the embedded controller.
 */
static int AcpiDump_Registers(array_of(str)* aml_files, bool json, bool only_ec) {
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

  e = Acpi_Analysis_Get_Info(aml_files, &acpi_info);
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

    nxjson_write_to_fd(array, STDOUT_FILENO, 2);

#if STRICT_CLEANUP
    nx_json_free(array);
#endif
  }
  else {
    for_each_array(AcpiRegister*, register_, acpi_info.registers) {
      if (only_ec && !AcpiDump_ContainsRegion(&acpi_info.ec_region_names, register_->region))
        continue;

      printf("%s [%s] byte=%u byte_hex=0x%X bit=%u total_bit=%u len=%u acc=%u\n",
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

#if STRICT_CLEANUP
  AcpiInfo_Free(&acpi_info);
#endif

  return NBFC_EXIT_SUCCESS;
}

static Error AcpiDump_MakeAMLFilesArray(array_of(str)* out) {
  if (Acpi_Dump_Options.files_size) {
    out->data = Acpi_Dump_Options.files;
    out->size = Acpi_Dump_Options.files_size;
    return err_success();
  }
  else {
    return Acpi_Analysis_Get_All_AML_Files(out);
  }
}

int AcpiDump(void) {
  Error e;
  array_of(str) aml_files = {0};
  const bool json = Acpi_Dump_Options.json;

  if (Acpi_Dump_Options.action == AcpiDump_Action_None) {
    Log_Error("Missing command");
    return NBFC_EXIT_CMDLINE;
  }

  if (! Acpi_Dump_Options.files_size) {
    check_root();
  }

  for (size_t i = 0; i < Acpi_Dump_Options.files_size; ++i) {
    if (! file_is_readable(Acpi_Dump_Options.files[i])) {
      Log_Error("%s: %s", Acpi_Dump_Options.files[i], strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
  }

  e = AcpiDump_MakeAMLFilesArray(&aml_files);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  switch (Acpi_Dump_Options.action) {
    case AcpiDump_Action_DSL:         return AcpiDump_DSL(&aml_files);
    case AcpiDump_Action_Methods:     return AcpiDump_Methods(&aml_files, json);
    case AcpiDump_Action_Registers:   return AcpiDump_Registers(&aml_files, json, false);
    case AcpiDump_Action_ECRegisters: return AcpiDump_Registers(&aml_files, json, true);
    default:                          return NBFC_EXIT_FAILURE;
  }
}
