#define _GNU_SOURCE

#ifndef DATADIR
#define DATADIR ""
#endif

#include "../acpi_call.c"
#include "../error.c"
#include "../file_utils.c"
#include "../memory.c"
#include "../nxjson.c"

#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h> // strcmp

static void test_AcpiCall_Parse_Path(
  int line,
  const char* path,
  struct AcpiCall_Path expected
) {
  Error e;
  struct AcpiCall_Path result;

  e = AcpiCall_Parse_Path(path, &result);
  if (e) {
    printf("%d: %s\n", line, err_print_all(e));
    exit(1);
  }

  if (! AcpiCall_Path_Equals(&result, &expected)) {
    printf("%d: Failed\n", line);
    exit(1);
  }
}

static void test_AcpiCall_Parse_Path_Error(
  int line,
  const char* path,
  const char* expected_error
) {
  Error e;
  struct AcpiCall_Path path_array;

  e = AcpiCall_Parse_Path(path, &path_array);
  if (! e) {
    printf("%d: Expected error\n", line);
    exit(1);
  }

  const char* having_error = err_print_all(e);
  if (strcmp(having_error, expected_error)) {
    printf("%d: %s != %s\n", line, having_error, expected_error);
    exit(1);
  }
}

static void test_AcpiCall_GetInt(
  int line,
  const char* data,
  const char* path,
  uint64_t expected
) {
  Error e;
  uint64_t result = 0;

  e = AcpiCall_GetInt(data, path, &result);
  if (e) {
    printf("%d: %s\n", line, err_print_all(e));
    exit(1);
  }

  if (result != expected) {
    printf("%d: %lu != %lu\n", line, result, expected);
    exit(1);
  }
}

static void test_AcpiCall_GetInt_Error(
  int line,
  const char* data,
  const char* path,
  const char* expected_error
) {
  Error e;
  uint64_t result;

  e = AcpiCall_GetInt(data, path, &result);
  if (! e) {
    printf("%d: Expected error\n", line);
    exit(1);
  }

  const char* having_error = err_print_all(e);
  if (strcmp(having_error, expected_error)) {
    printf("%d: %s != %s\n", line, having_error, expected_error);
    exit(1);
  }
}

int main(int argc, const char* argv[]) {
#define T(PATH, ...) test_AcpiCall_Parse_Path(__LINE__, PATH, (struct AcpiCall_Path) __VA_ARGS__)
  T("1",                               {1, 1});
  T(" 1",                              {1, 1});
  T("1 ",                              {1, 1});
  T(" 1 ",                             {1, 1});
  T("1 1",                             {2, 1, 1});
  T("1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6", {16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6});
#undef T

#define T(...) test_AcpiCall_Parse_Path_Error(__LINE__, __VA_ARGS__)
  T("",                                  "Empty path");
  T("1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7", "Path too long");
  T("777777",                            "Path: 777777: Number too large");
#undef T

#define T(...) test_AcpiCall_GetInt(__LINE__, __VA_ARGS__)
  T("[42]",                   "0",                    42);
  T("[[42]]",                 "0 0",                  42);
  T("[[[42]]]",               "0 0 0",                42);
  T("[[[[42]]]]",             "0 0 0 0",              42);
  T("[[[[[42]]]]]",           "0 0 0 0 0",            42);
  T("[[[[[[42]]]]]]",         "0 0 0 0 0 0",          42);
  T("[[[[[[[42]]]]]]]",       "0 0 0 0 0 0 0",        42);
  T("[[[[[[[[42]]]]]]]]",     "0 0 0 0 0 0 0 0",      42);
  T("[[[[[[[[[42]]]]]]]]]",   "0 0 0 0 0 0 0 0 0",    42);

  T("{42}",                   "0",                    42);
  T("{{42}}",                 "0 0",                  42);
  T("{{{42}}}",               "0 0 0",                42);
  T("{{{{42}}}}",             "0 0 0 0",              42);
  T("{{{{{42}}}}}",           "0 0 0 0 0",            42);
  T("{{{{{{42}}}}}}",         "0 0 0 0 0 0",          42);
  T("{{{{{{{42}}}}}}}",       "0 0 0 0 0 0 0",        42);
  T("{{{{{{{{42}}}}}}}}",     "0 0 0 0 0 0 0 0",      42);
  T("{{{{{{{{{42}}}}}}}}}",   "0 0 0 0 0 0 0 0 0",    42);

  T("[0xFF]",                 "0",                    255);
  T("{0xFF}",                 "0",                    255);

  T("[0, 42]",                "1",                    42);
  T("[0, 0, 42]",             "2",                    42);
  T("[0, 0, 0, 42]",          "3",                    42);
  T("[0, 0, 0, 0, 42]",       "4",                    42);
  T("[0, 0, 0, 0, 0, 42]",    "5",                    42);
  T("[0, 0, 0, 0, 0, 0, 42]", "6",                    42);

  T("[0, [42]]",              "1 0",                  42);
  T("[[0, 42]]",              "0 1",                  42);
  T("[[0,0], [0,0], 42]",     "2",                    42);
  T("[[[[0]]], 42]",          "1",                    42);
#undef T

#define T(...) test_AcpiCall_GetInt_Error(__LINE__, __VA_ARGS__)
  T("",                   "",   "Empty path");
  T("[[[[[[[[[[[[[[[[[",  "0",  "Nesting of ACPI result too deep");
  T("{{{{{{{{{{{{{{{{{",  "0",  "Nesting of ACPI result too deep");
  T(",",                  "0",  "Unexpected comma in ACPI result");
  T("]",                  "0",  "Unbalanced brackets in ACPI result");
  T("}",                  "0",  "Unbalanced brackets in ACPI result");
  T("",                   "0",  "No integer at path \"0\"");
  T("0",                  "0",  "No integer at path \"0\"");
  T("[]",                 "0",  "No integer at path \"0\"");
  T("[[]]",               "0",  "No integer at path \"0\"");
#undef T

  return 0;
}
