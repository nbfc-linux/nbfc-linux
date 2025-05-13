#ifndef CLIENT_GLOBAL_H_
#define CLIENT_GLOBAL_H_

#include "../optparse/optparse.h"

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

  // Set options
  Option_Set_Auto,
  Option_Set_Speed,
  Option_Set_Fan,

  // Start/Restart options
  Option_Start_ReadOnly,

  // Update options
  Option_Update_Parallel,
  Option_Update_Quiet,

  // Show-Variable options
  Option_ShowVariable_Variable,
};

extern const cli99_option main_options[];

#endif
