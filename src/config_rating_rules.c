#include "config_rating_rules.h"
#include "nxjson_utils.h"
#include "memory.h"

#include <stdio.h>  // printf
#include <string.h> // memcpy, memset, strcmp, strlen

static enum RegisterRuleFanMode RegisterRuleFanMode_FromString(const char*);

/*
 * Parse a register name.
 */
static Error RegisterName_FromJson(char* out, const nx_json* json) {
  if (json->type != NX_JSON_STRING)
    return err_string("Not a string");

  const size_t len = strlen(json->val.text);
  if (len == 0 || len > sizeof(AcpiRegisterName) - 1)
    return err_stringf("\"%s\": Invalid length", json->val.text);

  memcpy(out, json->val.text, len + 1);
  return err_success();
}

/*
 * Parse a register rule priority (0 - 100).
 */
static Error RegisterRulePriority_FromJson(uint8_t* out, const nx_json* json) {
  if (json->type != NX_JSON_INTEGER)
    return err_string("Not an integer");

  if (json->val.i < 0 || json->val.i > 100)
    return err_stringf("Value not in range (%d - %d): %ld", 0, 100, json->val.i);

  *out = (uint8_t) json->val.i;
  return err_success();
}

/*
 * Parse an array of register names.
 *
 * Example input:
 *   ["CFAN", "PFAN", "XFAN"]
 */
static Error ArrayOfRegisterNames_FromJson(array_of(AcpiRegisterName)* out, const nx_json* json) {
  Error e;

  if (json->type != NX_JSON_ARRAY)
    return err_string("Not an array");

  out->size = 0;
  array_realloc(AcpiRegisterName, *out, json->val.children.length);

  nx_json_for_each(child, json) {
    e = RegisterName_FromJson(out->data[out->size], child);
    if (e)
      return err_chain_stringf(e, "[%zd]", out->size);

    out->size++;
  }

  return err_success();
}

/*
 * Parse a register rule.
 *
 * Example input:
 *   {
 *     "Name": "XFAN",
 *     "Mode": "rw",
 *     "ReadPriority": 100,
 *     "WritePriority": 100,
 *     "Notice": "Foo bar"
 *   }
 */
static Error RegisterRule_FromJson(RegisterRule* out, const nx_json* json) {
  Error e;

  if (json->type != NX_JSON_OBJECT)
    return err_string("Not an object");

  out->Mode = RegisterRuleFanMode_None;
  out->Name[0] = '\0';
  out->ReadPriority = 0;
  out->WritePriority = 0;
  out->Notice = NULL;

  nx_json_for_each(child, json) {
    e = err_success();

    if (! strcmp(child->key, "Name")) {
      e = RegisterName_FromJson(out->Name, child);
    }
    else if (! strcmp(child->key, "Mode")) {
      if (child->type != NX_JSON_STRING)
        e = err_string("Not a string");
      else {
        out->Mode = RegisterRuleFanMode_FromString(child->val.text);
        if (out->Mode == RegisterRuleFanMode_None)
          e = err_string("Invalid mode");
      }
    }
    else if (! strcmp(child->key, "ReadPriority")) {
      e = RegisterRulePriority_FromJson(&out->ReadPriority, child);
    }
    else if (! strcmp(child->key, "WritePriority")) {
      e = RegisterRulePriority_FromJson(&out->WritePriority, child);
    }
    else if (! strcmp(child->key, "Notice")) {
      if (child->type != NX_JSON_STRING)
        e = err_string("Not a string");
      else
        out->Notice = Mem_Strdup(child->val.text);
    }
    else {
      e = err_string("Unknown key");
    }

    if (e)
      return err_chain_string(e, child->key);
  }

  if (! out->Name[0])
    return err_stringf("Missing key: %s", "Name");

  if (out->Mode == RegisterRuleFanMode_None)
    return err_stringf("Missing key: %s", "Mode");

  return err_success();
}

