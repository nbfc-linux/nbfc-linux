#include "aml_lexer.h"

#include <string.h> // strncmp, strchr, strspn, strstr, strlen, strerror
#include <stdlib.h> // strtoull
#include <errno.h>  // errno
#include <stdio.h>  // printf

#include "memory.h"

/*
 * Initialize an AML_Lexer.
 */
void AML_Lexer_Init(AML_Lexer* l, const char* source) {
  l->source = source;
  l->pos = 0;
}

/*
 * Return the character in the lexer at offset without advancing.
 */
static inline char AML_Lexer_PeekChar(const AML_Lexer* l, size_t offset) {
  return l->source[l->pos + offset];
}

/*
 * Return the text at the current position of the lexer.
 */
static inline const char* AML_Lexer_CurText(const AML_Lexer* l) {
  return l->source + l->pos;
}

/*
 * Return true if the current char is an identifier.
 */
static inline bool AML_Lexer_IsIdentifier(char c) {
  return (
    (c >= 'A' && c <= 'Z') ||
    (c >= 'a' && c <= 'z') ||
    (c >= '0' && c <= '9') ||
    (c == '\\') ||
    (c == '.')  ||
    (c == '_')  ||
    (c == '^')
  );
}

/*
 * Parse whitespace and store the result in token.
 */
static void AML_Lexer_ParseWhitespace(AML_Lexer* l, AML_Token* t) {
  t->type = AML_TOK_Whitespace;
  t->s = AML_Lexer_CurText(l);
  t->len = strspn(t->s, " \t\r\n");
  l->pos += t->len;
}

/*
 * Parse a line comment and store the result in token.
 */
static void AML_Lexer_ParseLineComment(AML_Lexer* l, AML_Token* t) {
  t->type = AML_TOK_Comment;
  t->s = AML_Lexer_CurText(l);
  const char* end = strchr(t->s, '\n');
  if (end)
    t->len = end - t->s + 1;
  else
    t->len = strlen(t->s);
  l->pos += t->len;
}

/*
 * Parse a block comment and store the result in token.
 */
static void AML_Lexer_ParseBlockComment(AML_Lexer* l, AML_Token* t) {
  t->type = AML_TOK_Comment;
  t->s = AML_Lexer_CurText(l);

  const char* end = strstr(t->s, "*/");
  if (end) {
    t->len = end - t->s + 2;
    l->pos += t->len;
  }
  else {
    t->type = AML_TOK_Error;
    l->error = "Unterminated block comment";
  }
}

/*
 * Parse a string and store the result in token.
 */
static void AML_Lexer_ParseString(AML_Lexer* l, AML_Token* t) {
  t->type = AML_TOK_String;
  t->s = AML_Lexer_CurText(l);
  t->len = 1;

  for (;;) {
    switch (AML_Lexer_PeekChar(l, t->len)) {
    case '\0':
      t->type = AML_TOK_Error;
      l->error = "Unterminated string";
      return;

    case '"':
      t->len++;
      l->pos += t->len;
      return;

    case '\\':
      t->len++;
      if (! AML_Lexer_PeekChar(l, t->len)) {
        t->type = AML_TOK_Error;
        l->error = "Unterminated string";
      }
    }

    t->len++;
  }
}

/*
 * Parse an operator or a special character and store the result in token.
 *
 * Return true if an operator was found, false otherwise.
 */
