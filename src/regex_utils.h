#ifndef NBFC_REGEX_UTILS_H_
#define NBFC_REGEX_UTILS_H_

#include <regex.h>

#define REGEX_WHITESPACE      "[[:space:]]+"
#define REGEX_HEX             "[[:xdigit:]]+"
#define REGEX_HEX_WITH_PREFIX "0[xX]" REGEX_HEX
#define REGEX_GROUP(PATTERN)  "(" PATTERN ")"

size_t    RegEx_Count(const regex_t*, regmatch_t*, size_t, const char*);
char*     RegEx_SubStr(const regmatch_t*, const char*);
void      RegEx_SubStr_Fixed(const regmatch_t*, const char*, char*, size_t);
long long RegEx_Strtoll(const regmatch_t*, const char*, int);

#endif
