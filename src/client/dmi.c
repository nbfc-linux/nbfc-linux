#include "dmi.h"

#include <stdio.h>  // snprintf
#include <string.h> // strerror, strcmp, strcspn
#include <stdlib.h> // exit
#include <errno.h>  // errno, ENODATA

#include "../nbfc.h"
#include "../log.h"
#include "../file_utils.h"
#include "str_functions.h"

#define DmiIdDirectoryPath "/sys/devices/virtual/dmi/id"

const char* get_system_product() {
  static char buf[128];

  if (slurp_file(buf, sizeof(buf), DmiIdDirectoryPath "/product_name") == -1)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get product name. Failed to read " DmiIdDirectoryPath "/product_name: %s\n", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* get_system_vendor() {
  static char buf[128];

  if (slurp_file(buf, sizeof(buf), DmiIdDirectoryPath "/sys_vendor") == -1)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get system vendor. Failed to read " DmiIdDirectoryPath "/sys_vendor: %s\n", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* get_model_name() {
  static char model_name[256];

  struct vendor_alias { const char* key; const char* value; };

  const struct vendor_alias vendor_aliases[] = {
    { "Hewlett-Packard", "HP" },
    { NULL, NULL }
  };

  const char* product = get_system_product();
  const char* vendor = get_system_vendor();

  for (const struct vendor_alias* alias = vendor_aliases; alias->key; ++alias) {
    if (! strcmp(vendor, alias->key)) {
      vendor = alias->value;
      break;
    }
  }

  if (str_starts_with_ignorecase(product, vendor))
    snprintf(model_name, sizeof(model_name), "%s", product);
  else
    snprintf(model_name, sizeof(model_name), "%s %s", vendor, product);

  return model_name;
}