static bool AML_Lexer_ParseOperator(AML_Lexer* l, AML_Token* t) {
  const char* const cur = AML_Lexer_CurText(l);
  if (! cur[0])
    return false;

  t->s = cur;

#define AML_RET(TYPE, LEN) \
  t->type = TYPE; \
  t->len = LEN; \
  l->pos += LEN; \
  return true

  switch (cur[1]) {
  case '=':
    switch (cur[0]) {
    case '=': AML_RET(AML_TOK_OP_Equals,        2);
    case '!': AML_RET(AML_TOK_OP_Unequals,      2);
    case '>': AML_RET(AML_TOK_OP_GreaterEquals, 2);
    case '<': AML_RET(AML_TOK_OP_LessEquals,    2);
    case '|': AML_RET(AML_TOK_OP_OrAssign,      2);
    case '&': AML_RET(AML_TOK_OP_AndAssign,     2);
    case '^': AML_RET(AML_TOK_OP_XorAssign,     2);
    case '+': AML_RET(AML_TOK_OP_AddAssign,     2);
    case '-': AML_RET(AML_TOK_OP_SubAssign,     2);
    case '*': AML_RET(AML_TOK_OP_MulAssign,     2);
    case '/': AML_RET(AML_TOK_OP_DivAssign,     2);
    case '%': AML_RET(AML_TOK_OP_ModAssign,     2);
    }
    break;

  case '.':
    if (cur[0] == '.') {
      if (cur[2] == '.') { AML_RET(AML_TOK_Any,           3); }
      else               { AML_RET(AML_TOK_AnyInScope,    2); }
    }
    break;

  case '<':
    if (cur[0] == '<') {
      if (cur[2] == '=') { AML_RET(AML_TOK_OP_LSH_Assign, 3); }
      else               { AML_RET(AML_TOK_OP_LSH,        2); }
    }
    break;

  case '>':
    if (cur[0] == '>') {
      if (cur[2] == '=') { AML_RET(AML_TOK_OP_RSH_Assign, 3); }
      else               { AML_RET(AML_TOK_OP_RSH,        2); }
    }
    break;

  case '&':
    if (cur[0] == '&')   { AML_RET(AML_TOK_OP_And,        2); }
    break;

  case '|':
    if (cur[0] == '|')   { AML_RET(AML_TOK_OP_Or,         2); }
    break;
  }

  switch (cur[0]) {
  case ',': AML_RET(AML_TOK_OP_Comma,        1);
  case '(': AML_RET(AML_TOK_OP_ParenOpen,    1);
  case ')': AML_RET(AML_TOK_OP_ParenClose,   1);
  case '{': AML_RET(AML_TOK_OP_BraceOpen,    1);
  case '}': AML_RET(AML_TOK_OP_BraceClose,   1);
  case '[': AML_RET(AML_TOK_OP_BracketOpen,  1);
  case ']': AML_RET(AML_TOK_OP_BracketClose, 1);
  case '|': AML_RET(AML_TOK_OP_BitOr,        1);
  case '&': AML_RET(AML_TOK_OP_BitAnd,       1);
  case '=': AML_RET(AML_TOK_OP_Assign,       1);
  case '!': AML_RET(AML_TOK_OP_Not,          1);
  case '~': AML_RET(AML_TOK_OP_BitNot,       1);
  case '^': AML_RET(AML_TOK_OP_BitXor,       1);
  case '+': AML_RET(AML_TOK_OP_Add,          1);
  case '-': AML_RET(AML_TOK_OP_Sub,          1);
  case '*': AML_RET(AML_TOK_OP_Mul,          1);
  case '/': AML_RET(AML_TOK_OP_Div,          1);
  case '%': AML_RET(AML_TOK_OP_Mod,          1);
  case '<': AML_RET(AML_TOK_OP_Less,         1);
  case '>': AML_RET(AML_TOK_OP_Greater,      1);
  }

  return false;
}

/*
 * Parse an integer and store the result in token.
 *
 * Integer may be:
 *   - a decimal number
 *   - a hexadecimal number
 *   - an octal number
 *   - "One"
 *   - "Zero"
 *
 * This function returns true if an integer could be found, false otherwise.
 * If an error (like overflow) occured, the token will be set to AML_TOK_Error.
 */
