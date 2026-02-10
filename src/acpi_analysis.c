#include "acpi_analysis.h"

#include "memory.h"
#include "process.h"
#include "file_utils.h"
#include "regex_utils.h"
#include "nxjson_utils.h"

#include <stdint.h>
#include <string.h> // strlen
#include <stdlib.h> // strtoll, system

#define ACPI_ANALYSIS_DSDT_TEMP_FILE       "/tmp/nbfc.acpi.dsdt.XXXXXX.dat"
#define ACPI_ANALYSIS_DSDT_TEMP_SUFFIX_LEN 4 // len of `.dat`

// Example:
//   \_SB.PC00.MHBR RegionField 0x6044c80a3f80 001 Rgn [HBUS] Off 24F Len 11 Acc 04
//   `````````````` ``````````` `````````````` ``` ``` `````` ``` ``` ``` `` ``` ``
//         1                           2        3        4         5      6      7
//
// 1: Register name
// 2: Some pointer (hex)
// 3: Flags (hex)
// 4: Region
// 5: Register offset in bits (hex)
// 6: Register length in bits (hex)
// 7: Register access byte width (hex)
//
// See also:
// https://github.com/acpica/acpica (nsdump.c):
//
//   AcpiOsPrintf (" Off %.3X Len %.2X Acc %.2X\n",
//       (ObjDesc->CommonField.BaseByteOffset * 8)
//           + ObjDesc->CommonField.StartFieldBitOffset,
//       ObjDesc->CommonField.BitLength,
//       ObjDesc->CommonField.AccessByteWidth);

#define ACPI_REGION_FIELDS_RE_GROUPS 7
#define ACPI_REGION_FIELDS_RE                             \
  REGEX_GROUP("[^[:space:]]+")               /* 1 */      \
  REGEX_WHITESPACE                                        \
  "RegionField"                                           \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX_WITH_PREFIX)         /* 2 */      \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 3 */      \
  REGEX_WHITESPACE                                        \
  "Rgn"                                                   \
  REGEX_WHITESPACE                                        \
  "\\[" REGEX_GROUP("[a-zA-Z0-9_]+") "\\]"   /* 4 */      \
  REGEX_WHITESPACE                                        \
  "Off"                                                   \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 5 */      \
  REGEX_WHITESPACE                                        \
  "Len"                                                   \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 6 */      \
  REGEX_WHITESPACE                                        \
  "Acc"                                                   \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 7 */      \
  ""

// Example:
//   \_GPE._L73 Method 0x5b82899e53c0 001 Args 0 Len 0000 Aml 0x723f283c7b91
//   `````````` `````` `````````````` ``` ```` ` ``` ```` ``` ``````````````
//       1                   2         3       4      5             6
//
// 1: Method name
// 2: Some pointer (hex)
// 3: Flags (hex)
// 4: Number of methods arguments (hex)
// 5: Method length (hex)
// 6: Method start (hex)
//
// See also:
// https://github.com/acpica/acpica (nsdump.c):
//
//   AcpiOsPrintf ("Args %X Len %.4X Aml %p\n",
//       (UINT32) ObjDesc->Method.ParamCount,
//       ObjDesc->Method.AmlLength, ObjDesc->Method.AmlStart);

#define ACPI_METHODS_RE_GROUPS 6
#define ACPI_METHODS_RE                                   \
  REGEX_GROUP("[^[:space:]]+")               /* 1 */      \
  REGEX_WHITESPACE                                        \
  "Method"                                                \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX_WITH_PREFIX)         /* 2 */      \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 3 */      \
  REGEX_WHITESPACE                                        \
  "Args"                                                  \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 4 */      \
  REGEX_WHITESPACE                                        \
  "Len"                                                   \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 5 */      \
  REGEX_WHITESPACE                                        \
  "[^[:space:]]+"                                         \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX_WITH_PREFIX)         /* 6 */      \
  ""

// Example:
//   \_SB.PC00.LPCB.EC0.ERAM Region 0x58702a437da0 001 [EmbeddedControl] Addr 0000000000000000 Len 00FF
//   ``````````````````````` `````` `````````````` ```  ```````````````  ```` ```````````````` ``` ````
//               1                         2        3          4                     5              6

