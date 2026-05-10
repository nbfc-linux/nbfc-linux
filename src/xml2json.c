#include "xml2json.h"

#include "memory.h"
#include "nxjson_utils.h"
#include "parse_double.h"
#include "parse_number.h"

#include <float.h>  // FLT_MAX
#include <string.h> // strcmp

/**
 * Read a string from an XML node and store it in the parent's JSON child.
 */
static Error Xml2Json_ParseString(xmlNode* node, const char* key, nx_json* parent) {
  xmlChar* content = xmlNodeGetContent(node);
  if (! content)
    return err_string("xmlNodeGetContent() returned NULL");

  create_json_string(key, parent, Mem_Strdup((const char*) content));
  xmlFree(content);
  return err_success();
}

/**
 * Parse an integer from an XML node and store it in the parent's JSON child.
 */
static Error Xml2Json_ParseInt(xmlNode* node, const char* key, nx_json* parent) {
  xmlChar* content = xmlNodeGetContent(node);
  if (! content)
    return err_string("xmlNodeGetContent() returned NULL");

  const char* err;
  int64_t val = parse_number((const char*) content, INT64_MIN, INT64_MAX, &err);
  xmlFree(content);
  if (err)
    return err_string(err);

  create_json_integer(key, parent, val);
  return err_success();
}

/**
 * Parse a float from an XML node and store it in the parent's JSON child.
 */
static Error Xml2Json_ParseFloat(xmlNode* node, const char* key, nx_json* parent) {
  xmlChar* content = xmlNodeGetContent(node);
  if (! content)
    return err_string("xmlNodeGetContent() return NULL");

  const char* err;
  double val = parse_double((const char*) content, -FLT_MAX, FLT_MAX, &err);
  xmlFree(content);
  if (err)
    return err_string(err);

  create_json_double(key, parent, val);
  return err_success();
}

/**
 * Parse a boolean from an XML node and store it in the parent's JSON child.
 */
static Error Xml2Json_ParseBool(xmlNode* node, const char* key, nx_json* parent) {
  xmlChar* content = xmlNodeGetContent(node);
  if (! content)
    return err_string("xmlNodeGetContent() returned NULL");

  int val = 2;
  if (! strcmp((const char*) content, "true")) {
    val = 1;
  }
  else if (! strcmp((const char*) content, "false")) {
    val = 0;
  }

  xmlFree(content);
  if (val == 2)
    return err_string("Not a bool");

  create_json_bool(key, parent, !!val);
  return err_success();
}

#define XML2JSON_PARSE_BEGIN()                                                \
  Error e = err_success();                                                    \
  for (xmlNode *node = root->children; node; node = node->next) {             \
    if (node->type == XML_ELEMENT_NODE) {                                     \
      if (false);                                                             \

#define XML2JSON_PARSE_END()                                                  \
      else {                                                                  \
        e = err_string("Unkown field");                                       \
      }                                                                       \
                                                                              \
      if (e)                                                                  \
        return err_chain_string(e, (const char*) node->name);                 \
    }                                                                         \
  }

#define ON(KEY) \
  else if (! strcmp((const char*) node->name, KEY))

#define PARSE_INT(NODE, KEY, PARENT) \
  e = Xml2Json_ParseInt(NODE, KEY, PARENT)

#define PARSE_FLT(NODE, KEY, PARENT) \
  e = Xml2Json_ParseFloat(NODE, KEY, PARENT)

#define PARSE_STR(NODE, KEY, PARENT) \
  e = Xml2Json_ParseString(NODE, KEY, PARENT)

#define PARSE_BOL(NODE, KEY, PARENT) \
  e = Xml2Json_ParseBool(NODE, KEY, PARENT)

static Error Xml2Json_ParseTemperatureThreshold(xmlNode* root, nx_json* parent) {
  nx_json* obj = create_json_object(NULL, parent);
  XML2JSON_PARSE_BEGIN()
  ON("UpThreshold")           { PARSE_INT(node, "UpThreshold",        obj); }
  ON("DownThreshold")         { PARSE_INT(node, "DownThreshold",      obj); }
  ON("FanSpeed")              { PARSE_FLT(node, "FanSpeed",           obj); }
  XML2JSON_PARSE_END()
  return err_success();
}

static Error Xml2Json_ParseTemperatureThresholds(xmlNode* root, nx_json* parent) {
  nx_json* array = create_json_array("TemperatureThresholds", parent);

  for (xmlNode *node = root->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE) {
      Error e = Xml2Json_ParseTemperatureThreshold(node, array);
      if (e)
        return e;
    }

  return err_success();
}

static Error Xml2Json_ParseFanSpeedPercentageOverride(xmlNode* root, nx_json* parent) {
  nx_json* obj = create_json_object(NULL, parent);
  XML2JSON_PARSE_BEGIN()
  ON("FanSpeedPercentage")    { PARSE_FLT(node, "FanSpeedPercentage", obj); }
  ON("FanSpeedValue")         { PARSE_INT(node, "FanSpeedValue",      obj); }
  ON("TargetOperation")       { PARSE_STR(node, "TargetOperation",    obj); }
  XML2JSON_PARSE_END()
  return err_success();
}

static Error Xml2Json_ParseFanSpeedPercentageOverrides(xmlNode* root, nx_json* parent) {
  nx_json* array = create_json_array("FanSpeedPercentageOverrides", parent);

  for (xmlNode *node = root->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE) {
      Error e = Xml2Json_ParseFanSpeedPercentageOverride(node, array);
      if (e)
        return e;
    }

  return err_success();
}

