#ifndef NBFC_CONFIG_RATING_H_
#define NBFC_CONFIG_RATING_H_

#include "macros.h"
#include "model_config.h"
#include "acpi_analysis.h"

/*
 * Default rules for rating a configuration.
 */
#define CONFIG_RATING_DEFAULT_RULES    \
  "{"                                  \
    "\"FanConfiguration\":"            \
      "{"                              \
        "\"RegisterFullMatch\":["      \
          "\"PFAN\","                  \
          "\"SFAN\","                  \
          "\"CFAN\","                  \
          "\"XFAN\","                  \
          "\"FAN1\","                  \
          "\"FSW1\","                  \
          "\"FRDC\","                  \
          "\"FTGC\","                  \
          "\"FR2C\","                  \
          "\"FT2C\""                   \
        "],"                           \
                                       \
        "\"RegisterPartialMatch\":["   \
          "\"FAN\","                   \
          "\"RPM\","                   \
          "\"PWM\""                    \
        "]"                            \
      "},"                             \
                                       \
    "\"RegisterWriteConfiguration\":"  \
      "{"                              \
        "\"RegisterFullMatch\":["      \
          "\"TEMP\","                  \
          "\"CRZN\","                  \
          "\"FSH1\","                  \
        "],"                           \
                                       \
        "\"RegisterPartialMatch\":["   \
        "]"                            \
      "}"                              \
  "}"

/*
 * Rules for rating a configuration.
 *
 * Defines how register names are matched during rating:
 *
 * FanRegistersFullMatch:
 *   Register in FanConfiguration must match exactly (whole name).
 *
 * FanRegistersPartialMatch:
 *   Register in FanConfiguration must contain the given name.
 *
 * RegisterWriteFullMatch:
 *   Register in RegisterWriteConfiguration must match exactly (whole name).
 *
 * RegisterPartialMatch:
 *   Register in RegisterWriteConfiguration must contain the given name.
 */
struct ConfigRatingRules {
  array_of(AcpiRegisterName) FanRegistersFullMatch;
  array_of(AcpiRegisterName) FanRegistersPartialMatch;
  array_of(AcpiRegisterName) RegisterWriteFullMatch;
  array_of(AcpiRegisterName) RegisterWritePartialMatch;
};
typedef struct ConfigRatingRules ConfigRatingRules;
declare_array_of(ConfigRatingRules);

/*
 * Main struct for rating.
 *
 * Contains ACPI information and the rating rules.
 *
 * methods:
 *   Contains information about ACPI methods.
 *
 * registers:
 *   Contains information about ACPI registers.
 *
 * ec_operation_regions:
 *   Holds the names of EC operation regions that expose registers
 *   accessible via the embedded controller.
 *
 * rules:
 *   Contains the rules for rating configurations.
 */
struct ConfigRating {
  array_of(AcpiMethod) methods;
  array_of(AcpiRegister) registers;
  array_of(AcpiOperationRegion) ec_operation_regions;
  ConfigRatingRules rules;
};
typedef struct ConfigRating ConfigRating;
declare_array_of(ConfigRating);

/*
 * Specifies the category of a register, e.g. whether it is a fan register or
 * a miscellaneous register used for register-write configurations.
 */
enum RegisterType {
  RegisterType_FanRegister,
  RegisterType_RegisterWriteConfigurationRegister
};

/*
 * Indicates how well an EC register name matches the firmware according to the
 * configuration rules.
 *
 * FullMatch:
 *   Exact match against the configuration rules.
 *
 * PartialMatch:
 *   Partial match against the configuration rules.
 *
 * MinimalMatch:
 *   Minimal match.
 *   Register name starts with 'F' (probably a fan register).
 *
 * NoMatch:
 *   No match against any rules.
 *
 * NotFound:
 *   Register could not be resolved.
 *
 * BadRegister:
 *   The register is bad.
 *   Register name starts with 'B' (probably a battery register)
 */
