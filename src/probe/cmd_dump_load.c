#include "../console.h"
#include "../file_utils.h"
#include "../sleep.h"
#include "../client/check_root.h"

#include <errno.h>  // errno
#include <unistd.h> // isatty, STDOUT_FILENO
#include <stddef.h> // size_t
#include <stdio.h>  // printf
#include <string.h> // strcmp, strncmp

static const struct cli99_Option Dump_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-c|--color",    Option_Color,    cli99_NoArgument},
  {"-C|--no-color", Option_NoColor,  cli99_NoArgument},
  cli99_Options_End()
};

static const struct cli99_Option Load_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"file",          Option_File,     cli99_NormalPositional},
  cli99_Options_End()
};

static const struct cli99_Option Watch_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-t|--timespan", Option_Timespan, cli99_RequiredArgument},
  {"-i|--interval", Option_Interval, cli99_RequiredArgument},
  cli99_Options_End()
};

static const char RegisterTableHeader[] =
  "---|------------------------------------------------\n"
  "   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
  "---|------------------------------------------------\n";

/*
 * Print out a table like this:
 *
 * ---|------------------------------------------------
 *    | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 * ---|------------------------------------------------
 * 00 | 00 00 00 00 00 00 40 00 00 00 00 00 00 00 13 01
 * 10 | 01 10 30 1A FF 00 FF FF FF FF FF FF 08 00 00 01
 * 20 | 00 00 00 00 0F 00 00 00 00 01 20 00 00 36 4A 52
 * 30 | 4B 50 30 42 54 5A 49 38 41 53 4B 00 01 00 00 FF
 * 40 | FF FF FF 01 00 FF FF FF FF 4C 49 4F 4E 00 00 00
 * 50 | 00 09 FF FF 31 00 00 00 00 00 2D 41 00 02 81 1A
 * 60 | 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 70 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 80 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 90 | 33 33 33 2D 32 44 2D 33 34 48 54 30 33 30 34 31
 * A0 | 00 01 00 C7 E1 00 00 3C 00 00 00 08 64 1A 00 78
 * B0 | 3A 3A 2C 00 00 00 00 00 00 00 08 00 32 12 11 11
 * C0 | 70 00 5A 10 01 17 88 32 88 2C 08 10 5A 10 64 22
 * D0 | 00 00 00 00 00 00 5A 10 A2 01 5B 11 00 00 E0 00
 * E0 | 00 00 00 D8 10 D9 10 D8 10 00 00 83 43 03 00 81
 * F0 | 00 08 0F 00 00 00 FA 32 44 00 00 D4 56 03 00 00
 *
 * The readings can optionally be colored using the `color` parameter.
 */
static void PrintRegisterTable(RegisterReadings* readings, RegisterColors colors) {
  if (colors)
    printf(CONSOLE_RESET);

  printf("%s", RegisterTableHeader);

  for (int i = 0; i <= 0xF0; i += 0x10) {
    if (colors)
      printf(CONSOLE_RESET);

    printf("%.2X |", i);

    if (colors) {
      for (int j = 0; j <= 0x0F; ++j)
        printf("%s %.2X", colors[i + j], readings->readings[i + j]);
    }
    else {
      for (int j = 0; j <= 0x0F; ++j)
        printf(" %.2X", readings->readings[i + j]);
    }

    printf("\n");
  }
}

/*
 * Print out a table like this:
 *
 * ---|------------------------------------------------
 *    | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 * ---|------------------------------------------------
 * 00 | 00 00 00 00 00 00 40 00 00 00 00 00 00 00 13 01
 * 10 | 01 10 30 1A FF 00 FF FF FF FF FF FF 08 00 00 01
 * 20 | 00 00 00 00 0F 00 00 00 00 01 20 00 00 36 4A 52
 * 30 | 4B 50 30 42 54 5A 49 38 41 53 4B 00 01 00 00 FF
 * 40 | FF FF FF 01 00 FF FF FF FF 4C 49 4F 4E 00 00 00
 * 50 | 00 09 FF FF 31 00 00 00 00 00 2D 41 00 02 81 1A
 * 60 | 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 70 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 80 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 90 | 33 33 33 2D 32 44 2D 33 34 48 54 30 33 30 34 31
 * A0 | 00 01 00 C7 E1 00 00 3C 00 00 00 08 64 1A 00 78
 * B0 | 3A 3A 2C 00 00 00 00 00 00 00 08 00 32 12 11 11
 * C0 | 70 00 5A 10 01 17 88 32 88 2C 08 10 5A 10 64 22
 * D0 | 00 00 00 00 00 00 5A 10 A2 01 5B 11 00 00 E0 00
 * E0 | 00 00 00 D8 10 D9 10 D8 10 00 00 83 43 03 00 81
 * F0 | 00 08 0F 00 00 00 FA 32 44 00 00 D4 56 03 00 00
 *
 * The registers will be colored depending on the register's state.
 */
