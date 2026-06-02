#include "../aml_analysis.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static AML_Analysis analysis;
static AML_Analysis_Pattern pattern;
static array_of(AML_Token) tokens;

#if 0
static void AML_Tokens_Print(array_of(AML_Token)* tokens) {
  for_each_array(AML_Token*, t, *tokens)
    AML_Token_Print(t);
}
#endif

static void Test_Init() {
  str files[1] = {"./DSDT.aml"};
  array_of(str) aml_files;
  aml_files.data = files;
  aml_files.size = 1;

  Error e;
  e = AML_Analysis_Init(&analysis, &aml_files);
  if (e) {
    printf("Test_Init(): %s\n", err_print_all(e));
    exit(1);
  }
}

static void Test_For_Methods() {
  bool M001 = false;
  bool M002 = false;
  bool M003 = false;
  bool M004 = false;

  for_each_array(AML_Method*, aml_method, analysis.methods) {
    M001 |= !strcmp(aml_method->name, "\\M001");
    M002 |= !strcmp(aml_method->name, "\\M002");
    M003 |= !strcmp(aml_method->name, "\\_SB.M003");
    M004 |= !strcmp(aml_method->name, "\\M004");
  }

  if (! M001) {
    printf("Test_For_Methods(): M001 not found\n");
    exit(2);
  }

  if (! M002) {
    printf("Test_For_Methods(): M002 not found\n");
    exit(2);
  }

  if (! M003) {
    printf("Test_For_Methods(): M003 not found\n");
    exit(2);
  }

  if (! M004) {
    printf("Test_For_Methods(): M004 not found\n");
    exit(2);
  }
}

static void Test_Pattern_Init(const char* s) {
  Error e;
  e = AML_Analysis_Pattern_Init(&pattern, s);
  if (e) {
    printf("Test_Pattern_Init(\"%s\"): %s\n", s, err_print_all(e));
    exit(3);
  }
}

static void Test_Pattern_Expect_Error(const char* s, const char* error) {
  Error e;
  e = AML_Analysis_Pattern_Init(&pattern, s);
  if (! e) {
    printf("Test_Pattern_Expect_Error(\"%s\"): Expected error\n", s);
    exit(3);
  }

  const char* e_string = err_print_all(e);

  if (strcmp(e_string, error)) {
    printf("Test_Pattern_Expect_Error(\"%s\"): Expected %s, Having %s\n", s, error, e_string);
    exit(3);
  }
}

static void Test_Pattern_Free() {
  AML_Analysis_Pattern_Free(&pattern);
}

static void Test_Patterns() {
  Test_Pattern_Init("Method(\\FOO)");
  assert(AML_Token_StrEq(&pattern.method_name, "\\FOO", 4));
  assert(pattern.pattern.size == 0);
  Test_Pattern_Free();

  Test_Pattern_Init("Method(\\FOO) {");
  assert(AML_Token_StrEq(&pattern.method_name, "\\FOO", 4));
  assert(pattern.pattern.size == 1);
  Test_Pattern_Free();

  Test_Pattern_Init("Method(\\FOO) { 0xFF");
  assert(AML_Token_StrEq(&pattern.method_name, "\\FOO", 4));
  assert(pattern.pattern.size == 3);
  assert(pattern.pattern.data[2].type == AML_TOK_Integer);
  assert(pattern.pattern.data[2].number = 0xFF);
  Test_Pattern_Free();

  Test_Pattern_Init("Method(\\FOO) { Switch(0xF00) { 0xFF");
  assert(AML_Token_StrEq(&pattern.method_name, "\\FOO", 4));
  assert(pattern.pattern.size == 10);
  assert(pattern.pattern.data[4].type == AML_TOK_Integer);
  assert(pattern.pattern.data[4].number = 0xF00);
  assert(pattern.pattern.data[9].type == AML_TOK_Integer);
  assert(pattern.pattern.data[9].number = 0xFF);
  Test_Pattern_Free();

  Test_Pattern_Expect_Error("",            "Expected \"Method\"");
  Test_Pattern_Expect_Error("Foo",         "Expected \"Method\"");
  Test_Pattern_Expect_Error("Method",      "Unexpected end of input");
  Test_Pattern_Expect_Error("Method(",     "Unexpected end of input");
  Test_Pattern_Expect_Error("Method(FOO",  "Unexpected end of input");
  Test_Pattern_Expect_Error("Method{",     "Expected token: (");
}

