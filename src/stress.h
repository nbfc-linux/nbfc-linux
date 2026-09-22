#ifndef NBFC_STRESS_H_
#define NBFC_STRESS_H_

#include "error.h"

#include <stddef.h> // size_t

size_t Stress_GetNumProcessors(void);
Error Stress_IsInstalled(void);

Error Stress_CPU_Start(size_t);
Error Stress_GPU_Start(size_t);

void Stress_CPU_End(void);
void Stress_GPU_End(void);

#endif
