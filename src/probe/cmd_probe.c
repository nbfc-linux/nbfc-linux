#include "../range.h"
#include "../sleep.h"
#include "../client/check_root.h"

#include <stdio.h>  // printf
#include <string.h> // strcmp, strcspn, memcpy

static const struct cli99_Option Probe_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-d|--dry",      Option_Dry,              cli99_NoArgument      },
  {"-w|--word",     Option_Word,             cli99_NoArgument      },
  {"-m|--map",      Option_Map,              cli99_RequiredArgument},
  {"-r|--read",     Option_ReadRegister,     cli99_RequiredArgument},
  {"--read-word",   Option_ReadWordRegister, cli99_RequiredArgument},
  {"-i|--interval", Option_Interval,         cli99_RequiredArgument},
  {"-t|--timespan", Option_Timespan,         cli99_RequiredArgument},
  {"register",      Option_Register,         cli99_NormalPositional},
  {"values",        Option_Values,           cli99_NormalPositional},
  cli99_Options_End()
};

static inline size_t Probe_StrCountCommas(const char* s) {
  size_t count = 0;
  for (; *s; ++s)
    count += (*s == ',');
  return count;
}

static bool Probe_ParsePreset(const char* s, bool word, Range* out) {
  if (! strcmp(s, "all")) {
    out->start = 0;
    out->stop = word ? UINT16_MAX : UINT8_MAX;
    out->step = 1;
    out->type = Range_Add;
    return true;
  }

  if (! strcmp(s, "all_reversed")) {
    out->start = word ? UINT16_MAX : UINT8_MAX;
    out->stop = 0;
    out->step = 1;
    out->type = Range_Sub;
    return true;
  }

  if (! strcmp(s, "bits")) {
    out->start = 1;
    out->stop = word ? 32768 : 128;
    out->step = 2;
    out->type = Range_Mul;
    return true;
  }

  if (! strcmp(s, "bits_reversed")) {
    out->start = word ? 32768 : 128;
    out->stop = 1;
    out->step = 2;
    out->type = Range_Div;
    return true;
  }

  return false;
}

static Error Probe_ParseRanges(const char* s, bool word, array_of(Range)* out) {
  Error e;
  array_size_t size = 0;
  array_calloc(Range, *out, (Probe_StrCountCommas(s) + 1));
  const uint64_t max = word ? UINT16_MAX : UINT8_MAX;

  if (!*s) {
    e = err_string("Empty values");
    goto error;
  }

  while (*s) {
    char buf[1024] = {0};
    const size_t len = strcspn(s, ",");

    if (len >= sizeof(buf) - 1) {
      e = err_string("Range too long");
      goto error;
    }

    memcpy(buf, s, len);

    if (! Probe_ParsePreset(buf, word, &out->data[size])) {
      e = Range_Parse(buf, &out->data[size]);
      if (e) {
        e = err_chain_string(e, buf);
        goto error;
      }
    }

    if (! Range_IsInRange(&out->data[size], 0, max)) {
      e = err_stringf("Values not in range of 0 - %" PRIu64 ": %s", max, buf);
      goto error;
    }

    size++;
    if (s[len] == '\0')
      break;
    s += len + 1;
  }

  out->size = size;
  return err_success();

error:
  Mem_Free(out->data);
  return e;
}

struct Probe_Read {
  const char* ref;
  uint8_t     addr;
  uint16_t    value;
  uint16_t    min;
  uint16_t    max;
  bool        word;
  bool        enabled;
};

struct Probe_Context {
  Error           error;
  array_of(Range) ranges;
  bool            dry;
  float           timespan;
  float           interval;
  float           elapsed;

  struct {
    const char*   ref;
    uint8_t       addr;
    uint16_t      saved;
    bool          word;
  } write;

  struct Probe_Read read[4];
};

static void Probe_PrintHeader(struct Probe_Context* ctxt) {
  printf("value");

  for (size_t i = 0; i < ARRAY_SIZE(ctxt->read); ++i) {
    if (! ctxt->read[i].enabled)
      break;

    if (ctxt->read[i].word)
      printf(" | %5s (min  , max  )", ctxt->read[i].ref);
    else
      printf(" | %4s (min, max)", ctxt->read[i].ref);
  }

  printf("\n");
}

static void Probe_Print(struct Probe_Context* ctxt, uint16_t written) {
  printf("%5d", written);

  for (size_t i = 0; i < ARRAY_SIZE(ctxt->read); ++i) {
    if (! ctxt->read[i].enabled)
      break;

    if (ctxt->read[i].word)
      printf(" | %5d (%5d, %5d)", ctxt->read[i].value, ctxt->read[i].min, ctxt->read[i].max);
    else
      printf(" | %4d (%3d, %3d)", ctxt->read[i].value, ctxt->read[i].min, ctxt->read[i].max);
  }

  printf("\n");
}