static void Test_Tokens_Init(const char* s) {
  Error e;
  AML_Lexer l;

  AML_Lexer_Init(&l, s);
  e = AML_Lexer_GetTokens(&l, &tokens);
  if (e) {
    printf("Test_Tokens_Init(\"%s\"): %s\n", s, err_print_all(e));
    exit(4);
  }
}

static void Test_Tokens_Free() {
  Mem_Free(tokens.data);
}

static void Test_AML_Analysis_RemoveDoubleParentheses() {
  Test_Tokens_Init("(())");
  AML_Analysis_RemoveDoubleParentheses(&tokens);
  assert(tokens.size == 4);
  assert(tokens.data[0].type == AML_TOK_Whitespace);
  assert(tokens.data[1].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[2].type == AML_TOK_OP_ParenClose);
  assert(tokens.data[3].type == AML_TOK_Whitespace);
  Test_Tokens_Free();

  Test_Tokens_Init("((FOO))");
  AML_Analysis_RemoveDoubleParentheses(&tokens);
  assert(tokens.size == 5);
  assert(tokens.data[0].type == AML_TOK_Whitespace);
  assert(tokens.data[1].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[2].type == AML_TOK_Identifier);
  assert(tokens.data[3].type == AML_TOK_OP_ParenClose);
  assert(tokens.data[4].type == AML_TOK_Whitespace);
  Test_Tokens_Free();

  Test_Tokens_Init("(((FOO)))");
  AML_Analysis_RemoveDoubleParentheses(&tokens);
  assert(tokens.size == 7);
  assert(tokens.data[0].type == AML_TOK_Whitespace);
  assert(tokens.data[1].type == AML_TOK_Whitespace);
  assert(tokens.data[2].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[3].type == AML_TOK_Identifier);
  assert(tokens.data[4].type == AML_TOK_OP_ParenClose);
  assert(tokens.data[5].type == AML_TOK_Whitespace);
  assert(tokens.data[6].type == AML_TOK_Whitespace);
  Test_Tokens_Free();

  Test_Tokens_Init("((FOO) )");
  AML_Analysis_RemoveDoubleParentheses(&tokens);
  assert(tokens.size == 6);
  assert(tokens.data[0].type == AML_TOK_Whitespace);
  assert(tokens.data[1].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[2].type == AML_TOK_Identifier);
  assert(tokens.data[3].type == AML_TOK_OP_ParenClose);
  assert(tokens.data[4].type == AML_TOK_Whitespace);
  assert(tokens.data[5].type == AML_TOK_Whitespace);
  Test_Tokens_Free();

  Test_Tokens_Init("FOO((");
  AML_Analysis_RemoveDoubleParentheses(&tokens);
  assert(tokens.size == 3);
  assert(tokens.data[0].type == AML_TOK_Identifier);
  assert(tokens.data[1].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[2].type == AML_TOK_OP_ParenOpen);
  Test_Tokens_Free();

  Test_Tokens_Init("((FOO)");
  AML_Analysis_RemoveDoubleParentheses(&tokens);
  assert(tokens.size == 4);
  assert(tokens.data[0].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[1].type == AML_TOK_OP_ParenOpen);
  assert(tokens.data[2].type == AML_TOK_Identifier);
  assert(tokens.data[3].type == AML_TOK_OP_ParenClose);
  Test_Tokens_Free();
}

