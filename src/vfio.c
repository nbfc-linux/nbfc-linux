#include "vfio.h"

#include "log.h"
#include "file_utils.h"
#include "str_functions.h"

#include <stdio.h>  // snprintf
#include <dirent.h> // DIR, opendir, readdir, closedir
#include <string.h> // strstr, strcmp, strrchr
#include <unistd.h> // readlink

// ============================================================================
// VFIO passthrough detection helpers
// ============================================================================

#define VFIO_PCI_DEVICES_PATH "/sys/bus/pci/devices"

// Check /proc/cmdline for vfio-pci.ids or vfio_pci.ids (fast path).
bool VFIO_CheckProcCmdline(void) {
  char cmdline[4096];
  FileResult res = File_Read(cmdline, sizeof(cmdline), "/proc/cmdline");
  if (!res.ok)
    return false;

  return strstr(cmdline, "vfio-pci.ids") || strstr(cmdline, "vfio_pci.ids");
}

// Scan /sys/bus/pci/devices/ for NVIDIA GPUs bound to vfio-pci or pci-stub.
bool VFIO_CheckSysBusPciDevices(void) {
  DIR* dir = opendir(VFIO_PCI_DEVICES_PATH);
  if (!dir) {
    Log_Debug("Could not open " VFIO_PCI_DEVICES_PATH
              " — cannot check PCI device bindings");
    return false;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.')
      continue;

    // Read the vendor file for this PCI device
    char vendor_path[PATH_MAX];
    snprintf(vendor_path, sizeof(vendor_path),
             "%s/%s/vendor", VFIO_PCI_DEVICES_PATH, entry->d_name);

    char vendor[8];
    FileResult res = File_Read(vendor, sizeof(vendor), vendor_path);
    if (!res.ok)
      continue;

    str_rstrip_whitespace(vendor, res.len);

    // NVIDIA PCI vendor ID is 0x10de / 10de
    if (!strstr(vendor, "10de"))
      continue;

    // Check if this NVIDIA device is bound to vfio-pci or pci-stub
    char driver_path[PATH_MAX];
    snprintf(driver_path, sizeof(driver_path),
             "%s/%s/driver", VFIO_PCI_DEVICES_PATH, entry->d_name);

    char driver_target[PATH_MAX];
    ssize_t linklen = readlink(driver_path, driver_target, sizeof(driver_target) - 1);
    if (linklen <= 0)
      continue;

    driver_target[linklen] = '\0';

    // Extract driver name (basename of the target path)
    const char* driver_name = strrchr(driver_target, '/');
    if (driver_name)
      driver_name++;
    else
      driver_name = driver_target;

    if (!strcmp(driver_name, "vfio-pci") || !strcmp(driver_name, "pci-stub")) {
      Log_Info("NVIDIA GPU at %s bound to '%s' —"
               " VFIO passthrough detected, skipping nvidia-ml sensor",
               entry->d_name, driver_name);
      closedir(dir);
      return true;
    }
  }

  closedir(dir);
  return false;
}