static bool Probe_LoopBody(struct Probe_Context* ctxt, uint64_t value) {
  if (! ctxt->dry) {
    if (ctxt->write.word) {
      if ((ctxt->error = ec->WriteWord(ctxt->write.addr, (uint16_t) value)))
        return false;
    }
    else {
      if ((ctxt->error = ec->WriteByte(ctxt->write.addr, (uint8_t) value)))
        return false;
    }
  }

  for (size_t i = 0; i < ARRAY_SIZE(ctxt->read); ++i) {
    if (! ctxt->read[i].enabled)
      break;

    if (ctxt->read[i].word) {
      uint16_t readval;
      if ((ctxt->error = ec->ReadWord(ctxt->read[i].addr, &readval)))
        return false;
      ctxt->read[i].value = readval;
    }
    else {
      uint8_t readval;
      if ((ctxt->error = ec->ReadByte(ctxt->read[i].addr, &readval)))
        return false;
      ctxt->read[i].value = readval;
    }

    ctxt->read[i].min = MIN(ctxt->read[i].min, ctxt->read[i].value);
    ctxt->read[i].max = MAX(ctxt->read[i].max, ctxt->read[i].value);
  }

  Probe_Print(ctxt, value);

  if (quit)
    return false;

  if (ctxt->timespan && ctxt->elapsed >= ctxt->timespan)
    return false;

  ctxt->elapsed += ctxt->interval;
  sleep_ms((unsigned int) (ctxt->interval * 1000.0f));
  return true;
}

static void Probe_Loop(struct Probe_Context* ctxt) {
  while (true) {
    for_each_array(Range*, range, ctxt->ranges) {
      uint64_t value = Range_Begin(range);

      do {
        if (! Probe_LoopBody(ctxt, value))
          return;
      }
      while (Range_Iterate(range, &value));
    }
  }
}

static int Probe(void) {
  Error e;
  struct Probe_Context ctxt = {0};

  ctxt.error        = err_success();
  ctxt.ranges.data  = NULL;
  ctxt.ranges.size  = 0;
  ctxt.dry          = options.dry;
  ctxt.timespan     = options.timespan;
  ctxt.interval     = options.interval;
  ctxt.elapsed      = 0.0f;
  ctxt.write.ref    = options.register_ref;
  ctxt.write.addr   = options.register_;
  ctxt.write.word   = options.use_word;
  ctxt.write.saved  = 0;

  if (ctxt.write.word && ctxt.write.addr > 254) {
    Log_Error("Register > 254: %s", ctxt.write.ref);
    return NBFC_EXIT_CMDLINE;
  }

  for (size_t i = 0; i < ARRAY_SIZE(options.read_registers); ++i) {
    if (options.read_registers[i].register_ref) {
      ctxt.read[i].ref = options.read_registers[i].register_ref;
      ctxt.read[i].word = options.read_registers[i].word;
      ctxt.read[i].enabled = true;
      ctxt.read[i].addr = RegisterResolve(ctxt.read[i].ref);
      ctxt.read[i].min = (ctxt.read[i].word ? UINT16_MAX : UINT8_MAX);
      ctxt.read[i].max = 0;
      if (ctxt.read[i].word && ctxt.read[i].addr > 254) {
        Log_Error("Register > 254: %s", ctxt.read[i].ref);
        return NBFC_EXIT_CMDLINE;
      }
    }
  }

  check_root();
  Initialize_EC();

  e = Probe_ParseRanges(options.values, ctxt.write.word, &ctxt.ranges);
  e_die();

  if (ctxt.write.word) {
    uint16_t saved;
    e = ec->ReadWord(ctxt.write.addr, &saved);
    ctxt.write.saved = saved;
    e_die();
  }
  else {
    uint8_t saved;
    e = ec->ReadByte(ctxt.write.addr, &saved);
    ctxt.write.saved = saved;
    e_die();
  }

  Log_Info("Writing to %s (original value: %d)", ctxt.write.ref, ctxt.write.saved);

  Probe_PrintHeader(&ctxt);
  Probe_Loop(&ctxt);
  Mem_Free(ctxt.ranges.data);

  if (ctxt.error)
    Log_Error("%s", err_print_all(ctxt.error));

  if (! ctxt.dry) {
    Log_Info("Restoring %s's value (%d)", ctxt.write.ref, ctxt.write.saved);

    if (ctxt.write.word)
      e = ec->WriteWord(ctxt.write.addr, (uint16_t) ctxt.write.saved);
    else
      e = ec->WriteByte(ctxt.write.addr, (uint8_t) ctxt.write.saved);

    if (e) {
      Log_Error("Failed to restore %s's value: %s", ctxt.write.ref, err_print_all(e));
      return NBFC_EXIT_FAILURE;
    }
  }

  return ctxt.error ? NBFC_EXIT_FAILURE : NBFC_EXIT_SUCCESS;
}