static void Test_AML_Analysis_IdentifierBasenameEquals(const char* a, const char* b, bool expected) {
  AML_Token token_a;
  AML_Token token_b;
  AML_Lexer l;

  AML_Lexer_Init(&l, a);
  token_a = AML_Lexer_GetToken(&l);
  if (token_a.type != AML_TOK_Identifier) {
    printf("Test_AML_Analysis_IdentifierBasenameEquals(\"%s\", \"%s\"): %s: Not an identifier\n", a, b, a);
    exit(5);
  }

  AML_Lexer_Init(&l, b);
  token_b = AML_Lexer_GetToken(&l);
  if (token_b.type != AML_TOK_Identifier) {
    printf("Test_AML_Analysis_IdentifierBasenameEquals(\"%s\", \"%s\"): %s: Not an identifier\n", a, b, b);
    exit(5);
  }

  bool result = AML_Analysis_IdentifierBasenameEquals(&token_a, &token_b);
  if (result != expected) {
    printf("Test_AML_Analysis_IdentifierBasenameEquals(\"%s\", \"%s\"): %d != %d\n", a, b, result, expected);
    exit(5);
  }
}

static void Test_AML_Analysis_MatchFingerprint(const char* fingerprint, bool expected) {
  Error e;
  bool result;

  e = AML_Analysis_MatchFingerprint(&analysis, fingerprint, &result);
  if (e) {
    printf("Test_AML_Analysis_MatchFingerprint(\"%s\"): %s\n", fingerprint, err_print_all(e));
    exit(6);
  }

  if (result != expected) {
    printf("Test_AML_Analysis_MatchFingerprint(\"%s\"): %d != %d\n", fingerprint, result, expected);
    exit(6);
  }
}

int main() {
  Test_Init();
  Test_For_Methods();
  Test_Patterns();
  Test_AML_Analysis_RemoveDoubleParentheses();

#define T(...) Test_AML_Analysis_IdentifierBasenameEquals(__VA_ARGS__)
  T("FOO",      "FOO",         true);

  T("\\FOO",    "FOO",         true);
  T("^FOO",     "FOO",         true);
  T("BAR.FOO",  "FOO",         true);

  T("FOO",      "\\FOO",       true);
  T("FOO",      "^FOO",        true);
  T("FOO",      "BAR.FOO",     true);

  T("FOO",      "OO",          false);
#undef T

#define T(...) Test_AML_Analysis_MatchFingerprint(__VA_ARGS__)
  T("Method(\\M001)",                                             true);
  T("Method(\\M002)",                                             true);
  T("Method(\\_SB.M003)",                                         true);
  T("Method(\\M004)",                                             true);
  T("Method(\\MERR)",                                             false);

  T("Method(\\M001) {",                                           true);
  T("Method(\\M001) { ... FFOO",                                  true);
  T("Method(\\M001) { .. Switch(Arg0)",                           true);
  T("Method(\\M001) { .. Switch(Arg0) {",                         true);
  T("Method(\\M001) { .. Switch(Arg0) { .. Case(One)",            true);
  T("Method(\\M001) { .. Switch(Arg0) { .. Case(One) {",          true);
  T("Method(\\M001) { .. Switch(Arg0) { .. Case(One) { .. FFOO",  true);
  T("Method(\\M001){..Switch(Arg0){..Case(One){..FFOO",           true);

  T("Method(\\M002) { .. Switch(Arg0) { .. Case(One) { .. FFOO",  false);
  T("Method(\\M002) { .. Switch(Arg0) { .. Case(One) { ... FFOO", true);
  T("Method(\\M002) { .. Switch(Arg0) { ... Case(One) { .. FFOO", true);

  T("Method(\\M004) { If(Arg0 == 0xFF)",                          true);
  T("Method(\\M004) { If(Arg0 == 0xFF) { Return(0xFF)",           true);
  T("Method(\\M004) {If(Arg0==0xFF){Return(0xFF)",                true);

  T("Method(\\M004) { If((Arg0 == 0xFF))",                        true);
  T("Method(\\M004) { If((Arg0 == 0xFF)) { Return(0xFF)",         true);
  T("Method(\\M004) {If((Arg0==0xFF)){Return(0xFF)",              true);
#undef T
}

#include "../error.c"
#include "../memory.c"
#include "../file_utils.c"
#include "../regex_utils.c"
#include "../process.c"
#include "../nxjson.c"
#include "../aml_lexer.c"
#include "../aml_parser.c"
#include "../acpi_analysis.c"