static void PrintWatchTable(
  RegisterReadings all_readings[],
  RegisterReadings* current,
  RegisterReadings* previous)
{
  RegisterColors colors;

  for (int register_ = 0; register_ < REGISTERS_SIZE; ++register_) {
    const uint8_t byte = current->readings[register_];
    const uint8_t diff = byte - previous->readings[register_];
    bool has_changed = false;

    uint8_t save = byte;
    for (range(RegisterReadings*, r, all_readings, previous)) {
      if (save != r->readings[register_]) {
        has_changed = true;
        break;
      }
    }

    /**/ if (diff)         colors[register_] = CONSOLE_YELLOW;
    else if (has_changed)  colors[register_] = CONSOLE_BOLD_BLUE;
    else if (byte == 0xFF) colors[register_] = CONSOLE_WHITE;
    else if (byte)         colors[register_] = CONSOLE_BOLD_WHITE;
    else                   colors[register_] = CONSOLE_BOLD_BLACK;
  }

  PrintRegisterTable(current, colors);
}

/*
 * Print out a table like this:
 *
 * ---|------------------------------------------------
 *    | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 * ---|------------------------------------------------
 * 00 | 00 00 00 00 00 00 40 00 00 00 00 00 00 00 13 01
 * 10 | 01 10 30 1A FF 00 FF FF FF FF FF FF 08 00 00 01
 * 20 | 00 00 00 00 0F 00 00 00 00 01 20 00 00 36 4A 52
 * 30 | 4B 50 30 42 54 5A 49 38 41 53 4B 00 01 00 00 FF
 * 40 | FF FF FF 01 00 FF FF FF FF 4C 49 4F 4E 00 00 00
 * 50 | 00 09 FF FF 31 00 00 00 00 00 2D 41 00 02 81 1A
 * 60 | 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 70 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 80 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 90 | 33 33 33 2D 32 44 2D 33 34 48 54 30 33 30 34 31
 * A0 | 00 01 00 C7 E1 00 00 3C 00 00 00 08 64 1A 00 78
 * B0 | 3A 3A 2C 00 00 00 00 00 00 00 08 00 32 12 11 11
 * C0 | 70 00 5A 10 01 17 88 32 88 2C 08 10 5A 10 64 22
 * D0 | 00 00 00 00 00 00 5A 10 A2 01 5B 11 00 00 E0 00
 * E0 | 00 00 00 D8 10 D9 10 D8 10 00 00 83 43 03 00 81
 * F0 | 00 08 0F 00 00 00 FA 32 44 00 00 D4 56 03 00 00
 *
 * If `use_color` is true, the registers will be colored depending
 * on the register's value.
 */
static void PrintDumpTable(RegisterReadings* register_readings, bool use_color) {
  RegisterColors colors;

  // Print table without color
  if (! use_color) {
    PrintRegisterTable(register_readings, NULL);
    return;
  }

  // Print table with color
  for (int i = 0; i < REGISTERS_SIZE; ++i) {
    switch (register_readings->readings[i]) {
      case 0x00: colors[i] = CONSOLE_BOLD_BLACK; break;
      case 0xFF: colors[i] = CONSOLE_BOLD_GREEN; break;
      default:   colors[i] = CONSOLE_BOLD_BLUE;  break;
    }
  }

  PrintRegisterTable(register_readings, colors);
  printf("%s", CONSOLE_RESET);
}