#define ACPI_OPERATION_REGION_RE_GROUPS 6
#define ACPI_OPERATION_REGION_RE                          \
  REGEX_GROUP("[^[:space:]]+")               /* 1 */      \
  REGEX_WHITESPACE                                        \
  "Region"                                                \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX_WITH_PREFIX)         /* 2 */      \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 3 */      \
  REGEX_WHITESPACE                                        \
  "\\[" REGEX_GROUP("[a-zA-Z0-9_]+") "\\]"   /* 4 */      \
  REGEX_WHITESPACE                                        \
  "Addr"                                                  \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 5 */      \
  REGEX_WHITESPACE                                        \
  "Len"                                                   \
  REGEX_WHITESPACE                                        \
  REGEX_GROUP(REGEX_HEX)                     /* 6 */      \
  ""

/*
 * Checks if the `iasl` program is installed.
 */
Error Acpi_Analysis_Is_IASL_Installed() {
  if (system("type " ACPI_ANALYSIS_IASL " >/dev/null 2>/dev/null") == 0)
    return err_success();

  errno = ENOENT;
  return err_stdlib(ACPI_ANALYSIS_IASL);
}

/*
 * Checks if the `acpiexec` program is installed.
 */
Error Acpi_Analysis_Is_AcpiExec_Installed() {
  if (system("type " ACPI_ANALYSIS_ACPIEXEC " >/dev/null 2>/dev/null") == 0)
    return err_success();

  errno = ENOENT;
  return err_stdlib(ACPI_ANALYSIS_ACPIEXEC);
}

void AcpiRegister_Free(AcpiRegister* acpi_register) {
  Mem_Free(acpi_register->name);
}

void AcpiMethod_Free(AcpiMethod* acpi_method) {
  Mem_Free(acpi_method->name);
}

void AcpiRegion_Free(AcpiOperationRegion* acpi_region) {
  Mem_Free(acpi_region->name);
  Mem_Free(acpi_region->type);
}

void AcpiInfo_Free(AcpiInfo* acpi_info) {
  for_each_array(AcpiMethod*, method, acpi_info->methods) {
    AcpiMethod_Free(method);
  }
  Mem_Free(acpi_info->methods.data);

  for_each_array(AcpiRegister*, register_, acpi_info->registers) {
    AcpiRegister_Free(register_);
  }
  Mem_Free(acpi_info->registers.data);

  for_each_array(AcpiOperationRegion*, region, acpi_info->regions) {
    AcpiRegion_Free(region);
  }
  Mem_Free(acpi_info->regions.data);

  Mem_Free(acpi_info->ec_region_names.data);

  memset(acpi_info, 0, sizeof(*acpi_info));
}

/*
 * Extracts a list or registers.
 *
 * Parses output from `acpiexec -b 'Objects RegionField'`.
 */
static Error Acpi_Analysis_Extract_Registers(const char* output, array_of(AcpiRegister)* out) {
  regex_t regex;
  regmatch_t matches[ACPI_REGION_FIELDS_RE_GROUPS + 1];
  size_t num_matches;

  // Clear output array
  memset(out, 0, sizeof(*out));

  // Early return (because we cannot free an uninitialized regex_t)
  if (regcomp(&regex, ACPI_REGION_FIELDS_RE, REG_EXTENDED))
    return err_string("Invalid regular expression");

  // Allocate space for output array
  num_matches = RegEx_Count(&regex, matches, ACPI_REGION_FIELDS_RE_GROUPS + 1, output);
  out->data = Mem_Calloc(num_matches, sizeof(AcpiRegister));

  // Iterate over matches and fill output array
  const char* text = output;
  while (regexec(&regex, text, ACPI_REGION_FIELDS_RE_GROUPS + 1, matches, 0) == 0) {
    AcpiRegister* acpi_register = &out->data[out->size++];

    acpi_register->name = RegEx_SubStr(&matches[1], text);
    RegEx_SubStr_Fixed(&matches[4], text, acpi_register->region, sizeof(AcpiOperationRegion));
    acpi_register->bit_offset = RegEx_Strtoll(&matches[5], text, 16);
    acpi_register->bit_length = RegEx_Strtoll(&matches[6], text, 16);
    acpi_register->access_byte_width = RegEx_Strtoll(&matches[7], text, 16);

    text += matches[0].rm_eo;
  }

  regfree(&regex);
  return err_success();
}

