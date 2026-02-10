#include "regex_utils.h"

#include "memory.h"

#include <string.h> // memcpy
#include <stdlib.h> // strtoll

/*
 * Returns the number of matches of a compiled `regex_t` within a string.
 *
 * NOTE: This function does not handle empty matches (rm_eo == 0) and may
 * loop indefinitely if the regular expression can match an empty string.
 */
size_t RegEx_Count(const regex_t* regex, regmatch_t* matches, size_t nmatches, const char* text) {
  size_t count = 0;

  while (regexec(regex, text, nmatches, matches, 0) == 0) {
    ++count;
    text += matches[0].rm_eo;
  }

  return count;
}

/*
 * Converts the result of a `regmatch_t` to long long using strtoll().
 *
 * NOTE: This function passes a pointer to the start of the match within
 * the original string to strtoll(). Parsing continues beyond the match end
 * and stops only when an invalid character is encountered.
 */
long long RegEx_Strtoll(const regmatch_t* m, const char* s, int base) {
  if (m->rm_so < 0 || m->rm_eo < 0)
    return 0;

  return strtoll(s + m->rm_so, NULL, base);
}

/*
 * Extracts the substring referenced by a `regmatch_t` from the input string.
 * Returns a newly allocated, NUL-terminated string, or NULL if the match
 * is invalid.
 */
char* RegEx_SubStr(const regmatch_t* m, const char* s) {
  if (m->rm_so < 0 || m->rm_eo < 0)
    return NULL;

  size_t len = (size_t) (m->rm_eo - m->rm_so);
  char *out = Mem_Malloc(len + 1);
  memcpy(out, s + m->rm_so, len);
  out[len] = '\0';
  return out;
}

/*
 * Extracts the substring referenced by a `regmatch_t` into a fixed-size buffer.
 * Copies at most `size - 1` characters and always NUL-terminates the output.
 */
void RegEx_SubStr_Fixed(const regmatch_t* m, const char* s, char* out, size_t size) {
  out[0] = '\0';

  if (m->rm_so < 0 || m->rm_eo < 0)
    return;

  size_t len = (size_t) (m->rm_eo - m->rm_so);
  if (len >= size)
    len = size - 1;

  memcpy(out, s + m->rm_so, len);
  out[len] = '\0';
}
