#include "str_functions.h"

#include <ctype.h>  // tolower
#include <string.h> // strlen, strncmp, memcpy

#include "../memory.h"

const char* str_from_bool(bool val) {
  return val ? "true" : "false";
}

char* str_to_lower(const char* a) {
  char* b = Mem_Strdup(a);
  for (char* c = b; *c; ++c)
    *c = (char) tolower(*c);
  return b;
}

char* str_replace_prefix(const char* s, const char* search, const char* replace)
{
  size_t search_len = strlen(search);

  if (strncmp(s, search, search_len) != 0)
    return Mem_Strdup(s);

  size_t replace_len = strlen(replace);
  size_t suffix_len = strlen(s + search_len);

  char* result = Mem_Malloc(suffix_len + replace_len + 1);

  memcpy(result, replace, replace_len);
  memcpy(result + replace_len, s + search_len, suffix_len + 1);

  return result;
}

int str_cmp_ignorecase(const char* a, const char* b) {
  while (*a && *b) {
    const int ca = tolower(*a);
    const int cb = tolower(*b);

    if (ca != cb)
      return ca - cb;

    ++a;
    ++b;
  }

  return (unsigned char)*a - (unsigned char)*b;
}

bool str_starts_with_ignorecase(const char* string, const char* prefix) {
  for (;*prefix; prefix++, string++)
    if (tolower(*string) != tolower(*prefix))
      return false;
  return true;
}

size_t str_rstrip_whitespace(char* s, size_t len) {
  if (! len)
    return 0;

  while (len > 0 && ((unsigned char) s[len - 1]) <= 32)
    --len;

  s[len] = '\0';
  return len;
}

size_t str_count_newlines(const char* s) {
  size_t count = 0;

  for (; *s; ++s)
    count += (*s == '\n');

  return count;
}

static size_t levenshtein_min(size_t a, size_t b, size_t c) {
  if (a <= b && a <= c) {
    return a;
  }
  else if (b <= a && b <= c) {
    return b;
  }
  else {
    return c;
  }
}

static size_t levenshtein(const char* s1, size_t s1len, const char* s2, size_t s2len) {
  const size_t rows = s2len + 1;
  const size_t cols = s1len + 1;
  size_t* matrix = Mem_Calloc(rows * cols, sizeof(size_t));
  size_t x, y;

#define M(i, j) matrix[(i) * cols + (j)]

  M(0, 0) = 0;

  for (x = 1; x <= s2len; x++)
    M(x, 0) = M(x-1, 0) + 1;

  for (y = 1; y <= s1len; y++)
    M(0, y) = M(0, y-1) + 1;

  for (x = 1; x <= s2len; x++)
    for (y = 1; y <= s1len; y++)
      M(x, y) = levenshtein_min(
        M(x-1, y) + 1,
        M(x, y-1) + 1,
        M(x-1, y-1) + (s1[y-1] == s2[x-1] ? 0 : 1)
      );

  size_t result = M(s2len, s1len);

#undef M

  Mem_Free(matrix);

  return result;
}

float str_similarity(const char* s1, const char* s2) {
  const size_t s1len = strlen(s1);
  const size_t s2len = strlen(s2);
  const size_t diff = levenshtein(s1, s1len, s2, s2len);
  if (s1len > s2len)
    return 1.0f - ((float) diff / (float) s1len);
  else
    return 1.0f - ((float) diff / (float) s2len);
}
