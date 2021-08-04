#include "optparse.h"

#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <stdio.h>  /* fprintf */
#include <stdlib.h> /* strtold, strtoll, strtoull, llabs */
#include <string.h> /* strncmp */

#ifndef cli99_EnabledFlags
#define cli99_EnabledFlags            0xFFFFFFFFFFFFFFFFuLL
#endif

#define cli99_FilterFlags(F)         ((F) & cli99_EnabledFlags)

#define cli99_int_fits(VAL, BYTES)   cli99_uint_fits(llabs(VAL), (BYTES))
#define cli99_uint_fits(VAL, BYTES)  (! (((uint64_t) (VAL)) >> ((BYTES)*8)))
#define cli99_float_fits(...)        (1)

#define cli99_argument_is_required(FLAGS) ( \
  (((FLAGS) & cli99_nargs_mask) != '?') && \
  (((FLAGS) & cli99_nargs_mask) != '*') && \
  (((FLAGS) & cli99_nargs_mask) != 0))

#ifndef cli99_optstring_delimiters
#define cli99_optstring_delimiters   " |,"
#endif

#ifndef my
#define my (*self)
#define cli99_defined_my_self // :D
#endif

static const char* const cli99_ErrorMessages[cli99_Err_Count] = {
  "Success",
  "Mutual exclusion conflict",
  "Type conversion of argument failed",
  "Unknown option",
  "Argument required",
  "Too many arguments",
  "Unknown error",
};

static void cli99_SetOptions_Impl(cli99*, const cli99_option*, int*, uint8_t*);

static inline bool cli99_IsNegativeNum(const char *s) {
  if (*s++ == '-') {
    for (int digits = 0;; ++s)
      if (*s >= '0' && *s <= '9') ++digits;
      else if (*s == '.' || *s == ',');
      else return (!*s) + digits;
  }
 return false;
}

static inline bool cli99_IsDelimiter(int c) {
  for (size_t i = 0; i < sizeof(cli99_optstring_delimiters) - 1; ++i)
    if (cli99_optstring_delimiters[i] == c)
      return true;
  return false;
}

void cli99_Init(cli99* self, int argc, char* const argv[], const cli99_option* options, uint64_t flags)
{
  assert(argv    && "cli99_Init(): argv == NULL");
  assert(options && "cli99_Init(): options == NULL");
  my.argc    = argc;
  my.argv    = (const char**) argv;
  my.flags   = flags;
  my._state  = cli99_state_uninitialized;
  my._options_size = 0;
  cli99_SetOptions(self, options, true);
}

void cli99_SetOptions(cli99* self, const cli99_option* options, bool reset) {
  int old_options_size = my._options_size;
  my._options_size = 0;
  memset(&my.by, 0, sizeof(my.by));

  if (reset)
    my._groups_count = my._groups = 0;

  int positional_count = 0;
  cli99_SetOptions_Impl(self, options, &positional_count, NULL);
  for (int i = old_options_size; i < my._options_size; ++i)
    my._options[i].count = 0;
}

static void cli99_SetOptions_Impl(cli99* self, const cli99_option* options, int* positional_count, uint8_t* current_group) {
  for (const char* optstring; (optstring = options->optstring); options++)
    switch ((uintptr_t) (optstring)) {
      case cli99_include_marker_uint:
        cli99_SetOptions_Impl(self, (const cli99_option*) (uintptr_t) options->value, positional_count, current_group);
        break;

      case cli99_group_marker_uint:
        current_group = &my._groups_count;
        my._groups_count++;
        break;

      case cli99_group_end_marker_uint:
        current_group = NULL;
        break;

      default:
      {
        cli99_option_internal* opt = &my._options[my._options_size++];
        opt->option = options;
        opt->group = current_group ? *current_group : 0;
        opt->long_opt_start = 0;
        opt->long_opt_len   = 0;

        while (*optstring) {
          const int ndashes = ((optstring[0]=='-') + (optstring[1]=='-'));
          optstring += ndashes;
          const char* o = optstring;

          for (; *optstring && !cli99_IsDelimiter(*optstring); ++optstring);
          for (; *optstring &&  cli99_IsDelimiter(*optstring); ++optstring);

          switch (ndashes) {
          case 0: my.by.positional[(*positional_count)++] = opt - my._options + 1;
                  break;
          case 1: my.by.short_opt[(uint8_t) *o] = opt - my._options + 1;
                  break;
          case 2: opt->long_opt_start = o - opt->option->optstring;
                  opt->long_opt_len   = optstring - o;
                  break;
          }
        }
      }
    }
}

/**
 * Convert an argument
 */
