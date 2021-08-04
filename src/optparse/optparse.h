#ifndef CLI99_H_
#define CLI99_H_

#include <stdint.h>
#include <stdbool.h>

#define cli99_OPTIONS_MAX 256

// Flag masks =================================================================
#define cli99_nargs_mask              0x000000000000003FuLL // num of args: 0|1|'?'|'*'|'+'
#define cli99_required_option         0x0000000000000040uLL // option is required
#define cli99_type_size_mask          0x0000000000000F00uLL // sizeof(type)
#define cli99_base_type_mask          0x00000000000FF000uLL // bool, char, int, unsigned, float
#define cli99_options_mask            0x000000000F000000uLL // behavioural options

// Basic data types ===========================================================
#define cli99_type_int_base           0x0000000000001000uLL // signed integer type
#define cli99_type_uint_base          0x0000000000002000uLL // unsigned integer type
#define cli99_type_float_base         0x0000000000004000uLL // floating value type
#define cli99_type_bool               0x0000000000008000uLL // bool type
#define cli99_type_char               0x0000000000010000uLL // char type
#define cli99_type_error_             0x00000000000F0000uLL
#define cli99_sized_type(T, SIZE)     ((T) | (SIZE) << 8)

// Aliased types (type|sizeof) ================================================
// [If you don't like the cli99_type() macro]
#define cli99_type_int8_t             cli99_sized_type(cli99_type_int_base,   1u)
#define cli99_type_int16_t            cli99_sized_type(cli99_type_int_base,   2u)
#define cli99_type_int32_t            cli99_sized_type(cli99_type_int_base,   4u)
#define cli99_type_int64_t            cli99_sized_type(cli99_type_int_base,   8u)
#define cli99_type_uint8_t            cli99_sized_type(cli99_type_uint_base,  1u)
#define cli99_type_uint16_t           cli99_sized_type(cli99_type_uint_base,  2u)
#define cli99_type_uint32_t           cli99_sized_type(cli99_type_uint_base,  4u)
#define cli99_type_uint64_t           cli99_sized_type(cli99_type_uint_base,  8u)
#define cli99_type_float              cli99_sized_type(cli99_type_float_base, sizeof(float))
#define cli99_type_double             cli99_sized_type(cli99_type_float_base, sizeof(double))
#define cli99_type_long_double        cli99_sized_type(cli99_type_float_base, sizeof(long double))

#define cli99_type_unsigned_char      cli99_sized_type(cli99_type_char)

#define cli99_type_short              cli99_sized_type(cli99_type_int_base,   sizeof(short))
#define cli99_type_int                cli99_sized_type(cli99_type_int_base,   sizeof(int))
#define cli99_type_long               cli99_sized_type(cli99_type_int_base,   sizeof(long))
#define cli99_type_long_long          cli99_sized_type(cli99_type_int_base,   sizeof(long long))
#define cli99_type_ssize_t            cli99_sized_type(cli99_type_int_base,   sizeof(ssize_t)
#define cli99_type_intmax_t           cli99_sized_type(cli99_type_int_base,   sizeof(intmax_t)

#define cli99_type_unsigned_short     cli99_sized_type(cli99_type_uint_base,  sizeof(unsigned short))
#define cli99_type_unsigned           cli99_sized_type(cli99_type_uint_base,  sizeof(unsigned int))
#define cli99_type_unsigned_long      cli99_sized_type(cli99_type_uint_base,  sizeof(unsigned long))
#define cli99_type_unsigned_long_long cli99_sized_type(cli99_type_uint_base,  sizeof(unsigned long long))
#define cli99_type_size_t             cli99_sized_type(cli99_type_uint_base,  sizeof(size_t)
#define cli99_type_uintmax_t          cli99_sized_type(cli99_type_uint_base,  sizeof(uintmax_t)

// Behaviour ==================================================================
#define cli99_no_separate_optionals   0x0000000001000000uLL
#define cli99_no_options_as_arguments 0x0000000002000000uLL
#define cli99_no_short_long_opts      0x0000000004000000uLL
#define cli99_no_abbreviated_opts     0x0000000008000000uLL

// Behaviour of well known parsers ============================================
#define cli99_options_python          (cli99_no_options_as_arguments)
#define cli99_options_getopt          (cli99_no_separate_optionals)

// Macro for converting a type into flags =====================================
#define cli99_type(T) (\
  cli99_type_is_bool(T)     ? (cli99_type_bool) : \
  cli99_type_is_char(T)     ? (cli99_type_char) : \
  cli99_type_is_floating(T) ? cli99_sized_type(cli99_type_float_base, sizeof(T)) : \
  cli99_type_is_unsigned(T) ? cli99_sized_type(cli99_type_uint_base,  sizeof(T)) : \
  cli99_type_is_int(T)      ? cli99_sized_type(cli99_type_int_base,   sizeof(T))   \
                            :  cli99_type_error_)

#define cli99_type_is_unsigned(T)     (((T)  -1) >  0)
#define cli99_type_is_bool(T)         (((T)   2) == 1)
#define cli99_type_is_floating(T)     (((T) 2.5) >  2.0)
#define cli99_type_is_int(T)          (((T) 2.5) == 2.0)
#define cli99_type_is_char(T)         (cli99_type_is_int(T) && sizeof(T) == 1)

