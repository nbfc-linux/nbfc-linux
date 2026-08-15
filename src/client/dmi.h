#ifndef NBFC_DMI_H_
#define NBFC_DMI_H_

#include <stdbool.h>

const char* DMI_Get_System_Product(void);
const char* DMI_Get_System_Vendor(void);
const char* DMI_Get_Model_Name(void);
bool        DMI_Model_Name_Equals(const char*, const char*);

#endif