static bool AML_Lexer_ParseInteger(AML_Lexer* l, AML_Token* t) {
  const char* const cur = AML_Lexer_CurText(l);
  char* end;

  switch (cur[0]) {
  case 'O':
    // "One", but not "Ones"
    if (cur[1] == 'n' && cur[2] == 'e' && cur[3] != 's') {
      t->type = AML_TOK_Integer;
      t->number = 1;
      t->s = cur;
      t->len = 3;
      l->pos += 3;
      return true;
    }
    break;

  case 'Z':
    // "Zero"
    if (cur[1] == 'e' && cur[2] == 'r' && cur[3] == 'o') {
      t->type = AML_TOK_Integer;
      t->number = 0;
      t->s = cur;
      t->len = 4;
      l->pos += 4;
      return true;
    }
    break;

  case '0': /* fall-through */
  case '1': /* fall-through */
  case '2': /* fall-through */
  case '3': /* fall-through */
  case '4': /* fall-through */
  case '5': /* fall-through */
  case '6': /* fall-through */
  case '7': /* fall-through */
  case '8': /* fall-through */
  case '9':
    errno = 0;
    t->number = strtoull(cur, &end, 0);

    if (errno) {
      t->type = AML_TOK_Error;
      l->error = strerror(errno);
      return true;
    }

    if (cur != end) {
      t->type = AML_TOK_Integer;
      t->s = cur;
      t->len = end - cur;
      l->pos += end - cur;
      return true;
    }
    break;
  }

  return false;
}

/*
 * Parse a keyword or an identifier and store the result in token.
 *
 * Keywords are something like: Method, Switch, Case
 * Identifiers are something like: \FOO, FOO, FOO.BAR, ^^FOO
 */
static void AML_Lexer_ParseKeywordOrIdentifier(AML_Lexer* l, AML_Token* t) {
  t->s = AML_Lexer_CurText(l);
  t->len = strspn(t->s, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz"
                        "0123456789"
                        "._^\\");
  l->pos += t->len;
  t->type = AML_TOK_Identifier;

  // If we see a lower case letter, it's a keyword
  for (size_t i = 1; i < t->len; ++i)
    if (t->s[i] >= 'a' && t->s[i] <= 'z') {
      t->type = AML_TOK_Keyword;
      return;
    }
}

/*
 * Get the next token from lexer and advance the stream.
 *
 * Returns an AML_TOK_EOF token if no more tokens can be parsed.
 * Returns an AML_TOK_Error token if an error occured.
 */
AML_Token AML_Lexer_GetToken(AML_Lexer* l) {
  AML_Token t;
  const char c = AML_Lexer_PeekChar(l, 0);
  char c2;

  switch (c) {
  case '\0':
    t.type = AML_TOK_EOF;
    t.s = "";
    t.len = 0;
    return t;

  case ' ':  /* fall-through */
  case '\t': /* fall-through */
  case '\r': /* fall-through */
  case '\n':
    AML_Lexer_ParseWhitespace(l, &t);
    return t;

  case '"':
    AML_Lexer_ParseString(l, &t);
    return t;

  case '/':
    c2 = AML_Lexer_PeekChar(l, 1);
    if (c2 == '/') {
      AML_Lexer_ParseLineComment(l, &t);
      return t;
    }

    if (c2 == '*') {
      AML_Lexer_ParseBlockComment(l, &t);
      return t;
    }

    AML_Lexer_ParseOperator(l, &t);
    return t;

  case '^':
    if (AML_Lexer_IsIdentifier(AML_Lexer_PeekChar(l, 1))) {
      AML_Lexer_ParseKeywordOrIdentifier(l, &t);
      return t;
    }
    else {
      AML_Lexer_ParseOperator(l, &t);
      return t;
    }

  case '.': /* fall-through */
  case '(': /* fall-through */
  case ')': /* fall-through */
  case '{': /* fall-through */
  case '}': /* fall-through */
  case '[': /* fall-through */
  case ']': /* fall-through */
  case ',': /* fall-through */
  case '!': /* fall-through */
  case '=': /* fall-through */
  case '<': /* fall-through */
  case '>': /* fall-through */
  case '+': /* fall-through */
  case '-': /* fall-through */
  case '*': /* fall-through */
  case '%': /* fall-through */
  case '~': /* fall-through */
  case '|': /* fall-through */
  case '&':
    AML_Lexer_ParseOperator(l, &t);
    return t;

  case '0': /* fall-through */
  case '1': /* fall-through */
  case '2': /* fall-through */
  case '3': /* fall-through */
  case '4': /* fall-through */
  case '5': /* fall-through */
  case '6': /* fall-through */
  case '7': /* fall-through */
  case '8': /* fall-through */
  case '9':
    AML_Lexer_ParseInteger(l, &t);
    return t;
  }

  if (AML_Lexer_ParseInteger(l, &t))
    return t;

  AML_Lexer_ParseKeywordOrIdentifier(l, &t);
  return t;
}

