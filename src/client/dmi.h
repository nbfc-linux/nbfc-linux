#ifndef NBFC_DMI_H_
#define NBFC_DMI_H_

#include <stdbool.h>

const char* DMI_GetSystemProduct(void);
const char* DMI_GetSystemVendor(void);
const char* DMI_GetModelName(void);
bool        DMI_ModelNameEquals(const char*, const char*);

#endif