/*
 * Parse an array of register rules.
 *
 * Example input:
 *   [ {"XFAN": "rw"}, {"FRDC": "r"} ]
 */
static Error ArrayOfRegisterRules_FromJson(array_of(RegisterRule)* out, const nx_json* json) {
  Error e;

  if (json->type != NX_JSON_ARRAY)
    return err_string("Not an array");

  out->size = 0;
  array_realloc(RegisterRule, *out, json->val.children.length);

  nx_json_for_each(object, json) {
    e = RegisterRule_FromJson(&out->data[out->size], object);
    if (e)
      return err_chain_stringf(e, "[%zd]", out->size);

    out->size++;
  }

  return err_success();
}

/*
 * Parses the JSON string and populates the `ConfigRatingRules` structure.
 *
 * Returns an error if:
 *  - The JSON itself is invalid.
 *  - The JSON structure is invalid (invalid fields, invalid types).
 */
Error ConfigRatingRules_FromJson(ConfigRatingRules* rules, const char* rules_json) {
  Error e = err_success();
  char* rules_json_temp = NULL;
  const nx_json* root = NULL;

  memset(rules, 0, sizeof(*rules));
  rules_json_temp = Mem_Strdup(rules_json);
  root = nx_json_parse_utf8(rules_json_temp);

  if (! root) {
    e = err_nxjson(NULL);
    goto end;
  }

  if (root->type != NX_JSON_OBJECT) {
    e = err_string("Not an object");
    goto end;
  }

  nx_json_for_each(child, root) {
    if (! strcmp(child->key, "FanRegisterFullMatch"))
      e = ArrayOfRegisterRules_FromJson(&rules->FanRegisterFullMatch, child);
    else if (! strcmp(child->key, "FanRegisterPartialMatch"))
      e = ArrayOfRegisterNames_FromJson(&rules->FanRegisterPartialMatch, child);
    else if (! strcmp(child->key, "RegisterWriteFullMatch"))
      e = ArrayOfRegisterNames_FromJson(&rules->RegisterWriteFullMatch, child);
    else if (! strcmp(child->key, "RegisterWritePartialMatch"))
      e = ArrayOfRegisterNames_FromJson(&rules->RegisterWritePartialMatch, child);
    else if (! strcmp(child->key, "BadRegisterFullMatch"))
      e = ArrayOfRegisterNames_FromJson(&rules->BadRegisterFullMatch, child);
    else if (! strcmp(child->key, "BadRegisterPartialMatch"))
      e = ArrayOfRegisterNames_FromJson(&rules->BadRegisterPartialMatch, child);
    else
      e = err_string("Unknown key");

    if (e) {
      e = err_chain_string(e, child->key);
      goto end;
    }
  }

end:
  nx_json_free(root);
  Mem_Free(rules_json_temp);

  if (e)
    ConfigRatingRules_Free(rules);

  return e;
}

static enum RegisterRuleFanMode RegisterRuleFanMode_FromString(const char* s) {
  if (! strcmp(s, "r")) return RegisterRuleFanMode_Read;
  if (! strcmp(s, "w")) return RegisterRuleFanMode_Write;
  if (! strcmp(s, "rw")) return RegisterRuleFanMode_Read | RegisterRuleFanMode_Write;
  if (! strcmp(s, "wr")) return RegisterRuleFanMode_Read | RegisterRuleFanMode_Write;
  return RegisterRuleFanMode_None;
}

static const char* RegisterRuleFanMode_ToStr(enum RegisterRuleFanMode mode) {
  switch (mode) {
    case RegisterRuleFanMode_Read:  return "r";
    case RegisterRuleFanMode_Write: return "w";
    default: return "rw";
  }
}

