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

#define DMI_BASE_DIRECTORY    "/sys/devices/virtual/dmi/id"
#define DMI_PRODUCT_NAME_FILE DMI_BASE_DIRECTORY "/product_name"
#define DMI_SYS_VENDOR_FILE   DMI_BASE_DIRECTORY "/sys_vendor"

struct DMI_VendorAlias {
  const char* vendor;
  const char* alias;
};

static const struct DMI_VendorAlias DMI_VendorAliases[] = {
  { "Hewlett-Packard",       "HP"   },
  { "ASUSTeK COMPUTER INC.", "Asus" },
  { NULL, NULL }
};

static const char* DMI_FindVendorAlias(const char* vendor) {
  for (const struct DMI_VendorAlias* a = DMI_VendorAliases; a->vendor; ++a) {
    if (! strcmp(a->vendor, vendor))
      return a->alias;
  }

  return NULL;
}

static char* DMI_ReplaceVendorAlias(const char* model_name) {
  for (const struct DMI_VendorAlias* a = DMI_VendorAliases; a->vendor; ++a) {
    if (! strncmp(model_name, a->vendor, strlen(a->vendor))) {
      return str_replace_prefix(model_name, a->vendor, a->alias);
    }
  }

  return Mem_Strdup(model_name);
}

bool DMI_ModelNameEquals(const char* a, const char* b) {
  char* a_replaced = DMI_ReplaceVendorAlias(a);
  char* b_replaced = DMI_ReplaceVendorAlias(b);
  const bool equals = !str_cmp_ignorecase(a_replaced, b_replaced);
  Mem_Free(a_replaced);
  Mem_Free(b_replaced);
  return equals;
}

const char* DMI_GetSystemProduct(void) {
  static char buf[128];

  if (! File_Read(buf, sizeof(buf), DMI_PRODUCT_NAME_FILE).ok)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get product name. Failed to read " DMI_PRODUCT_NAME_FILE ": %s", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* DMI_GetSystemVendor(void) {
  static char buf[128];

  if (! File_Read(buf, sizeof(buf), DMI_SYS_VENDOR_FILE).ok)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get system vendor. Failed to read " DMI_SYS_VENDOR_FILE": %s", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

const char* DMI_GetModelName(void) {
  static char model_name[256];

  const char* product = DMI_GetSystemProduct();
  const char* vendor  = DMI_GetSystemVendor();
  const char* vendor_alias = DMI_FindVendorAlias(vendor);

  if (vendor_alias)
    vendor = vendor_alias;

  if (str_starts_with_ignorecase(product, vendor))
    snprintf(model_name, sizeof(model_name), "%s", product);
  else
    snprintf(model_name, sizeof(model_name), "%s %s", vendor, product);

  return model_name;
}
