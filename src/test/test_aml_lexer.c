#include "../aml_lexer.c"

#include <stdio.h>
#include <stdlib.h>

static void Test_AML_Lexer_GetToken(const char* s, AML_TokenType expected) {
  AML_Lexer l;
  AML_Token t;
  AML_Lexer_Init(&l, s);

  t = AML_Lexer_GetToken(&l);

  if (t.type != expected) {
    printf("Test_AML_Lexer_GetToken(\"%s\"): Expected: %s Having: %s\n",
      AML_TokenType_ToStr(expected), AML_TokenType_ToStr(t.type));

    exit(1);
  }

  t = AML_Lexer_GetToken(&l);

  if (t.type != AML_TOK_EOF) {
    printf("Test_AML_Lexer_GetToken(\"%s\"): Expected EOF\n");

    exit(1);
  }
}

int main() {
#define T(...) Test_AML_Lexer_GetToken(__VA_ARGS__)
  T("(",        AML_TOK_OP_ParenOpen);
  T(")",        AML_TOK_OP_ParenClose);
  T("{",        AML_TOK_OP_BraceOpen);
  T("}",        AML_TOK_OP_BraceClose);
  T("[",        AML_TOK_OP_BracketOpen);
  T("]",        AML_TOK_OP_BracketClose);
  T(",",        AML_TOK_OP_Comma);
  T("<<=",      AML_TOK_OP_LSH_Assign);
  T(">>=",      AML_TOK_OP_RSH_Assign);
  T("<<",       AML_TOK_OP_LSH);
  T(">>",       AML_TOK_OP_RSH);
  T("==",       AML_TOK_OP_Equals);
  T("!=",       AML_TOK_OP_Unequals);
  T(">=",       AML_TOK_OP_GreaterEquals);
  T("<=",       AML_TOK_OP_LessEquals);
  T("|=",       AML_TOK_OP_OrAssign);
  T("&=",       AML_TOK_OP_AndAssign);
  T("^=",       AML_TOK_OP_XorAssign);
  T("+=",       AML_TOK_OP_AddAssign);
  T("-=",       AML_TOK_OP_SubAssign);
  T("*=",       AML_TOK_OP_MulAssign);
  T("/=",       AML_TOK_OP_DivAssign);
  T("%=",       AML_TOK_OP_ModAssign);
  T("||",       AML_TOK_OP_Or);
  T("&&",       AML_TOK_OP_And);
  T("!",        AML_TOK_OP_Not);
  T("|",        AML_TOK_OP_BitOr);
  T("&",        AML_TOK_OP_BitAnd);
  T("~",        AML_TOK_OP_BitNot);
  T("^",        AML_TOK_OP_BitXor);
  T("+",        AML_TOK_OP_Add);
  T("-",        AML_TOK_OP_Sub);
  T("*",        AML_TOK_OP_Mul);
  T("/",        AML_TOK_OP_Div);
  T("%",        AML_TOK_OP_Mod);
  T("=",        AML_TOK_OP_Assign);
  T("<",        AML_TOK_OP_Less);
  T(">",        AML_TOK_OP_Greater);

  T("..",       AML_TOK_AnyInScope);
  T("...",      AML_TOK_Any);

  T("//",       AML_TOK_Comment);
  T("//\n",     AML_TOK_Comment);
  T("//X",      AML_TOK_Comment);
  T("//X\n",    AML_TOK_Comment);
  T("/**/",     AML_TOK_Comment);
  T("/*X*/",    AML_TOK_Comment);

  T(" ",        AML_TOK_Whitespace);
  T("\t",       AML_TOK_Whitespace);
  T("\r",       AML_TOK_Whitespace);
  T("\n",       AML_TOK_Whitespace);
  T("  ",       AML_TOK_Whitespace);

  T("0",        AML_TOK_Integer);
  T("0x0",      AML_TOK_Integer);
  T("One",      AML_TOK_Integer);
  T("Zero",     AML_TOK_Integer);

  T("FOO",      AML_TOK_Identifier);
  T("^FOO",     AML_TOK_Identifier);
  T("^^FOO",    AML_TOK_Identifier);
  T("FOO.BAR",  AML_TOK_Identifier);

  T("Ones",     AML_TOK_Keyword);
  T("Method",   AML_TOK_Keyword);

  T("\"\"",     AML_TOK_String);
  T("\"\\\"\"", AML_TOK_String);

  T("",         AML_TOK_EOF);
#undef T

  return 0;
}

#include "../memory.c"
#include "../error.c"
#include "../nxjson.c"
