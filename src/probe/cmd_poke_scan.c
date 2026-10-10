#include "../console.h"
#include "../sleep.h"
#include "../client/check_root.h"

#include <stdio.h> // printf

static const struct cli99_Option Poke_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map",        Option_Map,        cli99_RequiredArgument},
  {"-w|--watch",      Option_Watch,      cli99_RequiredArgument},
  {"-t|--timespan",   Option_Timespan,   cli99_RequiredArgument},
  {"-i|--interval",   Option_Interval,   cli99_RequiredArgument},
  {"-n|--no-restore", Option_NoRestore,  cli99_NoArgument      },
  {"register",        Option_Register,   cli99_NormalPositional},
  {"value",           Option_Value,      cli99_NormalPositional},
  cli99_Options_End()
};

static const struct cli99_Option Scan_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map",        Option_Map,        cli99_RequiredArgument},
  {"-w|--watch",      Option_Watch,      cli99_RequiredArgument},
  {"-t|--timespan",   Option_Timespan,   cli99_RequiredArgument},
  {"-i|--interval",   Option_Interval,   cli99_RequiredArgument},
  {"register",        Option_Register,   cli99_NormalPositional},
  cli99_Options_End()
};

// Candidate values tried by `scan`. The series exercises individual bits plus
// all-ones, which is enough to spot most registers that move the fan.
static const uint8_t Scan_Values[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };

static uint8_t Options_WatchedRegister(uint8_t target) {
  return options.watch_ref ? options.watch : target;
}

static int MaxLoops(int timespan) {
  const double loops = (double) timespan / options.interval;
  return loops >= (double) INT_MAX ? INT_MAX : MAX(1, (int) loops);
}

static unsigned int SleepMilliseconds(void) {
  const float ms = options.interval * 1000.0f;
  if (ms >= (float) UINT_MAX)
    return UINT_MAX;
  return ms > 0.0f ? (unsigned int) ms : 0;
}

// Hold a register at a fixed value for a bounded time while sampling another
// register, then restore the original value.
static int Poke(void) {
  Error e;

  check_root();
  Initialize_EC();

  if (options.value > 255) {
    Log_Error("poke: Value too big: %d", options.value);
    return NBFC_EXIT_CMDLINE;
  }

  const uint8_t target = options.register_;
  const uint8_t watch  = Options_WatchedRegister(target);
  const int timespan   = options.timespan ? options.timespan : 10;
  const int max_loops  = MaxLoops(timespan);

  uint8_t original;
  e = ec->ReadByte(target, &original);
  e_die();

  printf("Poking 0x%.2X with %d for %d second(s); watching 0x%.2X (original: %d)\n",
         target, options.value, timespan, watch, original);
  printf("  time   poke   watch\n");

  for (int i = 0; ! quit && i < max_loops; ++i) {
    e = ec->WriteByte(target, (uint8_t) options.value);
    if (e)
      goto error;

    uint8_t poke_value, watch_value;
    e = ec->ReadByte(target, &poke_value);
    if (e)
      goto error;
    e = ec->ReadByte(watch, &watch_value);
    if (e)
      goto error;

    printf("  %4.1f    %3d     %3d\n", i * options.interval, poke_value, watch_value);
    sleep_ms(SleepMilliseconds());
  }

  if (! options.no_restore) {
    e = ec->WriteByte(target, original);
    if (e)
      goto error;
    printf("Restored 0x%.2X to %d\n", target, original);
  }

  return NBFC_EXIT_SUCCESS;

error:
  Log_Error("%s", err_print_all(e));
  if (! options.no_restore)
    ec->WriteByte(target, original); // best effort
  return NBFC_EXIT_FAILURE;
}

// Try a series of candidate values on one register, sampling another register
// for each, and report the minimum/maximum observed. The original value is
// restored between candidates and at the end.
static int Scan(void) {
  Error e;

  check_root();
  Initialize_EC();

  const uint8_t target = options.register_;
  const uint8_t watch  = Options_WatchedRegister(target);
  const int timespan   = options.timespan ? options.timespan : 3;
  const int max_loops  = MaxLoops(timespan);

  uint8_t original;
  e = ec->ReadByte(target, &original);
  e_die();

  printf("Scanning 0x%.2X, watching 0x%.2X, %d second(s) per value (original: %d)\n\n",
         target, watch, timespan, original);
  printf("  value   min   max\n");

  for (size_t v = 0; v < ARRAY_SIZE(Scan_Values) && ! quit; ++v) {
    const uint8_t value = Scan_Values[v];
    int min = 256;
    int max = -1;

    for (int i = 0; ! quit && i < max_loops; ++i) {
      e = ec->WriteByte(target, value);
      if (e)
        goto error;

      uint8_t watch_value;
      e = ec->ReadByte(watch, &watch_value);
      if (e)
        goto error;

      min = MIN(min, watch_value);
      max = MAX(max, watch_value);
      sleep_ms(SleepMilliseconds());
    }

    if (max < 0)
      printf("  %5d     -     -\n", value);
    else
      printf("  %5d   %3d   %3d\n", value, min, max);

    // Start the next candidate from a known state.
    e = ec->WriteByte(target, original);
    if (e)
      goto error;
  }

  e = ec->WriteByte(target, original);
  if (e)
    goto error;
  printf("\nRestored 0x%.2X to %d\n", target, original);

  return NBFC_EXIT_SUCCESS;

error:
  Log_Error("%s", err_print_all(e));
  ec->WriteByte(target, original); // best effort
  return NBFC_EXIT_FAILURE;
}
