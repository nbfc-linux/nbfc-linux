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

const struct cli99_Option AcpiDump_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"command",         Option_AcpiDump_Command,    cli99_NormalPositional},
  {"-d|--dsdt",       Option_AcpiDump_DSDT_File,  cli99_RequiredArgument},
  {"-D|--dsdt-dir",   Option_AcpiDump_DSDT_Dir,   cli99_RequiredArgument},
  {"-j|--json",       Option_AcpiDump_Json,       cli99_NoArgument      },
  {"-u|--unverified", Option_AcpiDump_Unverified, cli99_NoArgument      },
  cli99_Options_End()
};

enum NBFC_PACKED_ENUM AcpiDump_Action {
  AcpiDump_Action_None,
  AcpiDump_Action_Registers,
  AcpiDump_Action_ECRegisters,
  AcpiDump_Action_Methods,
  AcpiDump_Action_DSL,
  AcpiDump_Action_Map,
};

struct {
  enum AcpiDump_Action action;
  bool json;
  bool unverified;
  const char* files[ACPI_ANALYSIS_MAX_AML_FILES];
  size_t files_size;
  const char* dir;
} AcpiDump_Options = {
  AcpiDump_Action_None,
  false,
  false,
  {0},
  0,
  NULL,
};

enum AcpiDump_Action AcpiDump_CommandFromString(const char* s) {
  if (! strcmp(s, "registers"))    return AcpiDump_Action_Registers;
  if (! strcmp(s, "ec-registers")) return AcpiDump_Action_ECRegisters;
  if (! strcmp(s, "methods"))      return AcpiDump_Action_Methods;
  if (! strcmp(s, "dsl"))          return AcpiDump_Action_DSL;
  if (! strcmp(s, "map"))          return AcpiDump_Action_Map;
  return AcpiDump_Action_None;
}

/*
 * Disassembles the AML files and writes them to stdout.
 */
static int AcpiDump_DSL(array_of(str)* aml_files) {
  Error e;
  char* out;

  e = AcpiAnalysis_IsIaslInstalled();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  for_each_array(str*, file, *aml_files) {
    e = AcpiAnalysis_DisassembleFile(*file, &out);
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

  e = AcpiAnalysis_IsAcpiExecInstalled();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  e = AcpiAnalysis_GetInfo(aml_files, &acpi_info);
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
 * Dump all available registers from the given AML files to stdout.
 *
 * If `only_ec` is true, only output registers that are available
 * through the embedded controller.
 */
static int AcpiDump_Registers(array_of(str)* aml_files, bool json, bool only_ec, bool unverified) {
  Error e;
  AcpiInfo acpi_info = {0};

  // ==========================================================================
  // Check if apcica-tools are installed
  // ==========================================================================

  e = AcpiAnalysis_IsAcpiExecInstalled();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Get ACPI info
  // ==========================================================================

  e = AcpiAnalysis_GetInfo(aml_files, &acpi_info);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Add unverified EC registers
  // ==========================================================================

  if (unverified)
    AcpiAnalysis_AddUnverifiedEmbeddedControllerRegions(&acpi_info);

  // ==========================================================================
  // Output
  // ==========================================================================

  if (json) {
    nx_json root = {0};
    nx_json* array = create_json_array(NULL, &root);

    for_each_array(AcpiRegister*, register_, acpi_info.registers) {
      if (only_ec && !AcpiAnalysis_IsEmbeddedControllerRegion(&acpi_info, register_->region))
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
      if (only_ec && !AcpiAnalysis_IsEmbeddedControllerRegion(&acpi_info, register_->region))
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

/*
 * Dumps a map file to stdout:
 *
 *   EC_REGISTER_NAME=ADDRESS
 *   ...
 *   METHOD_NAME
 *
 * Registers are limited to EC registers, identified by their basename
 * and addressed by their byte offset (e.g. "CFAN=0x100").
 * All ACPI methods are printed as well, one per line.
 */
static int AcpiDump_Map(array_of(str)* aml_files, bool unverified) {
  Error e;
  AcpiInfo acpi_info = {0};

  // ==========================================================================
  // Check if apcica-tools are installed
  // ==========================================================================

  e = AcpiAnalysis_IsAcpiExecInstalled();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Get ACPI info
  // ==========================================================================

  e = AcpiAnalysis_GetInfo(aml_files, &acpi_info);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Add unverified EC registers
  // ==========================================================================

  if (unverified)
    AcpiAnalysis_AddUnverifiedEmbeddedControllerRegions(&acpi_info);

  // ==========================================================================
  // Output
  // ==========================================================================

  for_each_array(AcpiRegister*, register_, acpi_info.registers) {
    if (! AcpiAnalysis_IsEmbeddedControllerRegion(&acpi_info, register_->region))
      continue;

    printf("%s=0x%X\n",
      AcpiAnalysis_RegisterBasename(register_->name),
      register_->bit_offset / 8);
  }

  for_each_array(AcpiMethod*, method, acpi_info.methods) {
    printf("%s\n", method->name);
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
  if (AcpiDump_Options.files_size) {
    out->data = AcpiDump_Options.files;
    out->size = AcpiDump_Options.files_size;
    return err_success();
  }
  else if (AcpiDump_Options.dir) {
    return AcpiAnalysis_GetAmlFiles(AcpiDump_Options.dir, out);
  }
  else {
    return AcpiAnalysis_GetAmlFiles(NULL, out);
  }
}

int AcpiDump(void) {
  Error e;
  array_of(str) aml_files = {0};
  const bool json = AcpiDump_Options.json;
  const bool unverified = AcpiDump_Options.unverified;

  if (AcpiDump_Options.action == AcpiDump_Action_None) {
    Log_Error("acpi-dump: Missing command");
    return NBFC_EXIT_CMDLINE;
  }

  if (! AcpiDump_Options.files_size && ! AcpiDump_Options.dir) {
    check_root();
  }

  for (size_t i = 0; i < AcpiDump_Options.files_size; ++i) {
    if (! file_is_readable(AcpiDump_Options.files[i])) {
      Log_Error("%s: %s", AcpiDump_Options.files[i], strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
  }

  e = AcpiDump_MakeAMLFilesArray(&aml_files);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  switch (AcpiDump_Options.action) {
    case AcpiDump_Action_DSL:         return AcpiDump_DSL(&aml_files);
    case AcpiDump_Action_Methods:     return AcpiDump_Methods(&aml_files, json);
    case AcpiDump_Action_Registers:   return AcpiDump_Registers(&aml_files, json, false, unverified);
    case AcpiDump_Action_ECRegisters: return AcpiDump_Registers(&aml_files, json, true, unverified);
    case AcpiDump_Action_Map:         return AcpiDump_Map(&aml_files, unverified);
    default:                          return NBFC_EXIT_FAILURE;
  }
}
