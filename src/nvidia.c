#include "nvidia.h"

#include <dlfcn.h>

// ============================================================================
// Type definitions and constants for accessing NVIDIA GPU information via libnvidia-ml
// ============================================================================

typedef struct nvmlDevice_st* nvmlDevice_t;
typedef int nvmlReturn_t;

#define NVML_TEMPERATURE_GPU 0
#define NVML_SUCCESS 0

// ============================================================================
// NVML-related globals used for dynamically accessing GPU temperature:
// ============================================================================

static void *Nvidia_DlHandle = NULL;
static nvmlDevice_t Nvidia_Device;

nvmlReturn_t (*Nvidia_nvmlInit_fn)(void);
nvmlReturn_t (*Nvidia_nvmlShutdown_fn)(void);
nvmlReturn_t (*Nvidia_nvmlDeviceGetHandleByIndex_fn)(unsigned int, nvmlDevice_t*);
nvmlReturn_t (*Nvidia_nvmlDeviceGetTemperature_fn)(nvmlDevice_t, unsigned int, unsigned int*);

// ============================================================================
// Nvidia_* Functions
// ============================================================================

Nvidia_Error Nvidia_Init() {
  if (! Nvidia_DlHandle) {
    Nvidia_DlHandle = dlopen("libnvidia-ml.so.1", RTLD_LAZY);

    if (! Nvidia_DlHandle)
      return Nvidia_Error_DlOpen;
  }

  Nvidia_nvmlInit_fn = (nvmlReturn_t (*)(void)) dlsym(Nvidia_DlHandle, "nvmlInit");
  if (!Nvidia_nvmlInit_fn)
    return Nvidia_Error_DlOpen;

  Nvidia_nvmlShutdown_fn = (nvmlReturn_t (*)(void)) dlsym(Nvidia_DlHandle, "nvmlShutdown");
  if (!Nvidia_nvmlShutdown_fn)
    return Nvidia_Error_DlOpen;

  Nvidia_nvmlDeviceGetHandleByIndex_fn = (nvmlReturn_t (*)(unsigned int, nvmlDevice_t *)) dlsym(Nvidia_DlHandle, "nvmlDeviceGetHandleByIndex");
  if (!Nvidia_nvmlDeviceGetHandleByIndex_fn)
    return Nvidia_Error_DlOpen;

  Nvidia_nvmlDeviceGetTemperature_fn = (nvmlReturn_t (*)(nvmlDevice_t, unsigned int, unsigned int *)) dlsym(Nvidia_DlHandle, "nvmlDeviceGetTemperature");
  if (!Nvidia_nvmlDeviceGetTemperature_fn)
    return Nvidia_Error_DlOpen;

  if (Nvidia_nvmlInit_fn() != NVML_SUCCESS)
    return Nvidia_Error_API;

  if (Nvidia_nvmlDeviceGetHandleByIndex_fn(0, &Nvidia_Device) != NVML_SUCCESS)
    return Nvidia_Error_API;

  unsigned int temp; // NOLINT
  if (Nvidia_nvmlDeviceGetTemperature_fn(Nvidia_Device, NVML_TEMPERATURE_GPU, &temp) != NVML_SUCCESS)
    return Nvidia_Error_API;

  return Nvidia_Error_OK;
}

void Nvidia_Close() {
  if (Nvidia_nvmlShutdown_fn)
    Nvidia_nvmlShutdown_fn();

  if (Nvidia_DlHandle)
    dlclose(Nvidia_DlHandle);

  Nvidia_DlHandle = NULL;
  Nvidia_nvmlShutdown_fn = NULL;
}

Error* Nvidia_GetTemperature(float* out) {
  unsigned int temp; // NOLINT

  if (Nvidia_nvmlDeviceGetTemperature_fn(Nvidia_Device, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS)
    *out = temp;
  else
    return err_string(0, "nvidia-ml: Failed to get temperature");

  return err_success();
}
