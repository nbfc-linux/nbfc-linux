#include "config_rating_rules.h"
#include "nxjson_utils.h"
#include "memory.h"

#include <stdio.h>  // snprintf
#include <string.h> // memset, strcmp, strlen

/*
 * Parse an array of register names.
 *
 * Example input:
 *   ["CFAN", "PFAN", "XFAN"]
 */
static Error ParseRegisterNamesArray(array_of(AcpiRegisterName)* out, const nx_json* json) {
  Error e;

  if (json->type != NX_JSON_ARRAY) {
    e = err_stringf("%s: Not a JSON array", json->key);
    return e;
  }

  out->size = 0;
  out->data = Mem_Realloc(out->data,
    json->val.children.length * sizeof(AcpiRegisterName));

  nx_json_for_each(child, json) {
    if (child->type != NX_JSON_STRING) {
      e = err_stringf("%s[%zd]: Not a string", json->key, out->size);
      return e;
    }

    const size_t slen = strlen(child->val.text);
    if (slen == 0 || slen > sizeof(AcpiRegisterName) - 1) {
      e = err_stringf("%s[%zd]: \"%s\": Invalid length",
        json->key, out->size, child->val.text);
      return e;
    }

    snprintf(out->data[out->size], sizeof(AcpiRegisterName), "%s", child->val.text);

    out->size++;
  }

  return err_success();
}

/*
 * Parse a register rule.
 *
 * Example input:
 *   {"Name": "XFAN", "Mode": "rw"}
 */
static Error ParseRegisterRule(RegisterRule* out, const nx_json* json) {
  Error e;

  if (json->type != NX_JSON_OBJECT) {
    e = err_string("Not an object");
    return e;
  }

  out->Mode = RegisterRuleFanMode_None;
  out->Name[0] = '\0';

  nx_json_for_each(child, json) {
    if (! strcmp(child->key, "Name")) {
      if (child->type != NX_JSON_STRING) {
        e = err_string("Name: Not a string");
        return e;
      }

      const size_t slen = strlen(child->val.text);
      if (slen == 0 || slen > sizeof(AcpiRegisterName) - 1) {
        e = err_stringf("Name: \"%s\": Invalid length", child->val.text);
        return e;
      }

      snprintf(out->Name, sizeof(AcpiRegisterName), "%s", child->val.text);
    }
    else if (! strcmp(child->key, "Mode")) {
      if (child->type != NX_JSON_STRING) {
        e = err_string("Mode: Not a string");
        return e;
      }

      const size_t slen = strlen(child->val.text);
      if (slen == 0 || slen > 2) {
        e = err_stringf("Mode: \"%s\": Invalid length", child->val.text);
        return e;
      }

      for (const char* s = child->val.text; *s; ++s) {
        if (*s == 'r')
          out->Mode |= RegisterRuleFanMode_Read;
        else if (*s == 'w')
          out->Mode |= RegisterRuleFanMode_Write;
        else {
          e = err_stringf("Mode: Invalid char: '%c'", *s);
          return e;
        }
      }
    }
    else {
      e = err_stringf("Unknown key: %s", child->key);
      return e;
    }
  }

  if (out->Mode == RegisterRuleFanMode_None) {
    e = err_string("Missing key: Mode");
    return e;
  }

  if (! out->Name[0]) {
    e = err_string("Missing key: Name");
    return e;
  }

  return err_success();
}

/*
 * Parse an array of register rules.
 *
 * Example input:
 *   [ {"XFAN": "rw"}, {"FRDC": "r"} ]
 */
static Error ParseRegisterRuleArray(array_of(RegisterRule)* out, const nx_json* json) {
  Error e;

  if (json->type != NX_JSON_ARRAY) {
    e = err_stringf("%s: Not a JSON array", json->key);
    return e;
  }

  out->size = 0;
  out->data = Mem_Realloc(out->data,
    json->val.children.length * sizeof(RegisterRule));

  nx_json_for_each(object, json) {
    e = ParseRegisterRule(&out->data[out->size], object);
    if (e) {
      e = err_chain_stringf(e, "%s[%zd]", json->key, out->size);
      return e;
    }
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
    e = err_string("Not a JSON object");
    goto end;
  }

  nx_json_for_each(child, root) {
    if (! strcmp(child->key, "FanRegisterFullMatch")) {
      e = ParseRegisterRuleArray(&rules->FanRegisterFullMatch, child);
      if (e)
        goto end;
    }
    else if (! strcmp(child->key, "FanRegisterPartialMatch")) {
      e = ParseRegisterNamesArray(&rules->FanRegisterPartialMatch, child);
      if (e)
        goto end;
    }
    else if (! strcmp(child->key, "RegisterWriteFullMatch")) {
      e = ParseRegisterNamesArray(&rules->RegisterWriteFullMatch, child);
      if (e)
        goto end;
    }
    else if (! strcmp(child->key, "RegisterWritePartialMatch")) {
      e = ParseRegisterNamesArray(&rules->RegisterWritePartialMatch, child);
      if (e)
        goto end;
    }
    else if (! strcmp(child->key, "BadRegisterFullMatch")) {
      e = ParseRegisterNamesArray(&rules->BadRegisterFullMatch, child);
      if (e)
        goto end;
    }
    else if (! strcmp(child->key, "BadRegisterPartialMatch")) {
      e = ParseRegisterNamesArray(&rules->BadRegisterPartialMatch, child);
      if (e)
        goto end;
    }
    else {
      e = err_stringf("Unknown key: %s", child->key);
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

void ConfigRatingRules_Free(ConfigRatingRules* rules) {
  Mem_Free(rules->FanRegisterFullMatch.data);
  Mem_Free(rules->FanRegisterPartialMatch.data);
  Mem_Free(rules->RegisterWriteFullMatch.data);
  Mem_Free(rules->RegisterWritePartialMatch.data);
  Mem_Free(rules->BadRegisterFullMatch.data);
  Mem_Free(rules->BadRegisterPartialMatch.data);
  memset(rules, 0, sizeof(*rules));
}
