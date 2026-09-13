#ifndef NBFC_REGISTER_WRITE_CONFIGURATION_UTILS_H_
#define NBFC_REGISTER_WRITE_CONFIGURATION_UTILS_H_

#include "error.h"
#include "model_config.h"

Error RegisterWriteConfiguration_Apply(RegisterWriteConfiguration*);
Error RegisterWriteConfigurations_Apply(array_of(RegisterWriteConfiguration)*, bool);

Error RegisterWriteConfiguration_Reset(RegisterWriteConfiguration*);
Error RegisterWriteConfigurations_Reset(array_of(RegisterWriteConfiguration)*);

#endif
