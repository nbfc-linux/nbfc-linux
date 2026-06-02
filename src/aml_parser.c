#include "aml_parser.h"

// Holds arguments for Scope() { }
typedef struct AML_ScopeArgs AML_ScopeArgs;
struct AML_ScopeArgs {
  AML_Token name;
};

// Holds arguments for Device() { }
typedef struct AML_DeviceArgs AML_DeviceArgs;
struct AML_DeviceArgs {
  AML_Token name;
};

// Holds arguments for ThermalZone() { }
typedef struct AML_ThermalZone AML_ThermalZone;
struct AML_ThermalZone {
  AML_Token name;
};

// Holds arguments for Method() { }
typedef struct AML_MethodArgs AML_MethodArgs;
struct AML_MethodArgs {
  AML_Token name;
  AML_Token len;
  AML_Token serialized;
};

/*
 * Structure used for building a namespace path.
 */
typedef struct AML_Namespace AML_Namespace;
struct AML_Namespace {
  char path[AML_PARSER_MAX_NAMESPACE_LEN];
  size_t size;
};

/*
 * Initialize a AML_Parser.
 */
void AML_Parser_Init(AML_Parser* p, array_of(AML_Token) tokens) {
  memset(p, 0, sizeof(AML_Parser));
  p->tokens = tokens;
}

/*
 * Parse the next "real" token (not whitespace or comments).
 *
 * Returns true on success, false otherwise.
 */
bool AML_Parser_GetRealToken(AML_Parser* p, AML_Token* t) {
  for (; p->pos < p->tokens.size; p->pos++) {
    switch (p->tokens.data[p->pos].type) {
    case AML_TOK_Comment: /* fall-through */
    case AML_TOK_Whitespace:
      continue;

    default:
      *t = p->tokens.data[p->pos];
      p->pos++;
      return true;
    }
  }

  return false;
}

/*
 * Parse a certain token type without storing it.
 *
 * If the next token is of such a type, return true.
 * Otherwise return false and set an error.
 */
bool AML_Parser_EatToken(AML_Parser* p, AML_TokenType type) {
  AML_Token t;

  if (! AML_Parser_GetRealToken(p, &t)) {
    p->error = "Unexpected end of input";
    p->error2 = NULL;
    return false;
  }

  if (t.type != type) {
    p->error = "Expected token";
    p->error2 = AML_TokenType_ToStr(type);
    return false;
  }

  return true;
}

/*
 * Parse a certain token type and store it in token.
 *
 * If the next token is of such a type, return true.
 * Otherwise return false and set an error.
 */
bool AML_Parser_ParseToken(AML_Parser* p, AML_TokenType type, AML_Token* t) {
  if (! AML_Parser_GetRealToken(p, t)) {
    p->error = "Unexpected end of input";
    p->error2 = NULL;
    return false;
  }

  if (t->type != type) {
    p->error = "Expected token";
    p->error2 = AML_TokenType_ToStr(type);
    return false;
  }

  return true;
}

/*
 * Parse arguments of `Scope` and save the parsed tokens in `args`.
 *
 * Return true on success, false on error.
 */
static bool AML_Parse_ScopeArgs(AML_Parser* p, AML_ScopeArgs* args) {
  return
      AML_Parser_EatParenOpen(p) &&
      AML_Parser_ParseIdentifier(p, &args->name) &&
      AML_Parser_EatParenClose(p);
}

/*
 * Parse arguments of `Device` and save the parsed tokens in `args`.
 *
 * Return true on success, false on error.
 */
static bool AML_Parse_DeviceArgs(AML_Parser* p, AML_DeviceArgs* args) {
  return
      AML_Parser_EatParenOpen(p) &&
      AML_Parser_ParseIdentifier(p, &args->name) &&
      AML_Parser_EatParenClose(p);
}

/*
 * Parse arguments of `ThermalZone` and save the parsed tokens in `args`.
 *
 * Return true on success, false on error.
 */
