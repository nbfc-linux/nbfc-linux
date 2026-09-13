#ifndef NBFC_AML_LEXER_H_
#define NBFC_AML_LEXER_H_

#include <stddef.h> // size_t
#include <stdint.h> // uint64_t
#include <stdbool.h>

#include "error.h"
#include "macros.h"

/*
 * Available token types.
 */
typedef enum AML_TokenType AML_TokenType;
enum NBFC_PACKED_ENUM AML_TokenType {
  AML_TOK_Error,

  AML_TOK_EOF,
  AML_TOK_Whitespace,
  AML_TOK_Comment,
  AML_TOK_Integer,
  AML_TOK_String,
  AML_TOK_Keyword,
  AML_TOK_Identifier,

  AML_TOK_OP_ParenOpen,
  AML_TOK_OP_ParenClose,
  AML_TOK_OP_BraceOpen,
  AML_TOK_OP_BraceClose,
  AML_TOK_OP_BracketOpen,
  AML_TOK_OP_BracketClose,
  AML_TOK_OP_Comma,
  AML_TOK_OP_LSH_Assign,
  AML_TOK_OP_RSH_Assign,
  AML_TOK_OP_LSH,
  AML_TOK_OP_RSH,
  AML_TOK_OP_Equals,
  AML_TOK_OP_Unequals,
  AML_TOK_OP_GreaterEquals,
  AML_TOK_OP_LessEquals,
  AML_TOK_OP_OrAssign,
  AML_TOK_OP_AndAssign,
  AML_TOK_OP_XorAssign,
  AML_TOK_OP_AddAssign,
  AML_TOK_OP_SubAssign,
  AML_TOK_OP_MulAssign,
  AML_TOK_OP_DivAssign,
  AML_TOK_OP_ModAssign,
  AML_TOK_OP_Or,
  AML_TOK_OP_And,
  AML_TOK_OP_Not,
  AML_TOK_OP_BitOr,
  AML_TOK_OP_BitAnd,
  AML_TOK_OP_BitNot,
  AML_TOK_OP_BitXor,
  AML_TOK_OP_Add,
  AML_TOK_OP_Sub,
  AML_TOK_OP_Mul,
  AML_TOK_OP_Div,
  AML_TOK_OP_Mod,
  AML_TOK_OP_Assign,
  AML_TOK_OP_Less,
  AML_TOK_OP_Greater,

  AML_TOK_Any,
  AML_TOK_AnyInScope,
};

/*
 * Structure used for lexing.
 */
typedef struct AML_Lexer AML_Lexer;
struct AML_Lexer {
  const char* source;
  size_t pos;
  const char* error;
};

/*
 * A parsed token.
 */
typedef struct AML_Token AML_Token;
struct AML_Token {
  AML_TokenType type;
  const char* s;
  size_t len;
  uint64_t number;
};

declare_array_of(AML_Token);

void AML_Lexer_Init(AML_Lexer*, const char*);

AML_Token AML_Lexer_GetToken(AML_Lexer*);
Error AML_Lexer_GetTokens(AML_Lexer*, array_of(AML_Token)*);

bool AML_Token_StrEq(const AML_Token*, const char*, size_t);
void AML_Token_Print(const AML_Token*);
const char* AML_TokenType_ToStr(AML_TokenType);

#endif