bool cli99_ConvertOptarg(cli99* self, uint64_t flags) {
  const uint64_t type = cli99_FilterFlags(flags) & cli99_base_type_mask;
  const uint64_t size = (cli99_FilterFlags(flags) & cli99_type_size_mask) >> 8;

  if (!type) return true;
  if (!my.optarg || !my.optarg[0]) return false;

  errno = 0;
  char* end;

  switch (type) {
  case cli99_type_int_base:
    my.optval.i = strtoll(my.optarg, &end, 0);
    return !errno && !*end && cli99_int_fits(my.optval.i, size);

  case cli99_type_uint_base:
    my.optval.u = strtoull(my.optarg, &end, 0);
    return !errno && !*end && cli99_uint_fits(my.optval.u, size);

  case cli99_type_float_base:
    my.optval.ld = strtold(my.optarg, &end);
    return !errno && !*end && cli99_float_fits(my.optval.ld, size);

  case cli99_type_char:
    return (my.optval.c = my.optarg[0]), !my.optarg[1];

  case cli99_type_bool:
    if (((unsigned char) my.optarg[0] | 0x20U) == 'f' &&
        ((unsigned char) my.optarg[1] | 0x20U) == 'a' &&
        ((unsigned char) my.optarg[2] | 0x20U) == 'l' &&
        ((unsigned char) my.optarg[3] | 0x20U) == 's' &&
        ((unsigned char) my.optarg[4] | 0x20U) == 'e')
      return (my.optval.b = 0), !my.optarg[5];
    else if (
        ((unsigned char) my.optarg[0] | 0x20U) == 't' &&
        ((unsigned char) my.optarg[1] | 0x20U) == 'r' &&
        ((unsigned char) my.optarg[2] | 0x20U) == 'u' &&
        ((unsigned char) my.optarg[3] | 0x20U) == 'e')
      return (my.optval.b = 1), !my.optarg[4];
    else {
      my.optval.u = my.optarg[0] - '0';
      return my.optval.u <= 1 && !my.optarg[1];
    }

  default:
    return false; /* Unknown type */
  }
}

static inline bool cli99_OptionExclusiveCheck(const cli99* self, const cli99_option_internal* o) {
  return (! o->group) || (! (my._groups & (1ULL << (o->group - 1)))) || o->count;
}

bool cli99_CheckRequired(cli99* self) {
  for (int i = 0; i < my._options_size; ++i) {
    const cli99_option_internal* o = &my._options[i];
    if ((o->option->flags & cli99_required_option) && !o->count) {
      my.optopt = o->option->optstring;
      my.optarg = NULL;
      my.error = cli99_ErrArgumentRequired;
      return false;
    }
  }

  return true;
}

static inline uint64_t cli99_GetNextState(const char* s) {
  if (s[0] == '-') {
    if (s[1] == '-')
      return s[2] ? cli99_state_long_opt : cli99_state_options_end;
    return s[1] ? cli99_state_short_opt : cli99_state_positional;
  }
  return cli99_state_positional;
}

/**
 * The working horse of the parser.
 *
 * This function is used to control the internal parsing state.
 *
 * Operations:
 *   cli99_op_is_eof, 0
 *     Return true if parser is in cli99_state_EOF state.
 *
 *   cli99_op_next, 0
 *     Advance parsing position and change state
 *
 *   cli99_op_getarg,  <type> | cli99_argument_mask
 *     Read the current argument into `self->optarg` and do type
 *     conversion as wished.
 *
 *     Will fail if the current argument is not a positional.
 *
 *   cli99_op_getoptarg, <type> | cli99_argument_mask
 *     Read the current argument into `self->optarg` and do type
 *     conversion as wished.
 *
 *     Will fail if the current argument is not considered
 *     to be an argument as specified in `self->flags`.
 *
 *   cli99_op_rewind_short_opt, 0
 *     Restart parsing a short option from its argument.
 *     E.g. if this is called right after parsing `-ofoo`,
 *     the parser will continue parsing `f` as a short option instead of
 *     considering `foo` to be an argument of `-o`.
 */