/*
 * Extracts a list of methods.
 *
 * Parses output from `acpiexec -b 'Methods'`.
 */
static Error Acpi_Analysis_Extract_Methods(const char* output, array_of(AcpiMethod)* out) {
  regex_t regex;
  regmatch_t matches[ACPI_METHODS_RE_GROUPS + 1];
  size_t num_matches;

  // Clear output array
  memset(out, 0, sizeof(*out));

  // Early return (because we cannot free an uninitialized regex_t)
  if (regcomp(&regex, ACPI_METHODS_RE, REG_EXTENDED))
    return err_string("Invalid regular expression");

  // Allocate space for output array
  num_matches = RegEx_Count(&regex, matches, ACPI_METHODS_RE_GROUPS + 1, output);
  out->data = Mem_Calloc(num_matches, sizeof(AcpiMethod));

  // Iterate over matches and fill output array
  const char* text = output;
  while (regexec(&regex, text, ACPI_METHODS_RE_GROUPS + 1, matches, 0) == 0) {
    AcpiMethod* acpi_method = &out->data[out->size++];

    acpi_method->name = RegEx_SubStr(&matches[1], text);
    acpi_method->length = RegEx_Strtoll(&matches[4], text, 16);

    text += matches[0].rm_eo;
  }

  regfree(&regex);
  return err_success();
}

/*
 * Extracts a list of operation regions.
 *
 * Parses output from `acpiexec -b 'Objects Region'`.
 */
static Error Acpi_Analysis_Extract_OperationRegions(const char* output, array_of(AcpiOperationRegion)* out) {
  regex_t regex;
  regmatch_t matches[ACPI_OPERATION_REGION_RE_GROUPS + 1];
  size_t num_matches;

  // Clear output array
  memset(out, 0, sizeof(*out));

  // Early return (because we cannot free an uninitialized regex_t)
  if (regcomp(&regex, ACPI_OPERATION_REGION_RE, REG_EXTENDED))
    return err_string("Invalid regular expression");

  num_matches = RegEx_Count(&regex, matches, ACPI_OPERATION_REGION_RE_GROUPS + 1, output);
  out->data = Mem_Calloc(num_matches, sizeof(AcpiOperationRegion));

  // Iterate over matches and fill output array
  const char* text = output;
  while (regexec(&regex, text, ACPI_OPERATION_REGION_RE_GROUPS + 1, matches, 0) == 0) {
    AcpiOperationRegion* region = &out->data[out->size++];

    region->name = RegEx_SubStr(&matches[1], text);
    region->type = RegEx_SubStr(&matches[4], text);

    text += matches[0].rm_eo;
  }

  regfree(&regex);
  return err_success();
}

/*
 * Extracts a list of registers, methods and operation regions from the
 * DSDT file.
 *
 * This function requires the `acpiexec` program.
 */
