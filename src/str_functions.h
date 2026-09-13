#ifndef NBFC_STR_FUNCTIONS_H_
#define NBFC_STR_FUNCTIONS_H_

#include <stddef.h>
#include <stdbool.h>

const char* str_from_bool(bool);
char*       str_to_lower(const char*);
char*       str_replace_prefix(const char*, const char*, const char*);
int         str_cmp_ignorecase(const char*, const char*);
bool        str_starts_with_ignorecase(const char*, const char*);
size_t      str_rstrip_whitespace(char*, size_t);
size_t      str_count_newlines(const char*);
float       str_similarity(const char*, const char*);

#endif
