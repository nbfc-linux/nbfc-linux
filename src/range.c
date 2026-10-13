#include "range.h"

#include "parse_unumber.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

static bool Range_IsNumChar(char c) {
  return (
    (c >= '0' && c <= '9') ||
    (c >= 'a' && c <= 'f') ||
    (c >= 'A' && c <= 'F') ||
    (c == 'x' || c == 'X')
  );
}

static bool Range_AddOverflow(uint64_t a, uint64_t b, uint64_t* result) {
#if defined(__has_builtin)
  #if __has_builtin(__builtin_add_overflow)
    return __builtin_add_overflow(a, b, result);
  #endif
#elif defined(__GNUC__)
  return __builtin_add_overflow(a, b, result);
#endif

  if (UINT64_MAX - a < b)
    return true;

  *result = a + b;
  return false;
}

static bool Range_SubOverflow(uint64_t a, uint64_t b, uint64_t* result) {
#if defined(__has_builtin)
  #if __has_builtin(__builtin_sub_overflow)
    return __builtin_sub_overflow(a, b, result);
  #endif
#elif defined(__GNUC__)
  return __builtin_sub_overflow(a, b, result);
#endif

  if (a < b)
    return true;

  *result = a - b;
  return false;
}

static bool Range_MulOverflow(uint64_t a, uint64_t b, uint64_t* result) {
#if defined(__has_builtin)
  #if __has_builtin(__builtin_mul_overflow)
    return __builtin_mul_overflow(a, b, result);
  #endif
#elif defined(__GNUC__)
  return __builtin_mul_overflow(a, b, result);
#endif

  if (b != 0 && a > UINT64_MAX / b)
    return true;

  *result = a * b;
  return false;
}

static bool Range_DivOverflow(uint64_t a, uint64_t b, uint64_t* result) {
  if (b == 0)
    return true;

  *result = a / b;
  return false;
}

uint64_t Range_Begin(const Range* range) {
  return range->start;
}

bool Range_Iterate(const Range* range, uint64_t* val) {
  // Single value
  if (range->step == 0)
    return false;

  switch (range->type) {
  case Range_Add:
    if (Range_AddOverflow(*val, range->step, val))
      return false;
    if (*val > range->stop)
      return false;
    return true;

  case Range_Sub:
    if (Range_SubOverflow(*val, range->step, val))
      return false;
    if (*val < range->stop)
      return false;
    return true;

  case Range_Mul:
    if (Range_MulOverflow(*val, range->step, val))
      return false;
    if (*val > range->stop)
      return false;
    return true;

  case Range_Div:
    if (*val == 0)
      return false;
    if (Range_DivOverflow(*val, range->step, val))
      return false;
    if (*val < range->stop)
      return false;
    return true;
  }

  return false;
}

bool Range_IsInRange(const Range* range, uint64_t min, uint64_t max) {
  if (range->start < min || range->start > max)
    return false;

  if (range->stop < min || range->stop > max)
    return false;

  return true;
}

