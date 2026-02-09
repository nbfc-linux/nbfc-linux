#ifndef NBFC_CONFIG_RATING_RULES_H_
#define NBFC_CONFIG_RATING_RULES_H_

#include "macros.h"
#include "nxjson.h"
#include "acpi_analysis.h"

/*
 * Default rules for rating a configuration.
 */
#define CONFIG_RATING_DEFAULT_RULES             \
  "{"                                           \
    "\"FanRegisterFullMatch\":["                \
      "{\"Name\":\"PFAN\",\"Mode\":\"rw\"},"    \
      "{\"Name\":\"SFAN\",\"Mode\":\"rw\"},"    \
      "{\"Name\":\"CFAN\",\"Mode\":\"rw\"},"    \
      "{\"Name\":\"XFAN\",\"Mode\":\"rw\"},"    \
      "{\"Name\":\"FAN1\",\"Mode\":\"rw\"},"    \
      "{\"Name\":\"FSW1\",\"Mode\":\"rw\"},"    \
      "{\"Name\":\"FRDC\",\"Mode\":\"r\"},"     \
      "{\"Name\":\"FTGC\",\"Mode\":\"w\"},"     \
      "{\"Name\":\"FR2C\",\"Mode\":\"r\"},"     \
      "{\"Name\":\"FT2C\",\"Mode\":\"w\"}"      \
    "],"                                        \
                                                \
    "\"FanRegisterPartialMatch\":["             \
      "\"FAN\","                                \
      "\"RPM\","                                \
      "\"PWM\""                                 \
    "],"                                        \
                                                \
    "\"RegisterWriteFullMatch\":["              \
      "\"TEMP\","                               \
      "\"CRZN\","                               \
      "\"FSH1\""                                \
    "],"                                        \
                                                \
    "\"RegisterWritePartialMatch\":["           \
    "],"                                        \
                                                \
    "\"BadRegisterFullMatch\":["                \
      "\"FBCM\","                               \
      "\"FBGI\","                               \
      "\"FBAE\","                               \
      "\"FBCB\","                               \
      "\"FBW1\","                               \
      "\"FBW2\","                               \
      "\"FBID\","                               \
      "\"FUAE\","                               \
      "\"FRPS\""                                \
    "],"                                        \
                                                \
    "\"BadRegisterPartialMatch\":["             \
      "\"BAT\""                                 \
    "]"                                         \
  "}"

/*
 * Bitwise flags specifying which fan access modes a register rule applies to.
 *
 * Read:
 *   Match fan registers used for reading.
 *
 * Write:
 *   Match fan registers used for writing.
 */
enum NBFC_PACKED_ENUM RegisterRuleFanMode {
  RegisterRuleFanMode_None  = 0x00,
  RegisterRuleFanMode_Read  = 0x01,
  RegisterRuleFanMode_Write = 0x02,
};

/*
 * Defines a rule for matching a EC register by name and mode.
 *
 * Name:
 *   The name of the EC register.
 *
 * Mode:
 *   The mode of the EC register.
 */
struct RegisterRule {
  AcpiRegisterName Name;
  enum RegisterRuleFanMode Mode;
};
typedef struct RegisterRule RegisterRule;
declare_array_of(RegisterRule);

/*
 * Rules for rating a configuration.
 *
 * Defines how register names are matched during rating:
 *
 * FanRegisterFullMatch:
 *   Register in FanConfiguration must match exactly (whole name + mode).
 *
 * FanRegisterPartialMatch:
 *   Register in FanConfiguration must contain the given name.
 *
 * RegisterWriteFullMatch:
 *   Register in RegisterWriteConfiguration must match exactly (whole name).
 *
 * RegisterPartialMatch:
 *   Register in RegisterWriteConfiguration must contain the given name.
 *
 * BadRegisterFullMatch:
 *   Register must match exactly (whole name).
 *
 * BadRegisterPartialMatch:
 *   Register must contain given name.
 */
struct ConfigRatingRules {
  array_of(RegisterRule)     FanRegisterFullMatch;
  array_of(AcpiRegisterName) FanRegisterPartialMatch;
  array_of(AcpiRegisterName) RegisterWriteFullMatch;
  array_of(AcpiRegisterName) RegisterWritePartialMatch;
  array_of(AcpiRegisterName) BadRegisterFullMatch;
  array_of(AcpiRegisterName) BadRegisterPartialMatch;
};
typedef struct ConfigRatingRules ConfigRatingRules;
declare_array_of(ConfigRatingRules);

Error ConfigRatingRules_FromJson(ConfigRatingRules*, const char*);
void  ConfigRatingRules_Free(ConfigRatingRules*);
void  ConfigRatingRules_Print(ConfigRatingRules*);
nx_json* ConfigRatingRules_ToJson(ConfigRatingRules*);

#endif