static bool AML_Parse_ThermalZoneArgs(AML_Parser* p, AML_ThermalZone* args) {
  return
      AML_Parser_EatParenOpen(p) &&
      AML_Parser_ParseIdentifier(p, &args->name) &&
      AML_Parser_EatParenClose(p);
}

/*
 * Parse arguments of `Method` and save the parsed tokens in `args`.
 *
 * Return true on success, false on error.
 */
static bool AML_Parse_MethodArgs(AML_Parser* p, AML_MethodArgs* args) {
  return
      AML_Parser_EatParenOpen(p) &&
      AML_Parser_ParseIdentifier(p, &args->name) &&
      AML_Parser_EatComma(p) &&
      AML_Parser_ParseInteger(p, &args->len) &&
      AML_Parser_EatComma(p) &&
      AML_Parser_ParseKeyword(p, &args->serialized) &&
      AML_Parser_EatParenClose(p);
}

/*
 * Push a parenthesis/brace/bracket to the stack, including a namespace.
 *
 * Return true on success, false on error.
 */
static bool AML_Stack_PushWithScope(AML_Parser* p, AML_TokenType type, AML_Token name) {
  if (p->stack.size >= AML_PARSER_STACK_SIZE) {
    p->error = "Max stack size exceeded";
    p->error2 = NULL;
    return false;
  }

  p->stack.stack[p->stack.size].name = name;
  p->stack.stack[p->stack.size].type = type;
  p->stack.stack[p->stack.size].has_name = true;
  p->stack.size++;
  return true;
}

/*
 * Push a parenthesis/brace/bracket to the stack, without a namespace.
 *
 * Return true on success, false on error.
 */
static bool AML_Stack_Push(AML_Parser* p, AML_TokenType type) {
  if (p->stack.size >= AML_PARSER_STACK_SIZE) {
    p->error = "Max stack size exceeded";
    p->error2 = NULL;
    return false;
  }

  p->stack.stack[p->stack.size].type = type;
  p->stack.stack[p->stack.size].has_name = false;
  p->stack.size++;
  return true;
}

/*
 * Pop a parenthesis/brace/bracket from the stack.
 *
 * Return true on success, false on error.
 */
static bool AML_Stack_Pop(AML_Parser* p, AML_TokenType type) {
  if (! p->stack.size) {
    p->error = "Unexpected token";
    p->error2 = AML_TokenType_ToStr(type);
    return false;
  }

  p->stack.size--; // TODO
  return true;
}

/*
 * Pop a segment from a namespace.
 */
static void AML_Namespace_Pop(AML_Namespace* ns) {
  for (; ns->size; ns->size--)
    if (ns->path[ns->size - 1] == '.')
      break;
}

/*
 * Push one ore more segments to a namespace.
 */
static void AML_Namespace_Push(AML_Namespace* ns, AML_Token* name) {
  if (name->s[0] == '\\') {
    memcpy(ns->path, name->s + 1, name->len - 1);
    ns->size = name->len - 1;
    return;
  }

  size_t i = 0;
  while (name->s[i] == '^') {
    AML_Namespace_Pop(ns);
    i++;
  }

  if (ns->size)
    ns->path[ns->size++] = '.';

  for (; i < name->len; ++i) {
    ns->path[ns->size++] = name->s[i];
  }
}

/*
 * Create a namespace from stack.
 */
static void AML_Namespace_FromStack(AML_Namespace* ns, AML_Stack* stack) {
  ns->size = 0;

  for (size_t i = 0; i < stack->size; ++i)
    if (stack->stack[i].has_name)
      AML_Namespace_Push(ns, &stack->stack[i].name);
}

static void AML_Namespace_ToStr(AML_Namespace* ns, char* out) {
  out[0] = '\\';
  memcpy(out + 1, ns->path, ns->size);
  out[ns->size + 1] = '\0';
}

static void AML_ResolveName(AML_Stack* stack, AML_Token* name, char* out) {
  AML_Namespace ns;
  AML_Namespace_FromStack(&ns, stack);
  AML_Namespace_Push(&ns, name);
  AML_Namespace_ToStr(&ns, out);
}

