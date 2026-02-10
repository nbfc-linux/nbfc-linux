#ifndef NBFC_CONFIG_RATING_H_
#define NBFC_CONFIG_RATING_H_

#include "macros.h"
#include "model_config.h"
#include "acpi_analysis.h"
#include "config_rating_rules.h"
#include "nxjson.h"

/*
 * Main struct for rating.
 *
 * Contains ACPI information and the rating rules.
 *
 * acpi_info:
 *   Contains ACPI information (registers, methods, operation regions).
 *
 * rules:
 *   Contains the rules for rating configurations.
 */
struct ConfigRating {
  AcpiInfo acpi_info;
  ConfigRatingRules rules;
};
typedef struct ConfigRating ConfigRating;
declare_array_of(ConfigRating);

/*
 * Specifies the category of a register, e.g. whether it is a fan register or
 * a miscellaneous register used for register-write configurations.
 */
enum NBFC_PACKED_ENUM RegisterType {
  RegisterType_FanReadRegister,
  RegisterType_FanWriteRegister,
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
enum NBFC_PACKED_ENUM RegisterScore {
  RegisterScore_FullMatch,
  RegisterScore_PartialMatch,
  RegisterScore_MinimalMatch,
  RegisterScore_NoMatch,
  RegisterScore_NotFound,
  RegisterScore_BadRegister
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
enum NBFC_PACKED_ENUM MethodScore {
  MethodScore_Found,
  MethodScore_NotFound,
};

/*
 * Stores the rating of an EC register.
 *
 * offset:
 *   Original byte offset of the register.
 *
 * type:
 *   Type of the register. Indicates whether register is from FanConfiguration
 *   or RegisterWriteConfiguration.
 *
 * score:
 *   Score indicating how well the register matches the firwmare.
 *
 * info:
 *   Looked up register information.
 *   If `score` is `RegisterScore_NotFound` this field is NULL.
 */
struct ConfigRating_RegisterRating {
  int offset;
  enum RegisterType type;
  enum RegisterScore score;
  AcpiRegister* info;
};
typedef struct ConfigRating_RegisterRating ConfigRating_RegisterRating;
declare_array_of(ConfigRating_RegisterRating);

/*
 * Stores the rating of an ACPI method.
 *
 * call:
 *   Stores the original method call string from the configuration file.
 *
 * score:
 *   Score indicating how well the method matches the firmware.
 *
 * info:
 *   Looked up method information.
 *   If `method_score` is `MethodScore_NotFound` this field is NULL.
 */
struct ConfigRating_MethodRating {
  char* call;
  enum MethodScore score;
  AcpiMethod* info;
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

const char* RegisterType_ToStr(enum RegisterType);
const char* RegisterScore_ToStr(enum RegisterScore);
const char* MethodScore_ToStr(enum MethodScore);
nx_json* ConfigRating_ToJson(ConfigRating_Rating*, const char*, nx_json*);

#endif
