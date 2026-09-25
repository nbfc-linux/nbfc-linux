#include "dmi.h"

#include <stdio.h>  // snprintf
#include <string.h> // strerror, strcmp, strncmp, strcspn
#include <errno.h>  // errno, ENODATA

#include "nbfc.h"
#include "memory.h"
#include "file_utils.h"
#include "str_functions.h"

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

char* DMI_ReplaceVendorAlias(const char* model_name) {
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

Error DMI_GetSystemProduct(char* out, size_t len) {
  if (! File_Read(out, len, DMI_PRODUCT_NAME_FILE).ok)
    goto error;

  out[strcspn(out, "\n")] = '\0';

  if (*out == '\0') {
    errno = ENODATA;
    goto error;
  }

  return err_success();

error:
  return err_stdlib(DMI_PRODUCT_NAME_FILE);
}

Error DMI_GetSystemVendor(char* out, size_t len) {
  if (! File_Read(out, len, DMI_SYS_VENDOR_FILE).ok)
    goto error;

  out[strcspn(out, "\n")] = '\0';

  if (*out == '\0') {
    errno = ENODATA;
    goto error;
  }

  return err_success();

error:
  return err_stdlib(DMI_SYS_VENDOR_FILE);
}

Error DMI_GetModelName(char* out, size_t len) {
  Error e;
  char vendor[DMI_MAX_VENDOR_LEN];
  char product[DMI_MAX_PRODUCT_LEN];

  e = DMI_GetSystemVendor(vendor, sizeof(vendor));
  if (e)
    return e;

  e = DMI_GetSystemProduct(product, sizeof(product));
  if (e)
    return e;

  if (str_starts_with_ignorecase(product, vendor)) {
    snprintf(out, len, "%s", product);
    return err_success();
  }

  const char* vendor_alias = DMI_FindVendorAlias(vendor);
  if (! vendor_alias) {
    snprintf(out, len, "%s %s", vendor, product);
    return err_success();
  }

  if (str_starts_with_ignorecase(product, vendor_alias)) {
    snprintf(out, len, "%s", product);
    return err_success();
  }

  snprintf(out, len, "%s %s", vendor_alias, product);
  return err_success();
}
