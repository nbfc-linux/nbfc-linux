#ifndef NBFC_XML2JSON_H_
#define NBFC_XML2JSON_H_

#include "error.h"
#include "nxjson.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

Error Xml2Json_Convert(xmlNode*, nx_json**);
Error Xml2Json_ConvertFile(const char*, nx_json**);

#endif
