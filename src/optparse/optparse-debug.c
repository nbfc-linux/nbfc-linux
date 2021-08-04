#ifndef NDEBUG
#include <stdio.h>

static inline const char* cli99_state_tostr(uint64_t state) {
  switch (state) {
  case cli99_state_uninitialized   : return "uninitialized";
  case cli99_state_next_word       : return "next_word";
  case cli99_state_short_opt       : return "short_opt";
  case cli99_state_short_parameter : return "short_parameter";
  case cli99_state_long_opt        : return "long_opt";
  case cli99_state_long_parameter  : return "long_parameter";
  case cli99_state_positional      : return "positional";
  case cli99_state_options_end     : return "options_end";
  case cli99_state_EOF             : return "EOF";
  default:                           return "<invalid state>";
  }
}

static inline const char* cli99_operation_tostr(uint64_t operation) {
  switch (operation) {
  case cli99_op_is_eof:               return "op_is_eof";
  case cli99_op_next:                 return "op_next_state";
  case cli99_op_getoptarg:            return "op_getoptarg";
  case cli99_op_getarg:               return "op_getarg";
  case cli99_op_rewind_short_opt:     return "op_rewind_short_opt";
  default:                            return "op_unknown";
  }
}
#else
#define cli99_state_tostr(STATE)     "cli99_state_tostr() disabled by NDEBUG"
#define cli99_operation_tostr(STATE) "cli99_operation_tostr() disabled by NDEBUG"
#endif