static Error Xml2Json_ParseRegisterWriteConfiguration(xmlNode* root, nx_json* parent) {
  nx_json* obj = create_json_object(NULL, parent);
  XML2JSON_PARSE_BEGIN()
  ON("WriteMode")             { PARSE_STR(node, "WriteMode",          obj); }
  ON("WriteOccasion")         { PARSE_STR(node, "WriteOccasion",      obj); }
  ON("Register")              { PARSE_INT(node, "Register",           obj); }
  ON("Value")                 { PARSE_INT(node, "Value",              obj); }
  ON("ResetRequired")         { PARSE_BOL(node, "ResetRequired",      obj); }
  ON("ResetValue")            { PARSE_INT(node, "ResetValue",         obj); }
  ON("ResetWriteMode")        { PARSE_STR(node, "ResetWriteMode",     obj); }
  ON("Description")           { PARSE_STR(node, "Description",        obj); }
  XML2JSON_PARSE_END()
  return err_success();
}

static Error Xml2Json_ParseRegisterWriteConfigurations(xmlNode* root, nx_json* parent) {
  nx_json* array = create_json_array("RegisterWriteConfigurations", parent);

  for (xmlNode *node = root->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE) {
      Error e = Xml2Json_ParseRegisterWriteConfiguration(node, array);
      if (e)
        return e;
    }

  return err_success();
}

static Error Xml2Json_ParseFanConfiguration(xmlNode* root, nx_json* parent) {
  nx_json* obj = create_json_object(NULL, parent);
  XML2JSON_PARSE_BEGIN()
  ON("FanDisplayName")        { PARSE_STR(node, "FanDisplayName",     obj); }
  ON("ReadRegister")          { PARSE_INT(node, "ReadRegister",       obj); }
  ON("WriteRegister")         { PARSE_INT(node, "WriteRegister",      obj); }
  ON("MinSpeedValue")         { PARSE_INT(node, "MinSpeedValue",      obj); }
  ON("MaxSpeedValue")         { PARSE_INT(node, "MaxSpeedValue",      obj); }
  ON("MinSpeedValueRead")     { PARSE_INT(node, "MinSpeedValueRead",  obj); }
  ON("MaxSpeedValueRead")     { PARSE_INT(node, "MaxSpeedValueRead",  obj); }
  ON("IndependentReadMinMaxValues") { PARSE_BOL(node, "IndependentReadMinMaxValues", obj); }
  ON("ResetRequired")         { PARSE_BOL(node, "ResetRequired",      obj); }
  ON("FanSpeedResetValue")    { PARSE_INT(node, "FanSpeedResetValue", obj); }
  ON("TemperatureThresholds") { e = Xml2Json_ParseTemperatureThresholds(node, obj); }
  ON("FanSpeedPercentageOverrides") { e = Xml2Json_ParseFanSpeedPercentageOverrides(node, obj); }
  XML2JSON_PARSE_END()
  return err_success();
}

static Error Xml2Json_ParseFanConfigurations(xmlNode* root, nx_json* parent) {
  nx_json* array = create_json_array("FanConfigurations", parent);

  for (xmlNode *node = root->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE) {
      Error e = Xml2Json_ParseFanConfiguration(node, array);
      if (e)
        return e;
    }

  return err_success();
}

static Error Xml2Json_ParseModelConfig(xmlNode* root, nx_json* parent) {
  nx_json* obj = parent;
  XML2JSON_PARSE_BEGIN()
  ON("NotebookModel")         { PARSE_STR(node, "NotebookModel",       obj); }
  ON("Author")                { PARSE_STR(node, "Author",              obj); }
  ON("EcPollInterval")        { PARSE_INT(node, "EcPollInterval",      obj); }
  ON("CriticalTemperature")   { PARSE_INT(node, "CriticalTemperature", obj); }
  ON("CriticalTemperatureOffset") { PARSE_INT(node, "CriticalTemperatureOffset", obj); }
  ON("ReadWriteWords")        { PARSE_BOL(node, "ReadWriteWords",      obj); }
  ON("FanConfigurations")     { e = Xml2Json_ParseFanConfigurations(node, obj); }
  ON("RegisterWriteConfigurations") { e = Xml2Json_ParseRegisterWriteConfigurations(node, obj); }
  XML2JSON_PARSE_END()
  return err_success();
}

#undef XML2JSON_PARSE_BEGIN
#undef XML2JSON_PARSE_END
#undef ON
#undef PARSE_INT
#undef PARSE_FLT
#undef PARSE_STR
#undef PARSE_BOL

/**
 * Convert a NBFC configuration stored in an XML DOM to a JSON structure.
 */
Error Xml2Json_Convert(xmlNode* root, nx_json** out) {
  nx_json js_root = {0};
  nx_json* object = create_json_object(NULL, &js_root);
  Error e = Xml2Json_ParseModelConfig(root, object);
  *out = object;
  return e;
}

/**
 * Convert a NBFC configuration stored in an XML file to a JSON structure.
 */
Error Xml2Json_ConvertFile(const char* file, nx_json** out) {
  Error e;
  xmlDoc* doc;
  xmlNode* root;

  doc = xmlReadFile(file, NULL, 0);
  if (doc == NULL)
    return err_string("Failed to parse XML file");

  root = xmlDocGetRootElement(doc);
  if (! strcmp((const char*) root->name, "FanControlConfigV2"))
    e = Xml2Json_Convert(root, out);
  else
    e = err_string("XML root node is not \"FanControlConfigV2\"");

  xmlFreeDoc(doc);
  xmlCleanupParser();
  return e;
}