static void ConfigRatingRules_RegisterRulesToJson(
  const array_of(RegisterRule)* rules,
  const char* key,
  nx_json* parent)
{
  nx_json* array = create_json_array(key, parent);

  for_each_array(RegisterRule*, rule, *rules) {
    nx_json* object = create_json_object(NULL, array);
    create_json_string("Name", object, rule->Name);
    create_json_string("Mode", object, RegisterRuleFanMode_ToStr(rule->Mode));

    if (rule->ReadPriority)
      create_json_integer("ReadPriority", object, rule->ReadPriority);

    if (rule->WritePriority)
      create_json_integer("WritePriority", object, rule->WritePriority);

    if (rule->Notice)
      create_json_string("Notice", object, rule->Notice);
  }
}

static void ConfigRatingRules_RegisterNamesToJson(
  const array_of(AcpiRegisterName)* names,
  const char* key,
  nx_json* parent)
{
  nx_json* array = create_json_array(key, parent);

  for_each_array(AcpiRegisterName*, name, *names) {
    create_json_string(NULL, array, *name);
  }
}

nx_json* ConfigRatingRules_ToJson(const ConfigRatingRules* rules) {
  nx_json root = {0};
  nx_json* object = create_json_object(NULL, &root);
  ConfigRatingRules_RegisterRulesToJson(&rules->FanRegisterFullMatch, "FanRegisterFullMatch", object);
  ConfigRatingRules_RegisterNamesToJson(&rules->FanRegisterPartialMatch, "FanRegisterPartialMatch", object);
  ConfigRatingRules_RegisterNamesToJson(&rules->RegisterWriteFullMatch, "RegisterWriteFullMatch", object);
  ConfigRatingRules_RegisterNamesToJson(&rules->RegisterWritePartialMatch, "RegisterWritePartialMatch", object);
  ConfigRatingRules_RegisterNamesToJson(&rules->BadRegisterFullMatch, "BadRegisterFullMatch", object);
  ConfigRatingRules_RegisterNamesToJson(&rules->BadRegisterPartialMatch, "BadRegisterPartialMatch", object);
  return object;
}

void ConfigRatingRules_Print(const ConfigRatingRules* rules) {
  printf("FanRegisterFullMatch:\n");
  for_each_array(RegisterRule*, rule, rules->FanRegisterFullMatch)
    printf("\t%s (%s)\n", rule->Name, RegisterRuleFanMode_ToStr(rule->Mode));

  printf("\nFanRegisterPartialMatch:\n");
  for_each_array(AcpiRegisterName*, name, rules->FanRegisterPartialMatch)
    printf("\t%s\n", *name);

  printf("\nRegisterWriteFullMatch:\n");
  for_each_array(AcpiRegisterName*, name, rules->RegisterWriteFullMatch)
    printf("\t%s\n", *name);

  printf("\nRegisterWritePartialMatch:\n");
  for_each_array(AcpiRegisterName*, name, rules->RegisterWritePartialMatch)
    printf("\t%s\n", *name);

  printf("\nBadRegisterFullMatch:\n");
  for_each_array(AcpiRegisterName*, name, rules->BadRegisterFullMatch)
    printf("\t%s\n", *name);

  printf("\nBadRegisterPartialMatch:\n");
  for_each_array(AcpiRegisterName*, name, rules->BadRegisterPartialMatch)
    printf("\t%s\n", *name);
}

void ConfigRatingRules_Free(ConfigRatingRules* rules) {
  for_each_array(RegisterRule*, rule, rules->FanRegisterFullMatch)
    Mem_Free(rule->Notice);

  Mem_Free(rules->FanRegisterFullMatch.data);
  Mem_Free(rules->FanRegisterPartialMatch.data);
  Mem_Free(rules->RegisterWriteFullMatch.data);
  Mem_Free(rules->RegisterWritePartialMatch.data);
  Mem_Free(rules->BadRegisterFullMatch.data);
  Mem_Free(rules->BadRegisterPartialMatch.data);
  memset(rules, 0, sizeof(*rules));
}
