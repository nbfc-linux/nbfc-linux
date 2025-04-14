#ifndef STR_FUNCTIONS_H_
#define STR_FUNCTIONS_H_

#include <stdbool.h>

const char* bool_to_str(bool);
char*       str_to_lower(const char*);
bool        str_starts_with_ignorecase(const char*, const char*);
float       str_similarity(const char*, const char*);

#endif
