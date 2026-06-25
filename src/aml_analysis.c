#include "aml_analysis.h"

#include "memory.h"
#include "acpi_analysis.h"

#include <string.h> // strncmp, memset

static void AML_Analysis_RemoveDoubleParentheses(array_of(AML_Token)*);

/*
 * Initialize AML_Analysis.
 *
 * Each file in `aml_files` is disassembled and scanned for AML methods.
 * The extracted methods are stored in `analysis->methods`.
 *
 * Returns error on failure.
 */
Error AML_Analysis_Init(AML_Analysis* analysis, array_of(str)* aml_files) {
  Error e = err_success();
  AML_Lexer l;
  AML_Parser p;

  memset(analysis, 0, sizeof(AML_Analysis));

  analysis->sources.data = Mem_Calloc(aml_files->size, sizeof(AML_LexedSource));

  for_enumerate_array(array_size_t, i, *aml_files) {
    const char* const file = aml_files->data[i];
    AML_LexedSource* source = &analysis->sources.data[i];

    e = Acpi_Analysis_Get_DSL(file, &source->source);
    if (e)
      goto end;

    analysis->sources.size++;

    AML_Lexer_Init(&l, source->source);
    e = AML_Lexer_GetTokens(&l, &source->tokens);
    if (e) {
      e = err_chain_string(e, file);
      goto end;
    }

    AML_Analysis_RemoveDoubleParentheses(&source->tokens);

    AML_Parser_Init(&p, source->tokens);
    e = AML_Parser_ExtractMethods(&p, &analysis->methods);
    if (e) {
      e = err_chain_string(e, file);
      goto end;
    }
  }

end:
  if (e)
    AML_Analysis_Free(analysis);

  return e;
}

void AML_Analysis_Free(AML_Analysis* analysis) {
  Mem_Free(analysis->methods.data);

  for_each_array(AML_LexedSource*, source, analysis->sources) {
    Mem_Free(source->source);
    Mem_Free(source->tokens.data);
  }

  Mem_Free(analysis->sources.data);

  memset(analysis, 0, sizeof(AML_Analysis));
}

/*
 * Returns the next non-whitespace, non-comment token from the token list.
 *
 * The index pointed to by i is advanced until a real token is found.
 *
 * Returns NULL if no non-whitespace, non-comment tokens remain.
 */
static AML_Token* AML_Analysis_GetRealToken(array_of(AML_Token)* tokens, size_t* i) {
  for (; *i < tokens->size; ++(*i))
    switch (tokens->data[*i].type) {
    case AML_TOK_Comment: /* fall-through */
    case AML_TOK_Whitespace:
      continue;
    default:
      return &tokens->data[(*i)++];
    }

  return NULL;
}

/*
 * Like `AML_Analysis_GetRealToken`, but also updates `paren_count`.
 *
 * If an opening brace "{" is found, paren_count is incremented.
 * If a closing brace "}" is found, paren_count is decremented.
 */
static AML_Token* AML_Analysis_GetRealTokenWithScope(array_of(AML_Token)* tokens, size_t* i, size_t* paren_count) {
  for (; *i < tokens->size; ++(*i))
    switch (tokens->data[*i].type) {
    case AML_TOK_Comment: /* fall-through */
    case AML_TOK_Whitespace:
      continue;
    case AML_TOK_OP_BraceOpen:
      ++(*paren_count);
      return &tokens->data[(*i)++];
    case AML_TOK_OP_BraceClose:
      --(*paren_count);
      return &tokens->data[(*i)++];
    default:
      return &tokens->data[(*i)++];
    }

  return NULL;
}

/*
 * Returns true if both basenames of `a` and `b` are equal.
 *
 * "FOO",     "FOO"  -> true
 * "^FOO",    "FOO"  -> true
 * "\\FOO",   "FOO"  -> true
 * "BAR.FOO", "FOO"  -> true
 */
