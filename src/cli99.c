#include "cli99.h"

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define CLI99_UNREACHABLE_RETURN(VALUE)  __builtin_unreachable()
#else
#define CLI99_UNREACHABLE_RETURN(VALUE)  return VALUE
#endif

void cli99_Init(struct cli99* self, const struct cli99_Option* options, char* const* argv, int argc) {
  self->argv = argv;
  self->argc = argc;
  self->argi = 1;
  self->argii = 0;
  self->positional_count = 0;
  self->end_of_options = false;
  self->in_short_opt = false;
  self->options = options;
  self->option = NULL;
  self->optarg = NULL;
  self->error = cli99_ErrorSuccess;
  self->error_cause = "";
}

enum cli99_ArgumentType {
  cli99_Positional,
  cli99_ShortOption,
  cli99_LongOption,
  cli99_EndOfOptions,
};

/*
 * Classifies a command-line argument.
 *
 * Example:
 *
 *  "-"     -> cli99_Positional
 *  "--"    -> cli99_EndOfOptions
 *  "-a"    -> cli99_ShortOption
 *  "--arg" -> cli99_LongOption
 *  "foo"   -> cli99_Positional
 */
static enum cli99_ArgumentType cli99_GetArgumentType(const char* s) {
  if (s[0] == '-') {
    if (s[1] == '-')
      return s[2] ? cli99_LongOption : cli99_EndOfOptions;
    return s[1] ? cli99_ShortOption : cli99_Positional;
  }

  return cli99_Positional;
}

/*
 * Return true if `c` is a character in CLI99_OPTION_DELIMITERS.
 */
static bool cli99_IsOptionDelimiter(char c) {
  for (size_t i = 0; i < sizeof(CLI99_OPTION_DELIMITERS) - 1; ++i)
    if (c == CLI99_OPTION_DELIMITERS[i])
      return true;

  return false;
}

/*
 * Return the length of a long option.
 *
 * Example:
 *
 *  "--arg"  -> 5
 *  "--arg=" -> 5
 */
static size_t cli99_LongOptLen(const char* s) {
  size_t len = 0;
  while (s[len] && s[len] != '=')
    ++len;
  return len;
}

/*
 * Return true if `optstring` contains the short option `opt`.
 *
 * Example:
 *
 *  cli99_OptstringContainsShortOption("-a|--arg", 'a') -> true
 */
static bool cli99_OptstringContainsShortOption(const char* optstring, char opt) {
  while (*optstring) {
    if (optstring[0] == '-' && optstring[1] == opt)
      return true;

    while (*optstring && !cli99_IsOptionDelimiter(*optstring))
      optstring++;

    while (*optstring && cli99_IsOptionDelimiter(*optstring))
      optstring++;
  }

  return false;
}

/*
 * Return true if `optstring` contains the long option `opt` with a length
 * of `optlen`.
 *
 * Example:
 *
 *  cli99_OptstringContainsLongOption("-a|--arg", "arg", 3) -> true
 */
static bool cli99_OptstringContainsLongOption(const char* optstring, const char* opt, size_t optlen) {
  while (*optstring) {
    if (optstring[0] == '-' && optstring[1] == '-') {
      optstring += 2;

      if (optstring[optlen] && !cli99_IsOptionDelimiter(optstring[optlen]))
        goto next;

      for (size_t i = 0; i < optlen; ++i) {
        if (optstring[i] != opt[i])
          goto next;
      }

      return true;
    }

next:
    while (*optstring && !cli99_IsOptionDelimiter(*optstring))
      optstring++;

    while (*optstring && cli99_IsOptionDelimiter(*optstring))
      optstring++;
  }

  return false;
}

/*
 * Find the cli99_Option which holds the short option `shortopt`.
 *
 * This function operates recusively.
 *
 * Return NULL if no option could be found.
 */
static const struct cli99_Option* cli99_FindShortOption(const struct cli99_Option* option, char shortopt) {
  const struct cli99_Option* found;

next_option:
  if (option->optstring == NULL)
    return NULL;

  if ((uintptr_t) option->optstring == CLI99_INCLUDE_MARKER_UINT) {
    found = cli99_FindShortOption((const struct cli99_Option*) (uintptr_t) option->value, shortopt);
    if (found)
      return found;
  }
  else if (cli99_OptstringContainsShortOption(option->optstring, shortopt))
    return option;

  option++;
  goto next_option;
}

/*
 * Find the cli99_Option which holds the long option `longopt` with a length
 * of `optlen`.
 *
 * This function operates recusively.
 *
 * Return NULL if no option could be found.
 */
static const struct cli99_Option* cli99_FindLongOption(const struct cli99_Option* option, const char* longopt, size_t optlen) {
  const struct cli99_Option* found;

next_option:
  if (option->optstring == NULL)
    return NULL;

  if ((uintptr_t) option->optstring == CLI99_INCLUDE_MARKER_UINT) {
    found = cli99_FindLongOption((const struct cli99_Option*) (uintptr_t) option->value, longopt, optlen);
    if (found)
      return found;
  }
  else if (cli99_OptstringContainsLongOption(option->optstring, longopt, optlen))
    return option;

  option++;
  goto next_option;
}

static const struct cli99_Option* cli99_FindPositional_Impl(const struct cli99_Option* option, int* current, int positional) {
  const struct cli99_Option* found;

next_option:
  if (option->optstring == NULL)
    return NULL;

  if ((uintptr_t) option->optstring == CLI99_INCLUDE_MARKER_UINT) {
    found = cli99_FindPositional_Impl((const struct cli99_Option*) (uintptr_t) option->value, current, positional);
    if (found)
      return found;
  }
  else if (option->optstring[0] != '-') {
    if (option->mode == cli99_NormalPositional) {
      if (positional == *current)
        return option;
    }
    else if (option->mode == cli99_RepeatablePositional) {
      if (positional >= *current)
        return option;
    }

    (*current)++;
  }

  option++;
  goto next_option;
}