Error Acpi_Analysis_Get_Info(const char* file, AcpiInfo* out) {
  Error e = err_success();

  // Clear output arrays
  memset(out, 0, sizeof(*out));

  char* stdout_ = NULL;
  char* stderr_ = NULL;
  char* argv[] = {
    Mem_Strdup(ACPI_ANALYSIS_ACPIEXEC),
    Mem_Strdup("-dt"),
    Mem_Strdup("-di"),
    Mem_Strdup("-b"),
    Mem_Strdup("Objects RegionField; Objects Region; Methods; Exit"),
    Mem_Strdup(file),
    NULL
  };

  int ret = Process_Capture(ACPI_ANALYSIS_ACPIEXEC, argv, &stdout_, &stderr_);

  if (ret == -1) {
    e = err_stdlib(ACPI_ANALYSIS_ACPIEXEC);
    goto end;
  }

  if (ret != 0) {
    e = err_stringf(ACPI_ANALYSIS_ACPIEXEC " returned %d", ret);
    goto end;
  }

  if (! stdout_) {
    e = err_string(ACPI_ANALYSIS_ACPIEXEC " returned no output");
    goto end;
  }

  e = Acpi_Analysis_Extract_Registers(stdout_, &out->registers);
  if (e)
    goto end;

  e = Acpi_Analysis_Extract_Methods(stdout_, &out->methods);
  if (e)
    goto end;

  e = Acpi_Analysis_Extract_OperationRegions(stdout_, &out->regions);
  if (e)
    goto end;

  array_size_t num_ec_regions = 0;
  for_each_array(AcpiOperationRegion*, region, out->regions)
    num_ec_regions += !strcmp(region->type, "EmbeddedControl");

  out->ec_region_names.size = 0;
  out->ec_region_names.data = Mem_Calloc(num_ec_regions, sizeof(AcpiOperationRegionName));

  for_each_array(AcpiOperationRegion*, region, out->regions) {
    if (strcmp(region->type, "EmbeddedControl"))
      continue;

    const char* const name = Acpi_Analysis_Get_Register_Basename(region->name);
    snprintf(
      out->ec_region_names.data[out->ec_region_names.size++],
      sizeof(AcpiOperationRegionName),
      "%s",
      name);
  }

end:
  Mem_Free(argv[0]);
  Mem_Free(argv[1]);
  Mem_Free(argv[2]);
  Mem_Free(argv[3]);
  Mem_Free(argv[4]);
  Mem_Free(argv[5]);
  Mem_Free(stdout_);
  Mem_Free(stderr_);
  return e;
}

/*
 * Wrapper around `mkstemps` that also copies the contents of `file` to
 * `template_out`.
 */
static Error CopyToTemp(const char* file, char* template_out, int template_suffix_len) {
  Error e = err_success();
  char buf[4096];
  int infd = -1;
  int outfd = -1;
  ssize_t n;

  // Early return, because `template_out` is undefined if `mkstemps` fails.
  outfd = mkstemps(template_out, template_suffix_len);
  if (outfd < 0) {
    e = err_stdlib(template_out);
    return e;
  }

  infd = open(file, O_RDONLY);
  if (infd < 0) {
    e = err_stdlib(file);
    goto end;
  }

  while ((n = read(infd, buf, sizeof(buf))) > 0) {
    ssize_t w = 0;
    while (w < n) {
      ssize_t written = write(outfd, buf + w, n - w);
      if (written < 0) {
        e = err_stdlib(template_out);
        goto end;
      }
      w += written;
    }
  }

  if (n < 0) {
    e = err_stdlib(file);
    goto end;
  }

end:
  if (e)
    unlink(template_out);

  if (infd >= 0)
    close(infd);

  if (outfd >= 0)
    close(outfd);

  return e;
}

/*
 * Extracts the DSL for a DSDT file and stores the result in `out`
 *
 * This function requires the `iasl` program.
 */
Error Acpi_Analysis_Get_DSL(const char* file, char** out) {
  Error e;

  *out = NULL;

  // Early return (because `temp_file` is undefined if CopyToTemp() fails)
  char temp_file[PATH_MAX] = ACPI_ANALYSIS_DSDT_TEMP_FILE;
  e = CopyToTemp(file, temp_file, ACPI_ANALYSIS_DSDT_TEMP_SUFFIX_LEN);
  if (e)
    return e;

  char* stdout_ = NULL;
  char* stderr_ = NULL;
  char* argv[] = {
    Mem_Strdup(ACPI_ANALYSIS_IASL),
    Mem_Strdup("-d"),
    Mem_Strdup(temp_file),
    NULL
  };

  // Run `iasl -d temp_file` and immediately remove `temp_file`
  int ret = Process_Capture(ACPI_ANALYSIS_IASL, argv, &stdout_, &stderr_);
  int errno_save = errno;
  unlink(temp_file);
  errno = errno_save;

  // `temp_file` now holds the filename of the generated DSL
  size_t len = strlen(temp_file);
  temp_file[len - 3] = 'd';
  temp_file[len - 2] = 's';
  temp_file[len - 1] = 'l';

  if (ret == -1) {
    e = err_stdlib(ACPI_ANALYSIS_IASL);
    goto end;
  }

  if (ret != 0) {
    e = err_stringf(ACPI_ANALYSIS_IASL " returned %d", ret);
    goto end;
  }

  ssize_t nread = slurp_file_dynamic(out, temp_file);
  if (nread < 0) {
    e = err_stdlib(temp_file);
    goto end;
  }

end:
  unlink(temp_file); // remove the generated *.dsl file
  Mem_Free(argv[0]);
  Mem_Free(argv[1]);
  Mem_Free(argv[2]);
  Mem_Free(stdout_);
  Mem_Free(stderr_);
  return e;
}