enum RegisterScore {
  RegisterScore_FullMatch,
  RegisterScore_PartialMatch,
  RegisterScore_MinimalMatch,
  RegisterScore_NoMatch,
  RegisterScore_NotFound,
  RegisterScore_BadRegister
};

/*
 * Indicates whether a register is correctly aligned for access or if it is
 * misaligned.
 *
 * OK:
 *   The register is correctly aligned
 *
 * Misaligned:
 *   The register is not correctly aligned
 */
enum RegisterAlignment {
  RegisterAlignment_OK,
  RegisterAlignment_Misaligned
};

/*
 * Indicates how well an ACPI method name matches the firmware.
 *
 * Found:
 *   Method could be found.
 *
 * NotFound:
 *   Method could not be found.
 */
enum MethodScore {
  MethodScore_Found,
  MethodScore_NotFound,
};

/*
 * Stores the rating of an EC register.
 *
 * register_offset:
 *   Original byte offset of the register.
 *
 * register_type:
 *   Type of the register. Indicates whether register is from FanConfiguration
 *   or RegisterWriteConfiguration.
 *
 * register_score:
 *   Score indicating how well the register matches the firwmare.
 *
 * register_alignment:
 *   Indicates whether a register is correctly aligned for access or if it is
 *   misaligned.
 *
 * acpi_register:
 *   Looked up register information.
 *   If `register_score` is `RegisterScore_NotFound` this field is NULL.
 */
struct ConfigRating_RegisterRating {
  int register_offset;
  enum RegisterType register_type;
  enum RegisterScore register_score;
  enum RegisterAlignment register_alignment;
  AcpiRegister* acpi_register;
};
typedef struct ConfigRating_RegisterRating ConfigRating_RegisterRating;
declare_array_of(ConfigRating_RegisterRating);

/*
 * Stores the rating of an ACPI method.
 *
 * method_call:
 *   Stores the original method call string from the configuration file.
 *
 * method_score:
 *   Score indicating how well the method matches the firmware.
 *
 * acpi_method:
 *   Looked up method information.
 *   If `method_score` is `MethodScore_NotFound` this field is NULL.
 */
struct ConfigRating_MethodRating {
  char* method_call;
  enum MethodScore method_score;
  AcpiMethod* acpi_method;
};
typedef struct ConfigRating_MethodRating ConfigRating_MethodRating;
declare_array_of(ConfigRating_MethodRating);

/*
 * Stores the overall rating of a configuration file.
 *
 * score:
 *   Stores the score (0.0 to 10.0) of the model configuration.
 *
 * priority:
 *   Stores the priority.
 *   The priority further refines the configuration rating when multiple
 *   configurations have the same score.
 *   Higher priority is given to configurations that rely on ACPI methods
 *   or have a larger number of matched registers.
 *
 * register_ratings:
 *   Stores the register ratings.
 *
 * method_ratings:
 *   Stores the method ratings.
 */
struct ConfigRating_Rating {
  float score;
  int priority;
  array_of(ConfigRating_RegisterRating) register_ratings;
  array_of(ConfigRating_MethodRating) method_ratings;
};
typedef struct ConfigRating_Rating ConfigRating_Rating;
declare_array_of(ConfigRating_Rating);

Error ConfigRatingRules_FromJson(ConfigRatingRules*, const char*);
void  ConfigRatingRules_Free(ConfigRatingRules*);

Error ConfigRating_Init(ConfigRating*, const char*, const char*);
void  ConfigRating_Free(ConfigRating*);

void  ConfigRating_RateModelConfig(ConfigRating*, ModelConfig*, ConfigRating_Rating*);
void  ConfigRating_RatingPrint(ConfigRating_Rating*);

void  ConfigRating_RatingFree(ConfigRating_Rating*);
void  ConfigRating_MethodRatingFree(ConfigRating_MethodRating*);
void  ConfigRating_RegisterRatingFree(ConfigRating_RegisterRating*);

#endif