/*
 * Find the cli99_Option that holds the positional with the index stored in
 * `positional`.
 *
 * Index start from zero.
 *
 * Returns NULL if no option could be found.
 */
static inline const struct cli99_Option* cli99_FindPositional(const struct cli99_Option* option, int positional) {
  int current = 0;
  return cli99_FindPositional_Impl(option, &current, positional);
}

/*
 * Parse a positional.
 */
static int64_t cli99_ParsePositional(struct cli99* self) {
  self->option = cli99_FindPositional(self->options, self->positional_count);

  if (! self->option) {
    self->error = cli99_ErrorUnrecognizedPositional;
    self->error_cause = self->argv[self->argi];
    return -1;
  }

  self->optarg = self->argv[self->argi];
  self->positional_count++;
  self->argi++;
  return self->option->value;
}

/*
 * Parse a long option
 */
static int64_t cli99_ParseLongOpt(struct cli99* self){
  const char* const arg = self->argv[self->argi] + 2;
  size_t len = cli99_LongOptLen(arg);
  self->option = cli99_FindLongOption(self->options, arg, len);

  if (! self->option) {
    self->error = cli99_ErrorUnrecognizedOption;
    self->error_cause = self->argv[self->argi];
    return -1;
  }

  switch (self->option->mode) {
  case cli99_NoArgument:
    if (arg[len]) /* arg[len] is '=' */ {
      self->error = cli99_ErrorUnrecognizedArgument;
      self->error_cause = self->option->optstring;
      return -1;
    }

    self->argi++;
    return self->option->value;

  case cli99_RequiredArgument:
    if (arg[len]) /* arg[len] is '=' */ {
      self->optarg = &arg[len + 1];
      self->argi++;
      return self->option->value;
    }

    if (self->argi + 1 >= self->argc) {
      self->error = cli99_ErrorMissingArgument;
      self->error_cause = self->option->optstring;
      return -1;
    }

    self->optarg = self->argv[++(self->argi)];
    self->argi++;
    return self->option->value;

  case cli99_OptionalArgument:
    if (arg[len]) /* arg[len] is '=' */ {
      self->optarg = &arg[len + 1];
    }
    else {
      self->optarg = NULL;
    }

    self->argi++;
    return self->option->value;
  }

  CLI99_UNREACHABLE_RETURN(-1);
}

/*
 * Parse a short option
 */
static int64_t cli99_ParseShortOpt(struct cli99* self) {
  const char shortopt = self->argv[self->argi][self->argii];

  self->option = cli99_FindShortOption(self->options, shortopt);

  if (! self->option) {
    self->error = cli99_ErrorUnrecognizedOption;
    self->buf[0] = '-';
    self->buf[1] = shortopt;
    self->buf[2] = '\0';
    self->error_cause = self->buf;
    return -1;
  }

  switch (self->option->mode) {
    case cli99_NoArgument:
      if (self->argv[self->argi][self->argii + 1]) {
        self->argii++;
      }
      else {
        self->argi++;
        self->in_short_opt = false;
      }

      return self->option->value;

    case cli99_RequiredArgument:
      if (self->argv[self->argi][self->argii + 1]) {
        self->optarg = &self->argv[self->argi][self->argii + 1];
        self->argi++;
        self->in_short_opt = false;
        return self->option->value;
      }

      if (self->argi + 1 < self->argc) {
        self->optarg = self->argv[++(self->argi)];
        self->argi++;
        self->in_short_opt = false;
        return self->option->value;
      }

      self->in_short_opt = false;
      self->error = cli99_ErrorMissingArgument;
      self->error_cause = self->option->optstring;
      return -1;

    case cli99_OptionalArgument:
      self->optarg = &self->argv[self->argi][self->argii + 1];
      if (! self->optarg[0])
        self->optarg = NULL;
      self->argi++;
      self->in_short_opt = false;
      return self->option->value;
  }

  CLI99_UNREACHABLE_RETURN(-1);
}

/*
 * Parse the next available option or positional.
 *
 * Returns `0` if there are no more arguments to be parsed.
 *
 * Return `-1` on error.
 */
int64_t cli99_GetOpt(struct cli99* self) {
  if (self->argi >= self->argc)
    return 0;

  if (self->end_of_options)
    return cli99_ParsePositional(self);

  if (self->in_short_opt)
    return cli99_ParseShortOpt(self);

  switch (cli99_GetArgumentType(self->argv[self->argi])) {
    case cli99_EndOfOptions:
      self->end_of_options = true;
      self->argi++;
      return cli99_GetOpt(self);

    case cli99_Positional:
      return cli99_ParsePositional(self);

    case cli99_ShortOption:
      self->in_short_opt = true;
      self->argii = 1;
      return cli99_ParseShortOpt(self);

    case cli99_LongOption:
      return cli99_ParseLongOpt(self);
  }

  CLI99_UNREACHABLE_RETURN(-1);
}

static const char* const cli99_ErrorMessages[] = {
  // cli99_ErrorSuccess
  "Success",

  // cli99_ErrorUnrecognizedPositional
  "Unrecognized positional argument",

  // cli99_ErrorUnrecognizedArgument
  "Unrecognized argument",

  // cli99_ErrorUnrecognizedOption
  "Unrecognized option",

  // cli99_ErrorMissingArgument
  "Missing argument"
};

const char* cli99_StrError(enum cli99_Error e) {
  return cli99_ErrorMessages[e];
}
