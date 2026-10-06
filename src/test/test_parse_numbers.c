#include "../parse_double.h"
#include "../parse_number.h"
#include "../parse_unumber.h"

#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h> // strcmp
#include <limits.h>
#include <stdbool.h>
#include <float.h>

static bool strcmp_null(const char* a, const char* b) {
  if (a == NULL || b == NULL)
    return (a == NULL && b == NULL);

  return !strcmp(a, b);
}

static void test_parse_double(
  int line,
  const char* in,
  double min,
  double max,
  double expected,
  const char* expected_error)
{
  const char* having_error;
  double having = parse_double(in, min, max, &having_error);

  if (! strcmp_null(having_error, expected_error)) {
    printf("%d: having = %s, expected = %s\n", line, having_error, expected_error);
    exit(1);
  }

  if (having_error)
    return;

  if (having != expected) {
    printf("%d: having = %f, expected = %f\n", line, having, expected);
    exit(1);
  }
}

static void test_parse_number(
  int line,
  const char* in,
  int64_t min,
  int64_t max,
  int64_t expected,
  const char* expected_error)
{
  const char* having_error;
  int64_t having = parse_number(in, min, max, &having_error);

  if (! strcmp_null(having_error, expected_error)) {
    printf("%d: having = %s, expected = %s\n", line, having_error, expected_error);
    exit(1);
  }

  if (having_error)
    return;

  if (having != expected) {
    printf("%d: having = %ld, expected = %ld\n", line, having, expected);
    exit(1);
  }
}

static void test_parse_unumber(
  int line,
  const char* in,
  uint64_t min,
  uint64_t max,
  uint64_t expected,
  const char* expected_error)
{
  const char* having_error;
  uint64_t having = parse_unumber(in, min, max, &having_error);

  if (! strcmp_null(having_error, expected_error)) {
    printf("%d: having = %s, expected = %s\n", line, having_error, expected_error);
    exit(1);
  }

  if (having_error)
    return;

  if (having != expected) {
    printf("%d: having = %ld, expected = %ld\n", line, having, expected);
    exit(1);
  }
}

