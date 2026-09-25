#include "hardware_check.h"

#include "dmi.h"
#include "log.h"
#include "nbfc.h"
#include "file_utils.h"
#include "str_functions.h"

#include <errno.h>  // errno
#include <stdio.h>  // snprintf
#include <stdlib.h> // exit
#include <string.h> // strerror

/*
 * Returns the notebook model name.
 *
 * We intentionally re-implement DMI_GetModelName() here instead of using the
 * DMI function, because the latter resolves vendor aliases.
 * This could cause the hardware check to produce different results if the
 * vendor aliases change in the future.
 */
static Error HardwareCheck_GetModelName(char* out, size_t len) {
  Error e;
  char vendor[DMI_MAX_VENDOR_LEN];
  char product[DMI_MAX_PRODUCT_LEN];

  e = DMI_GetSystemVendor(vendor, sizeof(vendor));
  if (e)
    return e;

  e = DMI_GetSystemProduct(product, sizeof(product));
  if (e)
    return e;

  snprintf(out, len, "%s %s", vendor, product);
  return err_success();
}

void HardwareCheck(void) {
  Error e;
  FileResult res;
  char content[DMI_MAX_MODEL_NAME_LEN];
  char model_name[DMI_MAX_MODEL_NAME_LEN];

  e = HardwareCheck_GetModelName(model_name, sizeof(model_name));
  if (e) {
    Log_Error("%s", err_print_all(e));
    exit(NBFC_EXIT_FAILURE);
  }

  res = File_Read(content, sizeof(content), NBFC_CURRENT_MODEL_NAME_FILE);
  if (res.ok) {
    str_rstrip_whitespace(content, res.len);

    if (! strcmp(model_name, content))
      return;

    Log_Error("The notebook model has changed from \"%s\" to \"%s\"", content, model_name);
    Log_Error("The service will refuse to start because the current notebook "
              "model does not match the hardware the existing configuration "
              "was created for");
    Log_Error("This is a security mechanism that ensures that an NBFC "
              "installation only runs on the hardware it was configured for");
    Log_Error("You may want to reconfigure your NBFC installation");
    Log_Error("To proceed, please remove the file \"%s\"", NBFC_CURRENT_MODEL_NAME_FILE);
    exit(NBFC_EXIT_FAILURE);
  }
  else if (errno == ENOENT) {
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    const int flags = O_CREAT | O_WRONLY | O_TRUNC;
    res = File_Write(NBFC_CURRENT_MODEL_NAME_FILE, flags, mode, model_name, strlen(model_name));
    if (! res.ok) {
      Log_Error("%s: %s", NBFC_CURRENT_MODEL_NAME_FILE, strerror(errno));
      exit(NBFC_EXIT_FAILURE);
    }
  }
  else {
    Log_Error("%s: %s", NBFC_CURRENT_MODEL_NAME_FILE, strerror(errno));
    exit(NBFC_EXIT_FAILURE);
  }
}
