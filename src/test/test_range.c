#include "../range.c"
#include "../error.c"
#include "../nxjson.c"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

static void Test_Range_Parse(
  int line,
  const char* s,
  const char* expected_error,
  Range expected_range)
{
  Error e;
  Range having_range;
  const char* having_error;

  e = Range_Parse(s, &having_range);

  if (expected_error) {
    if (! e) {
      printf("%d: Expected error: %s\n", line, expected_error);
      exit(1);
    }

    having_error = err_print_all(e);
    if (strcmp(having_error, expected_error)) {
      printf("%d: Expected error: %s, Having error: %s\n", line, expected_error, having_error);
      exit(1);
    }

    return;
  }

  if (e) {
    having_error = err_print_all(e);
    printf("%d: Error: %s\n", line, having_error);
    exit(1);
  }

  if (having_range.type != expected_range.type) {
    printf("%d: Expected type != Having type\n", line);
    exit(1);
  }

  if (having_range.start != expected_range.start) {
    printf("%d: Expected start (%" PRIu64 ") != Having start (%" PRIu64 ")\n",
        line, expected_range.start, having_range.start);
    exit(1);
  }

  if (having_range.stop != expected_range.stop) {
    printf("%d: Expected stop (%" PRIu64 ") != Having stop (%" PRIu64 ")\n",
        line, expected_range.stop, having_range.stop);
    exit(1);
  }

  if (having_range.step != expected_range.step) {
    printf("%d: Expected step (%" PRIu64 ") != Having step (%" PRIu64 ")\n",
        line, expected_range.step, having_range.step);
    exit(1);
  }
}

static void Test_Range_Iterate(void) {
  Range range;
  uint64_t val;

  val = 0;
  range = (Range) {0, 0, 0, Range_Add};
  assert(! Range_Iterate(&range, &val));

  val = 0;
  range = (Range) {0, 0, 1, Range_Add};
  assert(! Range_Iterate(&range, &val));

  val = 0;
  range = (Range) {0, 0, 1, Range_Sub};
  assert(! Range_Iterate(&range, &val));

  val = UINT64_MAX;
  range = (Range) {0, UINT64_MAX, 1, Range_Add};
  assert(! Range_Iterate(&range, &val));

  val = 0;
  range = (Range) {0, 1, 1, Range_Add};
  assert(Range_Iterate(&range, &val));
  assert(val == 1);
  assert(! Range_Iterate(&range, &val));

  val = 1;
  range = (Range) {1, 0, 1, Range_Sub};
  assert(Range_Iterate(&range, &val));
  assert(val == 0);
  assert(! Range_Iterate(&range, &val));
}

static const char* F(const char* fmt, ...) {
  static char buf[512];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  return buf;
}

int main(void) {
#define T(RANGE, ...) Test_Range_Parse(__LINE__, RANGE, NULL, (Range) __VA_ARGS__)

  // ==========================================================================
  // Single values
  // ==========================================================================

  T("0",                                          {0, 0, 0, Range_Add});
  T("1",                                          {1, 1, 0, Range_Add});
  T(F("%" PRIu64 "", UINT64_MAX),                 {UINT64_MAX, UINT64_MAX, 0, Range_Add});

  // ==========================================================================
  // Simple ranges
  // ==========================================================================

  T("0-0",                                        {0, 0, 0, Range_Add});
  T("1-1",                                        {1, 1, 0, Range_Add});
  T(F("%" PRIu64 "-%" PRIu64 "", UINT64_MAX, UINT64_MAX), {UINT64_MAX, UINT64_MAX, 0, Range_Add});

  T("0-1",                                        {0, 1, 1, Range_Add});
  T("1-0",                                        {1, 0, 1, Range_Sub});
  T(F("%" PRIu64 "-%" PRIu64 "", UINT64_MAX - 1, UINT64_MAX), {UINT64_MAX - 1, UINT64_MAX, 1, Range_Add});
  T(F("%" PRIu64 "-%" PRIu64 "", UINT64_MAX, UINT64_MAX - 1), {UINT64_MAX, UINT64_MAX - 1, 1, Range_Sub});

  T("0-2",                                        {0, 2, 1, Range_Add});
  T("2-0",                                        {2, 0, 1, Range_Sub});

  // ==========================================================================
  // Ranges with step
  // ==========================================================================

  T("0-1:1",                                      {0, 1, 1, Range_Add});
  T("0-1:+1",                                     {0, 1, 1, Range_Add});
  T("1-0:-1",                                     {1, 0, 1, Range_Sub});
  T("0-4:2",                                      {0, 4, 2, Range_Add});
  T("0-4:+2",                                     {0, 4, 2, Range_Add});
  T("4-0:-2",                                     {4, 0, 2, Range_Sub});
  T(F("0-%" PRIu64 ":%" PRIu64 "", UINT64_MAX, UINT64_MAX),  {0, UINT64_MAX, UINT64_MAX, Range_Add});
  T(F("%" PRIu64 "-0:-%" PRIu64 "", UINT64_MAX, UINT64_MAX), {UINT64_MAX, 0, UINT64_MAX, Range_Sub});

#undef T
#define T(RANGE, EXPECTED_ERROR) Test_Range_Parse(__LINE__, RANGE, EXPECTED_ERROR, (Range) {0})

  // ==========================================================================
  // Errors
  // ==========================================================================

  T("",                    "Empty range");
  T("0-",                  "Invalid range");
  T("-1",                  "Invalid range");
  T("--",                  "Invalid range");
  T("::",                  "Invalid range");
  T("1 2 3",               "Invalid range");
  T("0-1:-1",              "start < stop (step = -1)");
  T("1-0:1",               "start > stop (step = +1)");
  T("1-2:/2",              "start < stop (step = /2)");
  T("2-1:*2",              "start > stop (step = *2)");
  T("0-1:*3",              "Invalid start (step = *3): 0");
  T("0-1:/3",              "Invalid start (step = /3): 0");
  T("1-2:0",               "+0: Invalid step");
  T("1-2:+0",              "+0: Invalid step");
  T("1-2:-0",              "-0: Invalid step");
  T("1-2:/0",              "/0: Invalid step");
  T("1-2:*0",              "*0: Invalid step");
  T("1-2:/1",              "/1: Invalid step");
  T("1-2:*1",              "*1: Invalid step");
  T("0xFFFFFFFFFFFFFFFFF", "0xFFFFFFFFFFFFFFFFF: Numerical result out of range");

#undef T

  Test_Range_Iterate();

  return 0;
}