/*
 * Converts an ACPI path segment (`EC0, EC0_`) to an uint32_t.
 *
 * Reads up to 4 characters and pads missing characters with '_'.
 *
 * Advances the pointer to the end of the segment.
 *
 * Parsing stops at NUL, '.' or any whitespace / control characters.
 */
static uint32_t Acpi_Analysis_Segment_To_UInt(const char** s) {
  uint32_t val = 0;

  for (int i = 0; i < 4; ++i) {
    char c = **s;

    // Check for NUL-byte, whitespace and dot
    if (c <= 32 || c == '.')
      c = '_';
    else
      (*s)++;

    val = (val << 8) | (uint8_t) c;
  }

  return val;
}

/*
 * Checks if both segments are equal while advancing both pointers to the
 * end of the segments.
 */
static inline bool Acpi_Analysis_Segment_Equal(const char** s1, const char** s2) {
  const uint32_t i1 = Acpi_Analysis_Segment_To_UInt(s1);
  const uint32_t i2 = Acpi_Analysis_Segment_To_UInt(s2);
  return i1 == i2;
}

/*
 * Checks if both ACPI paths are equal.
 *
 * Both input strings are parsed until NUL or whitespace / control characters.
 *
 * The path may start with a backslash.
 */
bool Acpi_Analysis_Path_Equals(const char* s1, const char* s2) {
  // Check for leading backslash
  if (*s1 == '\\' || *s2 == '\\') {
    if (*s1 != *s2)
      return false;

    ++s1;
    ++s2;
  }

  while (1) {
    // End of path
    if (*s1 <= 32 || *s2 <= 32)
      return (*s1 <= 32 && *s2 <= 32);

    if (! Acpi_Analysis_Segment_Equal(&s1, &s2))
      return false;

    if (*s1 == '.' || *s2 == '.') {
      if (*s1 != *s2)
        return false;

      ++s1;
      ++s2;
    }
  }
}

/*
 * Returns the basename of a register.
 *
 * Example:
 *   Input:  \_SB.PC00.LPCB.EC0.ERBD
 *   Output: ERDB
 */
const char* Acpi_Analysis_Get_Register_Basename(const char* path) {
  const size_t len = strlen(path);
  const char* p = path + len;

  while (p > path) {
    if (p[-1] == '.' || p[-1] == '\\')
      break;

    p--;
  }

  return p;
}

/*
 * Converts an ACPI method to a JSON object and attaches it to parent under
 * the given key.
 */
nx_json* AcpiMethod_ToJson(AcpiMethod* method, const char* key, nx_json* parent) {
  nx_json* object = create_json_object(key, parent);
  create_json_string("name", object, method->name);
  create_json_integer("length", object, method->length);
  return object;
}

/*
 * Converts an ACPI register to a JSON object and attaches it to parent under
 * the given key.
 */
nx_json* AcpiRegister_ToJson(AcpiRegister* register_, const char* key, nx_json* parent) {
  nx_json* object = create_json_object(key, parent);
  create_json_string("name", object, register_->name);
  create_json_string("region", object, register_->region);
  create_json_integer("bit_offset", object, register_->bit_offset);
  create_json_integer("bit_length", object, register_->bit_length);
  create_json_integer("access_byte_width", object, register_->access_byte_width);
  return object;
}
