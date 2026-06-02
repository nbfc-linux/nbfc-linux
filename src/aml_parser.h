#ifndef NBFC_AML_PARSER_H_
#define NBFC_AML_PARSER_H_

#include "aml_lexer.h"

#define AML_PARSER_STACK_SIZE        1024
#define AML_PARSER_MAX_NAMESPACE_LEN 128

/*
 * A stack entry.
 *
 * This is used to keep track of opening parentheses/braces/brackets.
 * It also holds a possible namespace name.
 */
typedef struct AML_StackEntry AML_StackEntry;
struct AML_StackEntry {
  AML_TokenType type;
  AML_Token name;
  bool has_name;
};

/*
 * Stack of opening parentheses/braces/brackets.
 */
typedef struct AML_Stack AML_Stack;
struct AML_Stack {
  AML_StackEntry stack[AML_PARSER_STACK_SIZE];
  size_t size;
};

/*
 * Structure used for parsing.
 */
typedef struct AML_Parser AML_Parser;
struct AML_Parser {
  array_of(AML_Token) tokens;
  array_size_t pos;
  AML_Stack stack;
  const char* error;
  const char* error2;
};

/*
 * An extracted method
 */
typedef struct AML_Method AML_Method;
struct AML_Method {
  char name[128];
  array_of(AML_Token) tokens;
};

declare_array_of(AML_Method);

void AML_Parser_Init(AML_Parser*, array_of(AML_Token));
bool AML_Parser_GetRealToken(AML_Parser*, AML_Token*);
bool AML_Parser_EatToken(AML_Parser*, AML_TokenType);
bool AML_Parser_ParseToken(AML_Parser*, AML_TokenType type, AML_Token*);

#define AML_Parser_EatComma(P)            AML_Parser_EatToken(P, AML_TOK_OP_Comma)
#define AML_Parser_EatParenOpen(P)        AML_Parser_EatToken(P, AML_TOK_OP_ParenOpen)
#define AML_Parser_EatParenClose(P)       AML_Parser_EatToken(P, AML_TOK_OP_ParenClose)
#define AML_Parser_EatBraceOpen(P)        AML_Parser_EatToken(P, AML_TOK_OP_BraceOpen)
#define AML_Parser_EatBraceClose(P)       AML_Parser_EatToken(P, AML_TOK_OP_BraceClose)

#define AML_Parser_ParseKeyword(P, T)     AML_Parser_ParseToken(P, AML_TOK_Keyword, T)
#define AML_Parser_ParseIdentifier(P, T)  AML_Parser_ParseToken(P, AML_TOK_Identifier, T)
#define AML_Parser_ParseInteger(P, T)     AML_Parser_ParseToken(P, AML_TOK_Integer, T)

Error AML_Parser_GetError(const AML_Parser*);
Error AML_Parser_ExtractMethods(AML_Parser*, array_of(AML_Method)*);

#endif
