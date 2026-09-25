#ifndef NBFC_DMI_H_
#define NBFC_DMI_H_

#include "error.h"

#include <stddef.h> // size_t
#include <stdbool.h>

#define DMI_MAX_VENDOR_LEN     128
#define DMI_MAX_PRODUCT_LEN    128
#define DMI_MAX_MODEL_NAME_LEN (DMI_MAX_VENDOR_LEN + DMI_MAX_PRODUCT_LEN + 1)

Error DMI_GetSystemVendor(char*, size_t);
Error DMI_GetSystemProduct(char*, size_t);
Error DMI_GetModelName(char*, size_t);
bool  DMI_ModelNameEquals(const char*, const char*);
char* DMI_ReplaceVendorAlias(const char*);

#endif
