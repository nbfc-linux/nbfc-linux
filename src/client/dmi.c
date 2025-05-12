#include "dmi.h"

#include <stdio.h>  // snprintf
#include <string.h> // strerror, strcmp, strcspn
#include <stdlib.h> // exit
#include <errno.h>  // errno, ENODATA

#include "../nbfc.h"
#include "../log.h"
#include "../file_utils.h"
#include "str_functions.h"

#define DMI_Directory       "/sys/devices/virtual/dmi/id"
#define DMI_ProductNameFile DMI_Directory "/product_name"
#define DMI_SysVendorFile   DMI_Directory "/sys_vendor"

const char* DMI_Get_System_Product() {
  static char buf[128];

  if (slurp_file(buf, sizeof(buf), DMI_ProductNameFile) == -1)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get product name. Failed to read " DMI_ProductNameFile ": %s\n", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* DMI_Get_System_Vendor() {
  static char buf[128];

  if (slurp_file(buf, sizeof(buf), DMI_SysVendorFile) == -1)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get system vendor. Failed to read " DMI_SysVendorFile ": %s\n", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* DMI_Get_Model_Name() {
  static char model_name[256];

  struct vendor_alias { const char* key; const char* value; };

  const struct vendor_alias vendor_aliases[] = {
    { "Hewlett-Packard", "HP" },
    { NULL, NULL }
  };

  const char* product = DMI_Get_System_Product();
  const char* vendor  = DMI_Get_System_Vendor();

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