/*
 * Load a table like this:
 *
 * ---|------------------------------------------------
 *    | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 * ---|------------------------------------------------
 * 00 | 00 00 00 00 00 00 40 00 00 00 00 00 00 00 13 01
 * 10 | 01 10 30 1A FF 00 FF FF FF FF FF FF 08 00 00 01
 * 20 | 00 00 00 00 0F 00 00 00 00 01 20 00 00 36 4A 52
 * 30 | 4B 50 30 42 54 5A 49 38 41 53 4B 00 01 00 00 FF
 * 40 | FF FF FF 01 00 FF FF FF FF 4C 49 4F 4E 00 00 00
 * 50 | 00 09 FF FF 31 00 00 00 00 00 2D 41 00 02 81 1A
 * 60 | 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 70 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 80 | 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 * 90 | 33 33 33 2D 32 44 2D 33 34 48 54 30 33 30 34 31
 * A0 | 00 01 00 C7 E1 00 00 3C 00 00 00 08 64 1A 00 78
 * B0 | 3A 3A 2C 00 00 00 00 00 00 00 08 00 32 12 11 11
 * C0 | 70 00 5A 10 01 17 88 32 88 2C 08 10 5A 10 64 22
 * D0 | 00 00 00 00 00 00 5A 10 A2 01 5B 11 00 00 E0 00
 * E0 | 00 00 00 D8 10 D9 10 D8 10 00 00 83 43 03 00 81
 * F0 | 00 08 0F 00 00 00 FA 32 44 00 00 D4 56 03 00 00
 */
static Error LoadRegisterTable(RegisterReadings* register_readings, const char* file) {
  FileResult res;
  char content[4096];
  const char* text;
  char* end;

  res = File_Read(content, sizeof(content), file);
  if (! res.ok)
    return err_stringf("%s: %s", file, strerror(errno));

  if (strncmp(content, RegisterTableHeader, sizeof(RegisterTableHeader) - 1))
    goto error;

  text = content + sizeof(RegisterTableHeader) - 1;

  for (int line_no = 0; line_no < 16; ++line_no) {
    // "00 | "
    strtoull(text, &end, 16);
    if (((void*) text) == ((void*) end))
      goto error;

    if (*end == ' ')
      ++end;

    if (*end == '|')
      ++end;

    text = end;

    for (int column_no = 0; column_no < 16; ++column_no) {
      const int register_no = (line_no * 16) + column_no;

      errno = 0;
      register_readings->readings[register_no] = (uint8_t) strtoull(text, &end, 16);

      if (errno)
        goto error;

      if (((void*) text) == ((void*) end))
        goto error;

      text = end;
    }
  }

  return err_success();
error:
  return err_string("File is not a valid register dump");
}

static int Dump(void) {
  check_root();
  Initialize_EC();

  Error e;
  bool use_color = false;
  RegisterReadings register_readings;

  e = Registers_FromEC(&register_readings);
  e_die();

  switch (options.color_mode) {
  case ColorModeAuto:    use_color = isatty(STDOUT_FILENO); break;
  case ColorModeEnable:  use_color = true;                  break;
  case ColorModeDisable: use_color = false;                 break;
  }

  PrintDumpTable(&register_readings, use_color);
  return NBFC_EXIT_SUCCESS;
}

static int Load(void) {
  check_root();
  Initialize_EC();

  Error e;
  const char* infile = options.file;
  if (! strcmp(infile, "-"))
    infile = "/dev/stdin";

  RegisterReadings register_readings;
  e = LoadRegisterTable(&register_readings, infile);
  e_die();

  e = Registers_ToEC(&register_readings);
  e_die();

  return NBFC_EXIT_SUCCESS;
}

static int Watch(void) {
  check_root();
  Initialize_EC();

  Error e;
  size_t max_loops = (size_t) -1;

  if (options.timespan)
    max_loops = (size_t) ((float) options.timespan / options.interval);

  if (max_loops > ARRAY_SIZE(Registers_Log))
    max_loops = ARRAY_SIZE(Registers_Log);

  RegisterReadings* regs = Registers_Log;
  e = Registers_FromEC(regs);
  e_die();

  for (size_t loops = 1; !quit && loops < max_loops; ++loops) {
    e = Registers_FromEC(regs + loops);
    e_die();

    PrintWatchTable(regs, regs + loops, regs + loops - 1);
    sleep_ms((unsigned int) (options.interval * 1000.0f));
  }

  return NBFC_EXIT_SUCCESS;
}
