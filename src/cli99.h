#ifndef CLI99_H_
#define CLI99_H_

#include <stddef.h> // size_t
#include <stdint.h> // int64_t, uintptr_t
#include <stdbool.h>

#ifndef CLI99_OPTION_DELIMITERS
#define CLI99_OPTION_DELIMITERS "|"
#endif

#define CLI99_INCLUDE_MARKER_UINT 1ULL
#define CLI99_INCLUDE_MARKER      ((const char*) CLI99_INCLUDE_MARKER_UINT)

#define cli99_Options_End()       {NULL, 0, 0}
#define cli99_Options_Include(O)  {CLI99_INCLUDE_MARKER, (int64_t) (uintptr_t) O, 0}

enum cli99_ArgumentMode {
  cli99_NoArgument,
  cli99_RequiredArgument,
  cli99_OptionalArgument,
};

enum cli99_PositionalMode {
  cli99_NormalPositional,
  cli99_RepeatablePositional,
};

enum cli99_Error {
  cli99_ErrorSuccess = 0,
  cli99_ErrorUnrecognizedPositional,
  cli99_ErrorUnrecognizedArgument,
  cli99_ErrorUnrecognizedOption,
  cli99_ErrorMissingArgument,
};

struct cli99_Option {
  const char* optstring;
  int64_t value;
  char mode; /* cli99_ArgumentMode / cli99_PositionalMode */
};

struct cli99 {
  char* const* argv;
  int          argc;
  int          argi;
  int          argii;
  int          positional_count;
  bool         end_of_options;
  bool         in_short_opt;
  const struct cli99_Option* options;
  const struct cli99_Option* option;
  const char*  optarg;
  enum cli99_Error error;
  const char*  error_cause;
  char         buf[3];
};

void cli99_Init(struct cli99*, const struct cli99_Option*, char* const*, int);
int64_t cli99_GetOpt(struct cli99*);
const char* cli99_StrError(enum cli99_Error);

#endif
