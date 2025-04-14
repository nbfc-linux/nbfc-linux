#include "str_functions.h"

#include <ctype.h>  // tolower
#include <string.h> // strlen

#include "../memory.h"

const char* bool_to_str(bool val) {
  static const char strings[2][6] = {"false", "true"};
  return strings[val];
}

char *str_to_lower(const char *a) {
  char* b = Mem_Strdup(a);
  for (char* c = b; *c; ++c)
    *c = tolower(*c);
  return b;
}

bool str_starts_with_ignorecase(const char* string, const char* prefix) {
  for (;*prefix; prefix++, string++)
    if (tolower(*string) != tolower(*prefix))
      return false;
  return true;
}

static int levenshtein_min(int a, int b, int c) {
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

static int levenshtein(const char *s1, const char *s2) {
  unsigned int x, y, s1len, s2len;
  s1len = strlen(s1);
  s2len = strlen(s2);
  unsigned int matrix[s2len+1][s1len+1];
  matrix[0][0] = 0;
  for (x = 1; x <= s2len; x++)
    matrix[x][0] = matrix[x-1][0] + 1;
  for (y = 1; y <= s1len; y++)
    matrix[0][y] = matrix[0][y-1] + 1;
  for (x = 1; x <= s2len; x++)
    for (y = 1; y <= s1len; y++)
      matrix[x][y] = levenshtein_min(matrix[x-1][y] + 1, matrix[x][y-1] + 1, matrix[x-1][y-1] + (s1[y-1] == s2[x-1] ? 0 : 1));

  return(matrix[s2len][s1len]);
}

float str_similarity(const char* s1, const char* s2) {
  const int s1len = strlen(s1);
  const int s2len = strlen(s2);
  const int diff = levenshtein(s1, s2);
  if (s1len > s2len)
    return 1.0f - ((float) diff / s1len);
  else
    return 1.0f - ((float) diff / s2len);
}