int main() {
  char int64_min_str[128];
  char int64_max_str[128];
  char uint64_max_str[128];
  snprintf(int64_min_str,  sizeof(int64_min_str),  "%ld", INT64_MIN);
  snprintf(int64_max_str,  sizeof(int64_max_str),  "%ld", INT64_MAX);
  snprintf(uint64_max_str, sizeof(uint64_max_str), "%lu", UINT64_MAX);

#define T(...) test_parse_double(__LINE__, __VA_ARGS__)
  T("0",      -FLT_MAX,     FLT_MAX,      0,    NULL);
  T("-0",     -FLT_MAX,     FLT_MAX,      0,    NULL);
  T("+0",     -FLT_MAX,     FLT_MAX,      0,    NULL);
  T("0.0",    -FLT_MAX,     FLT_MAX,      0,    NULL);
  T("-0.0",   -FLT_MAX,     FLT_MAX,      0,    NULL);
  T("+0.0",   -FLT_MAX,     FLT_MAX,      0,    NULL);

  T("1",      -FLT_MAX,     FLT_MAX,      1,    NULL);
  T("-1",     -FLT_MAX,     FLT_MAX,      -1,   NULL);
  T("+1",     -FLT_MAX,     FLT_MAX,      1,    NULL);
  T("1.0",    -FLT_MAX,     FLT_MAX,      1,    NULL);
  T("-1.0",   -FLT_MAX,     FLT_MAX,      -1,   NULL);
  T("+1.0",   -FLT_MAX,     FLT_MAX,      1,    NULL);

  T("",       -FLT_MAX,     FLT_MAX,      0,    "Invalid argument");
  T("0",      1,            FLT_MAX,      0,    "value too small");
  T("0",      -2,           -1,           0,    "value too large");
  T("NaN",    -FLT_MAX,     FLT_MAX,      0,    "value is not a number");
#undef T

#define T(...) test_parse_number(__LINE__, __VA_ARGS__)
  T("0",      INT64_MIN,    INT64_MAX,    0,    NULL);
  T("-0",     INT64_MIN,    INT64_MAX,    0,    NULL);
  T("+0",     INT64_MIN,    INT64_MAX,    0,    NULL);

  T("1",      INT64_MIN,    INT64_MAX,    1,    NULL);
  T("-1",     INT64_MIN,    INT64_MAX,    -1,   NULL);
  T(int64_min_str, INT64_MIN, INT64_MAX,  INT64_MIN, NULL);
  T(int64_max_str, INT64_MIN, INT64_MAX,  INT64_MAX, NULL);

  T("0b0",    INT64_MIN,    INT64_MAX,    0,    NULL);
  T("0B0",    INT64_MIN,    INT64_MAX,    0,    NULL);
  T("0b1",    INT64_MIN,    INT64_MAX,    1,    NULL);
  T("0B1",    INT64_MIN,    INT64_MAX,    1,    NULL);
  T("+0b1",   INT64_MIN,    INT64_MAX,    1,    NULL);
  T("+0B1",   INT64_MIN,    INT64_MAX,    1,    NULL);
  T("-0b1",   INT64_MIN,    INT64_MAX,    -1,   NULL);
  T("-0B1",   INT64_MIN,    INT64_MAX,    -1,   NULL);

  T("0x0",    INT64_MIN,    INT64_MAX,    0,    NULL);
  T("0X0",    INT64_MIN,    INT64_MAX,    0,    NULL);
  T("0x1",    INT64_MIN,    INT64_MAX,    1,    NULL);
  T("0X1",    INT64_MIN,    INT64_MAX,    1,    NULL);
  T("+0x1",   INT64_MIN,    INT64_MAX,    1,    NULL);
  T("+0X1",   INT64_MIN,    INT64_MAX,    1,    NULL);
  T("-0x1",   INT64_MIN,    INT64_MAX,    -1,   NULL);
  T("-0X1",   INT64_MIN,    INT64_MAX,    -1,   NULL);
  T("0xF",    INT64_MIN,    INT64_MAX,    15,   NULL);
  T("0XF",    INT64_MIN,    INT64_MAX,    15,   NULL);
  T("+0xF",   INT64_MIN,    INT64_MAX,    15,   NULL);
  T("+0XF",   INT64_MIN,    INT64_MAX,    15,   NULL);
  T("-0xF",   INT64_MIN,    INT64_MAX,    -15,  NULL);
  T("-0XF",   INT64_MIN,    INT64_MAX,    -15,  NULL);

  T("",       INT64_MIN,    INT64_MAX,    0,    "Invalid argument");
  T(" 0",     0,            INT64_MAX,    0,    "Invalid argument");
  T("0 ",     0,            INT64_MAX,    0,    "Invalid argument");
  T("00",     INT64_MIN,    INT64_MAX,    0,    "octal values not supported");
  T("01",     0,            INT64_MAX,    0,    "octal values not supported");
  T("02",     0,            INT64_MAX,    0,    "octal values not supported");
  T("03",     0,            INT64_MAX,    0,    "octal values not supported");
  T("04",     0,            INT64_MAX,    0,    "octal values not supported");
  T("05",     0,            INT64_MAX,    0,    "octal values not supported");
  T("06",     0,            INT64_MAX,    0,    "octal values not supported");
  T("07",     0,            INT64_MAX,    0,    "octal values not supported");
  T("08",     0,            INT64_MAX,    0,    "octal values not supported");
  T("09",     0,            INT64_MAX,    0,    "octal values not supported");
  T("0x",     INT64_MIN,    INT64_MAX,    0,    "Invalid argument");
  T("0X",     INT64_MIN,    INT64_MAX,    0,    "Invalid argument");
  T("0b",     INT64_MIN,    INT64_MAX,    0,    "Invalid argument");
  T("0B",     INT64_MIN,    INT64_MAX,    0,    "Invalid argument");
  T("0",      1,            INT64_MAX,    0,    "value too small");
  T("0",      -2,           -1,           0,    "value too large");
#undef T

#define T(...) test_parse_unumber(__LINE__, __VA_ARGS__)
  T("0",      0,            UINT64_MAX,   0,    NULL);
  T("-0",     0,            UINT64_MAX,   0,    NULL);
  T("+0",     0,            UINT64_MAX,   0,    NULL);

  T("1",      0,            UINT64_MAX,   1,    NULL);
  T("+1",     0,            UINT64_MAX,   1,    NULL);
  T(uint64_max_str, 0,      UINT64_MAX,   UINT64_MAX, NULL);

  T("0b0",    0,            UINT64_MAX,   0,    NULL);
  T("0B0",    0,            UINT64_MAX,   0,    NULL);
  T("0b1",    0,            UINT64_MAX,   1,    NULL);
  T("0B1",    0,            UINT64_MAX,   1,    NULL);
  T("+0b1",   0,            UINT64_MAX,   1,    NULL);
  T("+0B1",   0,            UINT64_MAX,   1,    NULL);

  T("0x0",    0,            UINT64_MAX,   0,    NULL);
  T("0X0",    0,            UINT64_MAX,   0,    NULL);
  T("0x1",    0,            UINT64_MAX,   1,    NULL);
  T("0X1",    0,            UINT64_MAX,   1,    NULL);
  T("+0x1",   0,            UINT64_MAX,   1,    NULL);
  T("+0X1",   0,            UINT64_MAX,   1,    NULL);
  T("0xF",    0,            UINT64_MAX,   15,   NULL);
  T("0XF",    0,            UINT64_MAX,   15,   NULL);
  T("+0xF",    0,           UINT64_MAX,   15,   NULL);
  T("+0XF",    0,           UINT64_MAX,   15,   NULL);

  T("",       0,            UINT64_MAX,   0,    "Invalid argument");
  T(" 0",      0,           UINT64_MAX,   0,    "Invalid argument");
  T("0 ",      0,           UINT64_MAX,   0,    "Invalid argument");
  T("00",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("01",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("02",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("03",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("04",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("05",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("06",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("07",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("08",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("09",     0,            UINT64_MAX,   0,    "octal values not supported");
  T("0x",     0,            UINT64_MAX,   0,    "Invalid argument");
  T("0X",     0,            UINT64_MAX,   0,    "Invalid argument");
  T("0b",     0,            UINT64_MAX,   0,    "Invalid argument");
  T("0B",     0,            UINT64_MAX,   0,    "Invalid argument");
  T("-1",     0,            UINT64_MAX,   -1,   "value is negative");
  T("0",      1,            UINT64_MAX,   0,    "value too small");
  T("9",      0,            1,            0,    "value too large");
#undef T

  return 0;
}
