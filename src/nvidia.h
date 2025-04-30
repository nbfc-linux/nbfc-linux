#ifndef NVIDIA_H_
#define NVIDIA_H_

#include "error.h"

enum Nvidia_Error {
  Nvidia_Error_OK     = 0,
  Nvidia_Error_DlOpen = -1,
  Nvidia_Error_API    = -2
};
typedef enum Nvidia_Error Nvidia_Error;

Nvidia_Error Nvidia_Init();
void         Nvidia_Close();
Error*       Nvidia_GetTemperature(float*);

#endif
