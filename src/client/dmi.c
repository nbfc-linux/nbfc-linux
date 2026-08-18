#include "dmi.h"

#include <stdio.h>  // snprintf
#include <string.h> // strerror, strcmp, strncmp, strcspn
#include <stdlib.h> // exit
#include <errno.h>  // errno, ENODATA

#include "../nbfc.h"
#include "../log.h"
#include "../memory.h"
#include "../file_utils.h"
#include "../str_functions.h"

#define DMI_Directory       "/sys/devices/virtual/dmi/id"
#define DMI_ProductNameFile DMI_Directory "/product_name"
#define DMI_SysVendorFile   DMI_Directory "/sys_vendor"

struct DMI_Vendor_Alias {
  const char* vendor;
  const char* alias;
};

static const struct DMI_Vendor_Alias DMI_Vendor_Aliases[] = {
  { "Hewlett-Packard",       "HP"   },
  { "ASUSTeK COMPUTER INC.", "Asus" },
  { NULL, NULL }
};

static const char* DMI_Find_Vendor_Alias(const char* vendor) {
  for (const struct DMI_Vendor_Alias* a = DMI_Vendor_Aliases; a->vendor; ++a) {
    if (! strcmp(a->vendor, vendor))
      return a->alias;
  }

  return NULL;
}

static char* DMI_Replace_Vendor_Alias(const char* model_name) {
  for (const struct DMI_Vendor_Alias* a = DMI_Vendor_Aliases; a->vendor; ++a) {
    if (! strncmp(model_name, a->vendor, strlen(a->vendor))) {
      return str_replace_prefix(model_name, a->vendor, a->alias);
    }
  }

  return Mem_Strdup(model_name);
}

bool DMI_Model_Name_Equals(const char* a, const char* b) {
  char* a_replaced = DMI_Replace_Vendor_Alias(a);
  char* b_replaced = DMI_Replace_Vendor_Alias(b);
  const bool equals = !str_cmp_ignorecase(a_replaced, b_replaced);
  Mem_Free(a_replaced);
  Mem_Free(b_replaced);
  return equals;
}

const char* DMI_Get_System_Product(void) {
  static char buf[128];

  if (! slurp_file(buf, sizeof(buf), DMI_ProductNameFile).ok)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get product name. Failed to read " DMI_ProductNameFile ": %s", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* DMI_Get_System_Vendor(void) {
  static char buf[128];

  if (! slurp_file(buf, sizeof(buf), DMI_SysVendorFile).ok)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get system vendor. Failed to read " DMI_SysVendorFile ": %s", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* DMI_Get_Model_Name(void) {
  static char model_name[256];

  const char* product = DMI_Get_System_Product();
  const char* vendor  = DMI_Get_System_Vendor();
  const char* vendor_alias = DMI_Find_Vendor_Alias(vendor);

  if (vendor_alias)
    vendor = vendor_alias;

  if (str_starts_with_ignorecase(product, vendor))
    snprintf(model_name, sizeof(model_name), "%s", product);
  else
    snprintf(model_name, sizeof(model_name), "%s %s", vendor, product);

  return model_name;
}
