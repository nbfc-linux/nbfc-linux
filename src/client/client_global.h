#ifndef NBFC_CLIENT_GLOBAL_H_
#define NBFC_CLIENT_GLOBAL_H_

#include "../cli99.h"

enum Option {
  Option_None = 0,

  // General options
  Option_Help,
  Option_Version,

  // Command
  Option_Command,

  // Status options
  Option_Status_All,
  Option_Status_Service,
  Option_Status_Fan,
  Option_Status_Watch,

  // Sensors options
  Option_Sensors_Command,
  Option_Sensors_Fan,
  Option_Sensors_Sensor,
  Option_Sensors_Algorithm,
  Option_Sensors_Force,

  // Config options
  Option_Config_List,
  Option_Config_Recommend,
  Option_Config_Set,
  Option_Config_Apply,
  Option_Config_Yes,

  // Rate-Config options
  Option_RateConfig_File,
  Option_RateConfig_DSDT_File,
  Option_RateConfig_DSDT_Dir,
  Option_RateConfig_All,
  Option_RateConfig_Full_Help,
  Option_RateConfig_Json,
  Option_RateConfig_Input,
  Option_RateConfig_Rules,
  Option_RateConfig_No_Download,
  Option_RateConfig_Print_Rules,
  Option_RateConfig_Min_Score,
  Option_RateConfig_Unverified,
  Option_RateConfig_Bad,
  Option_RateConfig_Quiet,
  Option_RateConfig_FanCount,

  // Acpi-Dump options
  Option_AcpiDump_Command,
  Option_AcpiDump_DSDT_File,
  Option_AcpiDump_DSDT_Dir,
  Option_AcpiDump_Json,
  Option_AcpiDump_Unverified,

  // Set options
  Option_Set_Auto,
  Option_Set_Speed,
  Option_Set_Fan,

  // Start/Restart options
  Option_Start_ReadOnly,

  // Update options
  Option_Update_Parallel,
  Option_Update_Quiet,

  // Support options
  Option_Support_Upload_Firmware,
  Option_Support_Print_Command,
  Option_Support_Create_Archive,

  // Show-Variable options
  Option_ShowVariable_Variable,

  // Xml2Json options
  Option_Xml2Json_File,
};

extern const struct cli99_Option Main_CommandLine[];

#endif