static bool AML_Parser_ExtractMethod(AML_Parser* p, AML_Token* name, AML_Method* out) {
  AML_ResolveName(&p->stack, name, out->name);

  size_t token_start = p->pos;
  out->tokens.data = p->tokens.data + token_start;

  if (! AML_Parser_EatBraceOpen(p))
    return false;

  size_t paren_count = 1;

  while (paren_count) {
    AML_Token t;
    if (! AML_Parser_GetRealToken(p, &t))
      return false;

    switch (t.type) {
    case AML_TOK_OP_ParenOpen: /* fall-through */
    case AML_TOK_OP_BraceOpen: /* fall-through */
    case AML_TOK_OP_BracketOpen:
      ++paren_count;
      break;

    case AML_TOK_OP_ParenClose: /* fall-through */
    case AML_TOK_OP_BraceClose: /* fall-through */
    case AML_TOK_OP_BracketClose:
      --paren_count;
      break;

    default:
      break;
    }
  }

  out->tokens.size = p->pos - token_start;
  return true;
}

/*
 * Extract all methods found in parser and append them to `out`.
 *
 * Returns error on failure.
 */
Error AML_Parser_ExtractMethods(AML_Parser* p, array_of(AML_Method)* out) {
  AML_Token t;
  union {
    AML_ScopeArgs   scope;
    AML_DeviceArgs  device;
    AML_MethodArgs  method;
    AML_ThermalZone thermal_zone;
  } args;

  while (AML_Parser_GetRealToken(p, &t)) {
    switch (t.type) {
    case AML_TOK_Keyword:
      if (AML_Token_StrEq(&t, "Method", STRLEN("Method"))) {
        out->data = Mem_Realloc(out->data, (out->size + 1) * sizeof(AML_Method));

        if (! AML_Parse_MethodArgs(p, &args.method) ||
            ! AML_Parser_ExtractMethod(p, &args.method.name, &out->data[out->size]))
        {
          goto error;
        }

        out->size++;
        continue;
      }

      if (AML_Token_StrEq(&t, "Device", STRLEN("Device"))) {
        if (! AML_Parse_DeviceArgs(p, &args.device) ||
            ! AML_Parser_EatBraceOpen(p) ||
            ! AML_Stack_PushWithScope(p, AML_TOK_OP_BraceOpen, args.device.name))
        {
          goto error;
        }

        continue;
      }

      if (AML_Token_StrEq(&t, "Scope", STRLEN("Scope"))) {
        if (! AML_Parse_ScopeArgs(p, &args.scope) ||
            ! AML_Parser_EatBraceOpen(p) ||
            ! AML_Stack_PushWithScope(p, AML_TOK_OP_BraceOpen, args.scope.name))
        {
          goto error;
        }

        continue;
      }

      if (AML_Token_StrEq(&t, "ThermalZone", STRLEN("ThermalZone"))) {
        if (! AML_Parse_ThermalZoneArgs(p, &args.thermal_zone) ||
            ! AML_Parser_EatBraceOpen(p) ||
            ! AML_Stack_PushWithScope(p, AML_TOK_OP_BraceOpen, args.thermal_zone.name))
        {
          goto error;
        }

        continue;
      }

      break;

    case AML_TOK_OP_ParenOpen: /* fall-through */
    case AML_TOK_OP_BraceOpen: /* fall-through */
    case AML_TOK_OP_BracketOpen:
      if (! AML_Stack_Push(p, t.type))
        goto error;
      break;
    case AML_TOK_OP_ParenClose: /* fall-through */
    case AML_TOK_OP_BraceClose: /* fall-through */
    case AML_TOK_OP_BracketClose:
      if (! AML_Stack_Pop(p, t.type))
        goto error;
      break;

    default:
      break;
    }
  }

  return err_success();

error:
  return AML_Parser_GetError(p);
}

Error AML_Parser_GetError(const AML_Parser* p) {
  if (p->error2)
    return err_stringf("%s: %s", p->error, p->error2);
  else
    return err_string(p->error);
}