#define cli99_include_marker_uint     (1ULL)
#define cli99_group_marker_uint       (2ULL)
#define cli99_group_end_marker_uint   (3ULL)
#define cli99_max_marker_uint         (9ULL)

#define cli99_include_marker          ((const char*) cli99_include_marker_uint)
#define cli99_group_marker            ((const char*) cli99_group_marker_uint)
#define cli99_group_end_marker        ((const char*) cli99_group_end_marker_uint)
#define cli99_max_marker              ((const char*) cli99_max_marker_uint)

#define cli99_options_end()           {NULL, 0, 0}
#define cli99_include_options(O)      {cli99_include_marker, (uintptr_t) O, 0}
#define cli99_exclusive_group(G, F)   {cli99_group_marker, (uintptr_t) G, F}
#define cli99_end_exclusive_group()   {cli99_group_end_marker, 0, 0}

enum cli99_Error {
  cli99_ErrSuccess,
  cli99_ErrMutuallyExclusive,
  cli99_ErrTypeConversionFailed,
  cli99_ErrUnknownOption,
  cli99_ErrArgumentRequired,
  cli99_ErrTooManyArguments,
  cli99_ErrUnknown,
  cli99_Err_Count,
};

enum cli99_operation { /* internal */
  cli99_op_is_eof,
  cli99_op_getoptarg,
  cli99_op_getarg,
  cli99_op_next,
  cli99_op_rewind_short_opt,
};

enum cli99_state {
  cli99_state_uninitialized,
  cli99_state_positional,
  cli99_state_short_opt,
  cli99_state_long_opt,
  cli99_state_short_parameter,
  cli99_state_long_parameter,
  cli99_state_options_end,
  cli99_state_next_word,
  cli99_state_EOF,
};

typedef enum   cli99_Error     cli99_Error;
typedef enum   cli99_state     cli99_state;
typedef enum   cli99_operation cli99_operation;
typedef struct cli99_option    cli99_option;
typedef struct cli99           cli99;

struct cli99_option {
  const char* optstring; // list of options, delimited by ` ,|`
  int64_t     value;     // value returned by `cli99_GetOpt`
  uint64_t    flags;     // [0, 1, '?', '*', '+'] | [cli99_required_option] | cli99_type(...)
};

typedef struct cli99_option_internal cli99_option_internal;
struct cli99_option_internal {
  const cli99_option* option;
  uint16_t count;
  uint8_t  group;
  uint16_t long_opt_start;
  uint16_t long_opt_len;
};

struct cli99 {
  const char**   argv;    // argv
  int            argc;    // argc
  int            argi;    // current index of argv
  const char*    optopt;  // current option (cli99_GetOpt)
  const char*    optarg;  // current optarg (cli99_GetOpt, cli99_GetOptarg)
  uint64_t       flags;   // behavioural flags
  cli99_state    _state;  // internal state
  cli99_Error    error;   // last error

  union {                 // converted optarg (cli99_ConvertOptarg)
    char          c, ch;
    unsigned char uc, uch, u8;
    bool          b;
    int64_t       i;
    uint64_t      u;
    long double   d, ld, f;
  } optval;

  const char*    _arg_s; // depends on _state
  int            _arg_l; // depends on _state
  int            _positional_count;
  int            _options_size;
  bool           _end_of_options; // we have seen a "--"
  uint8_t        _groups_count;
  uint64_t       _groups;

  union {
    uint8_t short_opt[256]; // There are no short options with control charaters,
    uint8_t positional[32]; // use this space for positionals.
  } by;

  cli99_option_internal _options[cli99_OPTIONS_MAX];
};

void          cli99_Init(cli99*, int, char* const[], const cli99_option*, uint64_t);
int           cli99_GetOpt(cli99*);
bool          cli99_GetOptarg(cli99*, uint64_t);
bool          cli99_GetArg(cli99*, uint64_t);
bool          cli99_ConvertOptarg(cli99*, uint64_t);
void          cli99_SetOptions(cli99*, const cli99_option*, bool);
bool          cli99_CheckRequired(cli99*);
const char*   cli99_StrError(int);
void          cli99_ExplainError(cli99*);
void          cli99_PrintFlagMask(cli99*);
bool          cli99_StateCtl(cli99*, cli99_operation, uint64_t);

static inline bool cli99_End(cli99* self)
{ return cli99_StateCtl(self, cli99_op_is_eof, 0); }

static inline const char* cli99_LastError(cli99* self)
{ return cli99_StrError(self->error); }

#endif

/* If `--foo` is an option with an optional argument:
 *
 * no flags:
 *   [1] ./prog --foo=bar       p->optarg will be `bar`
 *   [2] ./prog --foo bar       p->optarg will be `bar`
 *   [3] ./prog --foo --bar     p->optarg will be `--bar`
 *
 * cli99_no_separate_optionals:
 *   [1] ./prog --foo=bar       p->optarg will be `bar`
 *   [2] ./prog --foo bar       p->optarg will be NULL
 *   [3] ./prog --foo --bar     p->optarg will be NULL
 *
 * cli99_no_options_as_arguments:
 *   [1] ./prog --foo=bar       p->optarg will be `bar`
 *   [2] ./prog --foo bar       p->optarg will be `bar`
 *   [3] ./prog --foo --bar     p->optarg will be NULL, because `--bar` is an option
 */