bool cli99_StateCtl(cli99* self, cli99_operation operation, uint64_t flags_) {
  const uint64_t flags  = cli99_FilterFlags(flags_);

apply_state:
switch (my._state) {
case cli99_state_uninitialized: /* transient state */
  my.argi   = 0;
  my.optopt = NULL;
  my.optarg = NULL;
  my.error  = 0;
  my._arg_s = NULL;
  my._arg_l = -1;
  my._positional_count = 0;
  my._end_of_options = false;
  //my._state = cli99_state_next_word;
  /* FALLTHROUGH */

case cli99_state_next_word: /* transient state */
  if (my.argi + 1 >= my.argc) {
    my._state = cli99_state_EOF;
    goto apply_state;
  }

  my.argi++;
  my._arg_s = my.argv[my.argi];
  my._arg_l = -1;

  if (my._end_of_options) {
    my._state = cli99_state_positional;
    goto apply_state;
  }

  switch ((int) (my._state = cli99_GetNextState(my.argv[my.argi]))) {
    case cli99_state_short_opt:
      my._arg_s = &my.argv[my.argi][1];
      my._arg_l = 1;
      goto apply_state;

    case cli99_state_long_opt:
      my._arg_s = &my.argv[my.argi][2];
      for (
        my._arg_l = 0;
        my._arg_s[my._arg_l] && my._arg_s[my._arg_l] != '=';
        my._arg_l++
      );
      goto apply_state;

    case cli99_state_options_end:
      my._end_of_options = 1;
      goto apply_state;

    case cli99_state_positional:
      goto apply_state;
  }
  break;

case cli99_state_short_opt:
  switch (operation) {
  case cli99_op_getoptarg:
    if (flags & cli99_no_separate_optionals)
      return !cli99_argument_is_required(flags);

    if (flags & cli99_no_options_as_arguments)
      if (! cli99_IsNegativeNum(my.argv[my.argi]))
        return !cli99_argument_is_required(flags);

    my.optarg = my.argv[my.argi];
    my._state = cli99_state_next_word;
    return true;

  case cli99_op_next:
    if (my._arg_s[1]) {
      my._arg_s++;
      my._state = cli99_state_short_parameter;
    }
    else
      my._state = cli99_state_next_word;

    return true;

  default: return false;
  }

case cli99_state_long_opt:
  switch (operation) {
  case cli99_op_getoptarg:
    if (flags & (cli99_no_options_as_arguments|cli99_no_separate_optionals))
      return !cli99_argument_is_required(flags);

    my.optarg = my.argv[my.argi];
    my._state = cli99_state_next_word;
    return true;

  case cli99_op_next:
    if (my._arg_s[my._arg_l]) { // --arg=
      my._arg_s += my._arg_l + 1;
      my._arg_l = -1;
      my._state = cli99_state_long_parameter;
    }
    else
      my._state = cli99_state_next_word;
    return true;

  default: return false;
  }

case cli99_state_short_parameter:
  switch (operation) {
  case cli99_op_getoptarg:
    my.optarg = my._arg_s; // FALLTHROUGH

  case cli99_op_next:
    my._state = cli99_state_next_word;
    return true;

  case cli99_op_rewind_short_opt:
    my._state = cli99_state_short_opt;
    return true;

  default: return false;
  }

case cli99_state_long_parameter:
  switch (operation) {
  case cli99_op_getoptarg:
    my.optarg = my._arg_s; // FALLTHROUGH

  case cli99_op_next:
    my._state = cli99_state_next_word;
    return true;

  default: return false;
  }

case cli99_state_options_end:
  switch (operation) {
  case cli99_op_next:
    my._state = cli99_state_next_word;
    return true;

  default: return false;
  }

case cli99_state_positional:
  switch (operation) {
  case cli99_op_getoptarg:
    if ((flags & cli99_no_separate_optionals) && cli99_argument_is_required(flags))
      return false;
    // FALLTHROUGH
  case cli99_op_getarg:
    my.optarg = my._arg_s;
    // FALLTHROUGH
  case cli99_op_next:
    my._state = cli99_state_next_word;
    return true;
  default:
    return false;
  }

case cli99_state_EOF:
  switch (operation) {
  case cli99_op_getarg: // FALLTHROUGH
  case cli99_op_getoptarg:
    return !cli99_argument_is_required(flags);
  case cli99_op_is_eof:
    return true;
  default:
    return false;
  }
}
return false;
}

static cli99_option_internal* cli99_MatchOptions(cli99* self) {
  cli99_StateCtl(self, 0, 0);
  const char* optstring;
  cli99_option_internal* option;

  switch ((int) my._state) {
  case cli99_state_positional:
    if (my._positional_count < 32 &&
        my.by.positional[my._positional_count])
      return &my._options[my.by.positional[my._positional_count] - 1];
    break;

  case cli99_state_short_opt:
    if (my.by.short_opt[(uint8_t) *my._arg_s])
      return &my._options[my.by.short_opt[(uint8_t) *my._arg_s] - 1];
    break;

  case cli99_state_long_opt:
    // Try first long option (without parsing `optstring`)
    for (int i = 0; i < my._options_size; ++i) {
      option = &my._options[i];
      optstring = option->option->optstring;
      if ((my._arg_l == option->long_opt_len) &&
          (*my._arg_s == optstring[option->long_opt_start]) &&
           !strncmp(&optstring[option->long_opt_start], my._arg_s, my._arg_l))
        return option;
    }

    for (int i = 0; i < my._options_size; ++i) {
      option = &my._options[i];
      if (! option->long_opt_start)
        continue;

      // Skip first option
      optstring = option->option->optstring + option->long_opt_start + option->long_opt_len;

      while (*optstring) {
        const int ndashes = ((optstring[0]=='-') + (optstring[1]=='-'));
        optstring += ndashes;
        const char* o = optstring;
        for (; *optstring && !cli99_IsDelimiter(*optstring); ++optstring);
        for (; *optstring &&  cli99_IsDelimiter(*optstring); ++optstring);
        if (ndashes == 2) {
          int option_len = o - optstring;
          if (my._arg_l == option_len && !strncmp(o, my._arg_s, my._arg_l))
            return option;
        }
      }
    }
    break;
  }

  return NULL;
}