static bool AML_Analysis_IdentifierBasenameEquals(const AML_Token* a, const AML_Token* b) {
  size_t a_i = a->len - 1;
  size_t b_i = b->len - 1;

  for (;;) {
    const char a_c = (a_i == ((size_t) -1)) ? '\0' : a->s[a_i];
    const char b_c = (b_i == ((size_t) -1)) ? '\0' : b->s[b_i];

    if (a_c == '\0' || a_c == '\\' || a_c == '^' || a_c == '.')
      return (b_c == '\0' || b_c == '\\' || b_c == '^' || b_c == '.');

    if (a_c != b_c)
      return false;

    --a_i;
    --b_i;
  }
}

/*
 * Returns true if both tokens are equal.
 */
static bool AML_Analysis_TokenEquals(const AML_Token* a, const AML_Token* b) {
  if (a->type == b->type) {
    switch (a->type) {
    case AML_TOK_Integer:
      return a->number == b->number;
    case AML_TOK_Identifier:
      return AML_Analysis_IdentifierBasenameEquals(a, b);
    case AML_TOK_Keyword: /* fall-through */
    case AML_TOK_String:  /* fall-through */
    case AML_TOK_Comment: /* fall-through */
    case AML_TOK_Whitespace:
      return (a->len == b->len && !strncmp(a->s, b->s, a->len));
    default:
      return true;
    }
  }

  return false;
}

static bool AML_Analysis_EatParenthesesBlock(array_of(AML_Token)* tokens, size_t pos, size_t* end_pos) {
  AML_Token* t;
  size_t paren_count = 1;

  t = AML_Analysis_GetRealToken(tokens, &pos);
  if (! t || t->type != AML_TOK_OP_ParenOpen)
    return false;

  for (; pos < tokens->size; ++pos) {
    switch (tokens->data[pos].type) {
    case AML_TOK_OP_ParenOpen: /* fall-through */
    case AML_TOK_OP_BraceOpen: /* fall-through */
    case AML_TOK_OP_BracketOpen:
      ++paren_count;
      break;

    case AML_TOK_OP_ParenClose: /* fall-through */
    case AML_TOK_OP_BraceClose: /* fall-through */
    case AML_TOK_OP_BracketClose:
      if (! --paren_count) {
        *end_pos = pos + 1;
        return true;
      }

    default:
      break;
    }
  }

  return false;
}

/*
 * Remove double parentheses from tokens.
 *
 * If ((Arg0 == 0xFF))  ->  If (Arg0 == 0xFF)
 */
static void AML_Analysis_RemoveDoubleParentheses(array_of(AML_Token)* tokens) {
  AML_Token* t;
  size_t end_pos;

  for (size_t pos = 0; pos < tokens->size; ++pos) {
    if (tokens->data[pos].type != AML_TOK_OP_ParenOpen)
      continue;

    if (! AML_Analysis_EatParenthesesBlock(tokens, pos + 1, &end_pos))
      continue;

    if (! (t = AML_Analysis_GetRealToken(tokens, &end_pos)))
      continue;

    if (t->type != AML_TOK_OP_ParenClose)
      continue;

    tokens->data[pos].type = AML_TOK_Whitespace;
    tokens->data[pos].len = 0;
    tokens->data[end_pos - 1].type = AML_TOK_Whitespace;
    tokens->data[end_pos - 1].len = 0;
  }
}

typedef struct AML_Analysis_Pattern AML_Analysis_Pattern;
struct AML_Analysis_Pattern {
  char* source;
  array_of(AML_Token) tokens;
  AML_Token method_name;
  array_of(AML_Token) pattern;
};

static void AML_Analysis_Pattern_Free(AML_Analysis_Pattern* pattern) {
  Mem_Free(pattern->source);
  Mem_Free(pattern->tokens.data);
}

/*
 * Initialize an AML_Analysis_Pattern.
 */
