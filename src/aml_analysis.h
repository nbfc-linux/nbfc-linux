#ifndef NBFC_AML_ANALYSIS_H_
#define NBFC_AML_ANALYSIS_H_

#include "error.h"
#include "macros.h"
#include "aml_parser.h"

/*
 * Holds the source code and the lexed tokens of an AML file.
 *
 * Since the tokens reference the source buffer, both are kept
 * together as a pair.
 */
typedef struct AML_LexedSource AML_LexedSource;
struct AML_LexedSource {
  char* source;
  array_of(AML_Token) tokens;
};

declare_array_of(AML_LexedSource);

/*
 * Holds the parsed methods and the sources.
 */
typedef struct AML_Analysis AML_Analysis;
struct AML_Analysis {
  array_of(AML_Method) methods;
  array_of(AML_LexedSource) sources;
};

Error AML_Analysis_Init(AML_Analysis*, array_of(str)*);
void  AML_Analysis_Free(AML_Analysis*);
Error AML_Analysis_MatchFingerprint(AML_Analysis*, const char*, bool*);

#endif
