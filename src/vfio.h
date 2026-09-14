#ifndef NBFC_VFIO_H_
#define NBFC_VFIO_H_

#include <stdbool.h>

bool VFIO_CheckProcCmdline(void);
bool VFIO_CheckSysBusPciDevices(void);

#endif