static Error AML_Analysis_Pattern_Init(AML_Analysis_Pattern* pattern, const char* s) {
  Error e;
  AML_Lexer l;
  AML_Parser p;
  AML_Token t;

  memset(pattern, 0, sizeof(AML_Analysis_Pattern));

  pattern->source = Mem_Strdup(s);

  AML_Lexer_Init(&l, pattern->source);
  e = AML_Lexer_GetTokens(&l, &pattern->tokens);
  if (e)
    goto err;

  AML_Analysis_RemoveDoubleParentheses(&pattern->tokens);

  AML_Parser_Init(&p, pattern->tokens);

  if (! AML_Parser_ParseKeyword(&p, &t) ||
      ! AML_Token_StrEq(&t, "Method", STRLEN("Method")))
  {
    e = err_string("Expected \"Method\"");
    goto err;
  }

  if (! AML_Parser_EatParenOpen(&p) ||
      ! AML_Parser_ParseIdentifier(&p, &t) ||
      ! AML_Parser_EatParenClose(&p))
  {
    e = AML_Parser_GetError(&p);
    goto err;
  }

  pattern->method_name = t;

  if (! AML_Parser_GetRealToken(&p, &t))
    return err_success();

  if (t.type != AML_TOK_OP_BraceOpen) {
    e = err_string("Expected {");
    goto err;
  }

  pattern->pattern.data = &pattern->tokens.data[p.pos - 1];
  pattern->pattern.size = pattern->tokens.size - p.pos + 1;
  return err_success();

err:
  AML_Analysis_Pattern_Free(pattern);
  return e;
};

static bool AML_Analysis_Tokens_Match_Pattern(
  array_of(AML_Token)* tokens,
  array_of(AML_Token)* pattern,
  size_t paren_count,
  size_t token_i,
  size_t pattern_i)
{
  AML_Token* tok;
  AML_Token* pattern_tok = AML_Analysis_GetRealToken(pattern, &pattern_i);
  size_t last_paren_count = paren_count;

  if (! pattern_tok) {
    return true;
  }

  switch (pattern_tok->type) {
  case AML_TOK_AnyInScope:
    if (! (pattern_tok = AML_Analysis_GetRealToken(pattern, &pattern_i)))
      return true;

    for (;;) {
      if (! (tok = AML_Analysis_GetRealTokenWithScope(tokens, &token_i, &paren_count)))
        return false;

      if (last_paren_count > paren_count) {
        return false;
      }

      if (last_paren_count == paren_count) {
        if (AML_Analysis_TokenEquals(tok, pattern_tok)) {
          bool match = AML_Analysis_Tokens_Match_Pattern(tokens, pattern, paren_count, token_i, pattern_i);
          if (match)
            return true;
        }
      }
    }
    break;

  case AML_TOK_Any:
    if (! (pattern_tok = AML_Analysis_GetRealToken(pattern, &pattern_i)))
      return true;

    for (;;) {
      if (! (tok = AML_Analysis_GetRealTokenWithScope(tokens, &token_i, &paren_count)))
        return false;

      if (last_paren_count > paren_count) {
        return false;
      }

      if (AML_Analysis_TokenEquals(tok, pattern_tok)) {
        bool match = AML_Analysis_Tokens_Match_Pattern(tokens, pattern, paren_count, token_i, pattern_i);
        if (match)
          return true;
      }
    }
    break;

  default:
    if (! (tok = AML_Analysis_GetRealTokenWithScope(tokens, &token_i, &paren_count))) {
      return false;
    }

    if (! AML_Analysis_TokenEquals(pattern_tok, tok)) {
      return false;
    }

    return AML_Analysis_Tokens_Match_Pattern(tokens, pattern, paren_count, token_i, pattern_i);
  }
}

/*
 * Matches a firmware fingerprint against AML code.
 *
 * If the firmware contains the fingerprint, `match` will be set to true.
 *
 * If the fingerprint itself is invalid, an error is returned.
 */
Error AML_Analysis_MatchFingerprint(AML_Analysis* analysis, const char* fingerprint, bool* match) {
  Error e;
  AML_Analysis_Pattern pattern;

  *match = false;

  e = AML_Analysis_Pattern_Init(&pattern, fingerprint);
  if (e)
    return e;

  for_each_array(AML_Method*, aml_method, analysis->methods) {
    const bool name_match = AML_Token_StrEq(
        &pattern.method_name, aml_method->name, strlen(aml_method->name));

    if (name_match)
    {
      *match = AML_Analysis_Tokens_Match_Pattern(&aml_method->tokens, &pattern.pattern, 0, 0, 0);
      if (*match)
        break;
    }
  }

  AML_Analysis_Pattern_Free(&pattern);
  return err_success();
}