/**
 * Try to parse an option.
 *
 * On success:
 *   - `self->optopt` will hold the option name
 *   - `self->optarg` will hold the option argument (if any)
 *   - `self->optval` will hold the result of the converted `optarg`.
 *   - `cli99_option->value` is returned.
 *
 * On failure:
 *   -  `0` will be returned if no option is available
 *   - `-1` will be returned if an error occured
 */
int cli99_GetOpt(cli99* self) {
  my.optarg = NULL;
  cli99_option_internal* o = cli99_MatchOptions(self);

  if (! o)
    switch (my._state) {
    case cli99_state_options_end:
      cli99_StateCtl(self, cli99_op_next, 0);
      return cli99_GetOpt(self);

    case cli99_state_EOF:
      return 0;

    default:
      my.optopt = my.argv[my.argi];
      my.error = cli99_ErrUnknownOption;
      return -1;
    }

  my.optopt = o->option->optstring;

  if (! cli99_OptionExclusiveCheck(self, o)) {
    my.error = cli99_ErrMutuallyExclusive;
    return -1;
  }

  o->count++;
  if (o->group)
    my._groups |= (1ULL << (o->group - 1));

  if (my._state == cli99_state_positional) {
    if (! cli99_GetOptarg(self, o->option->flags))
      return -1;
    ++my._positional_count;
    return o->option->value;
  }

  cli99_StateCtl(self, cli99_op_next, 0);

  if ((o->option->flags & cli99_nargs_mask) == 0) {
    if (my._state == cli99_state_long_parameter) {
      my.error = cli99_ErrTooManyArguments;
      return -1;
    }
    else if (my._state == cli99_state_short_parameter)
      cli99_StateCtl(self, cli99_op_rewind_short_opt, 0);
  }
  else if (! cli99_GetOptarg(self, o->option->flags))
    return -1;

  return o->option->value;
}

bool cli99_GetArg(cli99* self, uint64_t flags) {
  if (! cli99_StateCtl(self, cli99_op_getarg, my.flags | flags))
    return !(my.error = cli99_ErrArgumentRequired);

  if (my.optarg)
    if (! cli99_ConvertOptarg(self, flags))
      return !(my.error = cli99_ErrTypeConversionFailed);
  return true;
}

/**
 * Try to parse an option argument (-o arg, --option=arg, ...).
 *
 * If the argument has been parsed successfully, it is stored in self->optarg.
 * The result of the type conversion is available in the self->optval.
 *
 * The following errors may be set on failure:
 *    - cli99_ErrArgumentRequired
 *    - cli99_ErrTypeConversionFailed
 *
 * @param cli99_optional_argument   Don't treat a missing argument as an error.
 * @param cli99_type(...)           Convert argument to TYPE.
 *
 * @return @true on success, @false on failure
 */
bool cli99_GetOptarg(cli99* self, uint64_t flags) {
  if (! cli99_StateCtl(self, cli99_op_getoptarg, my.flags | flags))
    return !(my.error = cli99_ErrArgumentRequired);

  if (my.optarg)
    if (! cli99_ConvertOptarg(self, flags))
      return !(my.error = cli99_ErrTypeConversionFailed);
  return true;
}

void cli99_ExplainError(cli99* self) {
  fprintf(stderr, "%s: %s%s%s%s%s\n",
    my.argv[0],
    (my.optopt ? my.optopt : ""),
    (my.optopt ? ": " : ""),
    cli99_LastError(self),
    (my.optarg ? ": " : ""),
    (my.optarg ? my.optarg : "")
  );
}

void cli99_PrintFlagMask(cli99* self) {
  uint64_t flags = self->flags;
  for (int i = 0; i < my._options_size; ++i)
    flags |= my._options[i].option->flags;
  printf("#define cli99_EnabledFlags 0x%.16lXuLL\n", flags);
  if (flags != cli99_EnabledFlags)
    printf("// Warning, parser configuration has changed\n");
}

const char* cli99_StrError(int e) {
  if (e < 0 || e >= cli99_Err_Count)
    e = cli99_ErrUnknown;
  return cli99_ErrorMessages[e];
}

#ifdef cli99_defined_my_self
#undef my
#undef cli99_defined_my_self
#endif
