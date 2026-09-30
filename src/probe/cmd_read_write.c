#include "../to_binary.h"
#include "../client/check_root.h"

#include <stdio.h> // printf, snprintf

static const struct cli99_Option Read_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map",    Option_Map,       cli99_RequiredArgument},
  {"-w|--word",   Option_Word,      cli99_NoArgument      },
  {"-f|--format", Option_Format,    cli99_RequiredArgument},
  {"register",    Option_Register,  cli99_NormalPositional},
  cli99_Options_End()
};

static const struct cli99_Option Write_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map",    Option_Map,       cli99_RequiredArgument},
  {"-w|--word",   Option_Word,      cli99_NoArgument      },
  {"register",    Option_Register,  cli99_NormalPositional},
  {"value",       Option_Value,     cli99_NormalPositional},
  cli99_Options_End()
};

static const struct cli99_Option ReadBit_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map",    Option_Map,       cli99_RequiredArgument},
  {"register",    Option_Register,  cli99_NormalPositional},
  {"bit_offset",  Option_BitOffset, cli99_NormalPositional},
  cli99_Options_End()
};

static const struct cli99_Option WriteBit_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map",    Option_Map,       cli99_RequiredArgument},
  {"-d|--dry",    Option_Dry,       cli99_NoArgument      },
  {"register",    Option_Register,  cli99_NormalPositional},
  {"bit_offset",  Option_BitOffset, cli99_NormalPositional},
  {"value",       Option_BitValue,  cli99_NormalPositional},
  cli99_Options_End()
};

static const char* FormatValue(char* buf, size_t bufsz, char fmt, uint16_t val, bool word) {
  switch (fmt) {
  case 'b': /* fall-through */
  case 'B':
    snprintf(buf, bufsz, "0b%s", to_binary(val, (word ? 16U : 8U)));
    break;

  case 'd': /* fall-through */
  case 'D':
    snprintf(buf, bufsz, "%d", val);
    break;

  case 'x':
    snprintf(buf, bufsz, "0x%.*x", (word ? 4 : 2), val);
    break;

  case 'X':
    snprintf(buf, bufsz, "0x%.*X", (word ? 4 : 2), val);
    break;

  default:
    if (word)
      snprintf(buf, bufsz, "%d (0x%.4X 0b%s)", val, val, to_binary(val, 16U));
    else
      snprintf(buf, bufsz, "%d (0x%.2X 0b%s)", val, val, to_binary(val, 8U));
  }

  return buf;
}

static int Read(void) {
  check_root();
  Initialize_EC();

  char buf[128];

  if (options.use_word) {
    uint16_t word;
    Error e = ec->ReadWord(options.register_, &word);
    e_die();
    printf("%s\n", FormatValue(buf, sizeof(buf), options.format, word, true));
  }
  else {
    uint8_t byte;
    Error e = ec->ReadByte(options.register_, &byte);
    e_die();
    printf("%s\n", FormatValue(buf, sizeof(buf), options.format, byte, false));
  }

  return NBFC_EXIT_SUCCESS;
}

static int Write(void) {
  check_root();
  Initialize_EC();

  if (options.use_word) {
    Error e = ec->WriteWord(options.register_, options.value);
    e_die();
  }
  else {
    if (options.value > 255) {
      Log_Error("write: Value too big: %d", options.value);
      return NBFC_EXIT_CMDLINE;
    }
    Error e = ec->WriteByte(options.register_, (uint8_t) options.value);
    e_die();
  }

  return NBFC_EXIT_SUCCESS;
}

static int ReadBit(void) {
  check_root();
  Initialize_EC();

  uint8_t byte;
  Error e = ec->ReadByte(options.register_, &byte);
  e_die();

  uint8_t bit = (byte >> (options.bit_offset)) & 1;
  printf("%d\n", bit);
  return NBFC_EXIT_SUCCESS;
}

static int WriteBit(void) {
  Error e;
  check_root();
  Initialize_EC();

  uint8_t byte;
  e = ec->ReadByte(options.register_, &byte);
  e_die();

  uint8_t mask = 1U << (options.bit_offset);
  uint8_t new_byte = options.bit_value ? (byte | mask) : (byte & ~mask);

  if (options.dry) {
    Log_Info("Dry run: Write %d (0x%.2X 0b%s)", new_byte, new_byte, to_binary(new_byte, 8U));
  }
  else {
    e = ec->WriteByte(options.register_, new_byte);
    e_die();
  }

  return NBFC_EXIT_SUCCESS;
}