Error Range_Parse(const char* s, Range* out) {
  char num1[128] = {0};
  char sep1      = '\0';
  char num2[128] = {0};
  char sep2      = '\0';
  char type      = '\0';
  char num3[128] = {0};
  uint64_t integer1 = 0;
  uint64_t integer2 = 0;
  uint64_t integer3 = 0;
  const char* err;
  size_t i;

  if (!*s)
    return err_string("Empty range");

  // ==========================================================================
  // Parse the string into numbers and separators.
  // ==========================================================================

  // Parse first number
  for (i = 0; Range_IsNumChar(*s); ++i, ++s)
    if (i >= sizeof(num1) - 1) {
      errno = ERANGE;
      return err_stdlib(num1);
    }
    else
      num1[i] = *s;

  // Parse first separator
  if (*s == '\0')
    goto end_of_parsing;
  sep1 = *s;
  ++s;

  // Parse second number
  for (i = 0; Range_IsNumChar(*s); ++i, ++s)
    if (i >= sizeof(num2) - 1) {
      errno = ERANGE;
      return err_stdlib(num2);
    }
    else
      num2[i] = *s;

  // Parse second separator
  if (*s == '\0')
    goto end_of_parsing;
  sep2 = *s;
  ++s;

  // Parse type
  if (*s == '\0')
    goto end_of_parsing;

  if (Range_IsNumChar(*s))
    type = '+';
  else {
    type = *s;
    ++s;
  }

  // Parse third number
  for (i = 0; Range_IsNumChar(*s); ++i, ++s)
    if (i >= sizeof(num3) - 1) {
      errno = ERANGE;
      return err_stdlib(num3);
    }
    else
      num3[i] = *s;

  // Check for end of string
  if (*s)
    return err_string("Unexpected characters at end of string");

  // ==========================================================================
  // Parse the buffers into integers
  // ==========================================================================

end_of_parsing:

  if (*num1) {
    integer1 = parse_unumber(num1, 0, UINT64_MAX, &err);
    if (err)
      return err_stringf("%s: %s", num1, err);
  }

  if (*num2) {
    integer2 = parse_unumber(num2, 0, UINT64_MAX, &err);
    if (err)
      return err_stringf("%s: %s", num2, err);
  }

  if (*num3) {
    integer3 = parse_unumber(num3, 0, UINT64_MAX, &err);
    if (err)
      return err_stringf("%s: %s", num3, err);
  }

  // ==========================================================================
  // Create the range objects
  // ==========================================================================

  // <value>
  if (*num1 && (sep1 + *num2 + sep2 + *num3) == 0) {
    out->start = integer1;
    out->stop = integer1;
    out->step = 0;
    out->type = Range_Add;
    return err_success();
  }

  // <start> - <stop> : (+|-|*|/) <step>
  if (*num1 && sep1 == '-' && *num2 && sep2 == ':' && *num3) {
    out->start = integer1;
    out->stop = integer2;
    out->step = integer3;

    if (out->step == 0)
      return err_stringf("%c%s: Invalid step", type, num3);

    if (type == '-') {
      if (integer1 < integer2)
        return err_stringf("start < stop (step = %c%" PRIu64 ")", type, integer3);

      out->type = Range_Sub;
    }
    else if (type == '+') {
      if (integer1 > integer2)
        return err_stringf("start > stop (step = %c%" PRIu64 ")", type, integer3);

      out->type = Range_Add;
    }
    else if (type == '/') {
      if (integer3 == 1)
        return err_stringf("%c%s: Invalid step", type, num3);

      if (integer1 == 0)
        return err_stringf("Invalid start (step = %c%" PRIu64 "): 0", type, integer3);

      if (integer1 < integer2)
        return err_stringf("start < stop (step = %c%" PRIu64 ")", type, integer3);

      out->type = Range_Div;
    }
    else if (type == '*') {
      if (integer3 == 1)
        return err_stringf("%c%s: Invalid step", type, num3);

      if (integer1 == 0)
        return err_stringf("Invalid start (step = %c%" PRIu64 "): 0", type, integer3);

      if (integer1 > integer2)
        return err_stringf("start > stop (step = %c%" PRIu64 ")", type, integer3);

      out->type = Range_Mul;
    }
    else
      return err_stringf("%c%s: Invalid step", type, num3);

    return err_success();
  }

  // <start> - <stop>
  if (*num1 && sep1 == '-' && *num2 && (sep2 + *num3) == 0) {
    out->start = integer1;
    out->stop = integer2;
    out->step = 1;

    if (integer1 == integer2) {
      out->step = 0; // Single value
      out->type = Range_Add;
    }
    else if (integer1 < integer2)
      out->type = Range_Add;
    else
      out->type = Range_Sub;

    return err_success();
  }

  return err_string("Invalid range");
}