/*
 * Parse all tokens in lexer and store it in an array of tokens.
 *
 * Returns an error on failure.
 */
Error AML_Lexer_GetTokens(AML_Lexer* l, array_of(AML_Token)* out) {
  size_t capacity = 1024;
  out->size = 0;
  out->data = Mem_Malloc(capacity * sizeof(AML_Token));

  for (;;) {
    AML_Token t = AML_Lexer_GetToken(l);

    switch (t.type) {
    case AML_TOK_Error: return err_string(l->error);
    case AML_TOK_EOF:   return err_success();
    default:            break;
    }

    if (out->size >= capacity) {
      capacity += 1024;
      out->data = Mem_Realloc(out->data, capacity * sizeof(AML_Token));
    }

    out->data[out->size++] = t;
  }

  return err_success();
}

/*
 * Return true if token is equal to string.
 */
bool AML_Token_StrEq(const AML_Token* t, const char* string, size_t len) {
  return (t->len == len && !strncmp(t->s, string, len));
}

/*
 * Print a token.
 */
void AML_Token_Print(const AML_Token* t) {
  printf("> %.*s\n", (int) t->len, t->s);
}

const char* AML_TokenType_ToStr(AML_TokenType type) {
  switch (type) {
  case AML_TOK_Error:             return "?";
  case AML_TOK_EOF:               return "EOF";
  case AML_TOK_Whitespace:        return "whitespace";
  case AML_TOK_Comment:           return "comment";
  case AML_TOK_Integer:           return "integer";
  case AML_TOK_String:            return "string";
  case AML_TOK_Keyword:           return "keyword";
  case AML_TOK_Identifier:        return "identifier";
  case AML_TOK_Any:               return "...";
  case AML_TOK_AnyInScope:        return "..";
  case AML_TOK_OP_LSH_Assign:     return "<<=";
  case AML_TOK_OP_RSH_Assign:     return ">>=";
  case AML_TOK_OP_LSH:            return "<<";
  case AML_TOK_OP_RSH:            return ">>";
  case AML_TOK_OP_Equals:         return "==";
  case AML_TOK_OP_Unequals:       return "!=";
  case AML_TOK_OP_GreaterEquals:  return ">=";
  case AML_TOK_OP_LessEquals:     return "<=";
  case AML_TOK_OP_OrAssign:       return "|=";
  case AML_TOK_OP_AndAssign:      return "&=";
  case AML_TOK_OP_XorAssign:      return "^=";
  case AML_TOK_OP_AddAssign:      return "+=";
  case AML_TOK_OP_SubAssign:      return "-=";
  case AML_TOK_OP_MulAssign:      return "*=";
  case AML_TOK_OP_DivAssign:      return "/=";
  case AML_TOK_OP_ModAssign:      return "%=";
  case AML_TOK_OP_Or:             return "||";
  case AML_TOK_OP_And:            return "&&";
  case AML_TOK_OP_Not:            return "!";
  case AML_TOK_OP_BitOr:          return "|";
  case AML_TOK_OP_BitAnd:         return "&";
  case AML_TOK_OP_BitNot:         return "~";
  case AML_TOK_OP_BitXor:         return "^";
  case AML_TOK_OP_Add:            return "+";
  case AML_TOK_OP_Sub:            return "-";
  case AML_TOK_OP_Mul:            return "*";
  case AML_TOK_OP_Div:            return "/";
  case AML_TOK_OP_Mod:            return "%";
  case AML_TOK_OP_Assign:         return "=";
  case AML_TOK_OP_Less:           return "<";
  case AML_TOK_OP_Greater:        return ">";
  case AML_TOK_OP_Comma:          return ",";
  case AML_TOK_OP_ParenOpen:      return "(";
  case AML_TOK_OP_ParenClose:     return ")";
  case AML_TOK_OP_BraceOpen:      return "{";
  case AML_TOK_OP_BraceClose:     return "}";
  case AML_TOK_OP_BracketOpen:    return "[";
  case AML_TOK_OP_BracketClose:   return "]";
  }

  return "?";
}
