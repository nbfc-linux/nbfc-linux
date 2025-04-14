#ifndef CLIENT_GLOBAL_H_
#define CLIENT_GLOBAL_H_

#include <stdbool.h>

#include "../optparse/optparse.h"

struct Global_Options_ {
  bool python_hack;
};

enum Option {
  Option_None = 0,

  // General options
  Option_Help,
  Option_Version,
  Option_PythonHack,

  // Command
  Option_Command,

  // Status options
  Option_Status_All,
  Option_Status_Service,
  Option_Status_Fan,
  Option_Status_Watch,

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

  // Show-Variable options
  Option_ShowVariable_Variable,
};

extern struct Global_Options_ Global_Options;
extern const cli99_option main_options[];

#endif
