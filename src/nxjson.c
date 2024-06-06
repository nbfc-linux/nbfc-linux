/*
 * Copyright (c) 2013 Yaroslav Stavnichiy <yarosla@gmail.com>
 *
 * This file is part of NXJSON.
 *
 * NXJSON is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * NXJSON is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NXJSON. If not, see <http://www.gnu.org/licenses/>.
 */

// this file can be #included in your code
#ifndef NXJSON_C
#define NXJSON_C

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "nxjson.h"

// redefine NX_JSON_CALLOC & NX_JSON_FREE to use custom allocator
#ifndef NX_JSON_CALLOC
#define NX_JSON_CALLOC(SIZE) calloc(1, SIZE)
#define NX_JSON_FREE(json)   free((void*)(json))
#endif

static inline nx_json* nx_calloc(nx_json_type type) {
  const size_t NX_JSON_BASE_SIZE = 3 * sizeof(void*);
  switch (type) {
  case NX_JSON_NULL:   return NX_JSON_CALLOC(NX_JSON_BASE_SIZE);
  case NX_JSON_OBJECT:
  case NX_JSON_ARRAY:  return NX_JSON_CALLOC(sizeof(nx_json));
  default:
                       return NX_JSON_CALLOC(NX_JSON_BASE_SIZE + sizeof(double));
  }
}

int                NX_JSON_SRC_LINE;
enum nx_json_error NX_JSON_ERROR;
const char*        NX_JSON_STRING_POS;

#define NX_JSON_Callback(_, MSG) MSG,
const char* NX_JSON_MSGS[NX_JSON_ERR_INVALID_NUMBER + 1] = {
  NX_JSON_FOREACH_ERRORS(NX_JSON_Callback)
};
#undef  NX_JSON_Callback

#define NX_JSON_REPORT_ERROR(E, S) do {\
  NX_JSON_SRC_LINE   = __LINE__;\
  NX_JSON_ERROR      = NX_JSON_ERR_ ## E;\
  NX_JSON_STRING_POS = (S);\
} while(0)

#define IS_WHITESPACE(c) ((unsigned char)(c)<=(unsigned char)' ')

nx_json *create_json(nx_json_type type, const char *key, nx_json *parent) {
	nx_json *js = nx_calloc(type);
	assert(js);
	js->type = type;
	js->key = key;
	if (!parent->val.children.last) {
		parent->val.children.first = parent->val.children.last = js;
	} else {
		parent->val.children.last->next = js;
		parent->val.children.last = js;
	}
	parent->val.children.length++;
	return js;
}

void nx_json_free(const nx_json *js) {
	if (!js) {
		return;
	}
	if (js->type == NX_JSON_OBJECT || js->type == NX_JSON_ARRAY) {
		nx_json *p = js->val.children.first;
		nx_json *p1;
		while (p) {
			p1 = p->next;
			nx_json_free (p);
			p = p1;
		}
	}
	NX_JSON_FREE(js);
}

static int unicode_to_utf8(unsigned int codepoint, char *p, char **endp) {
	// code from http://stackoverflow.com/a/4609989/697313
	if (codepoint < 0x80) *p++ = codepoint;
	else if (codepoint < 0x800) *p++ = 192 + codepoint / 64, *p++ = 128 + codepoint % 64;
	else if (codepoint - 0xd800u < 0x800) return 0; // surrogate must have been treated earlier
	else if (codepoint < 0x10000)
		*p++ = 224 + codepoint / 4096, *p++ = 128 + codepoint / 64 % 64, *p++ = 128 + codepoint % 64;
	else if (codepoint < 0x110000)
		*p++ = 240 + codepoint / 262144, *p++ = 128 + codepoint / 4096 % 64, *p++ = 128 + codepoint / 64 % 64, *p++ =
				128 + codepoint % 64;
	else return 0; // error
	*endp = p;
	return 1;
}

nx_json_unicode_encoder nx_json_unicode_to_utf8 = unicode_to_utf8;

static inline int hex_val(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

static char *unescape_string(char *s, char **end, nx_json_unicode_encoder encoder) {
	char *p = s;
	char *d = s;
	char c;
	while ((c = *p++)) {
		if (c == '"') {
			*d = '\0';
			*end = p;
			return s;
		} else if (c == '\\') {
			switch (*p) {
			case '\\':
			case '/':
			case '"':
				*d++ = *p++;
				break;
			case 'b':
				*d++ = '\b';
				p++;
				break;
			case 'f':
				*d++ = '\f';
				p++;
				break;
			case 'n':
				*d++ = '\n';
				p++;
				break;
			case 'r':
				*d++ = '\r';
				p++;
				break;
			case 't':
				*d++ = '\t';
				p++;
				break;
			case 'u': { // unicode
				if (!encoder) {
					// leave untouched
					*d++ = c;
					break;
				}
				char *ps = p - 1;
				int h1, h2, h3, h4;
				if ((h1 = hex_val (p[1])) < 0 || (h2 = hex_val (p[2])) < 0 || (h3 = hex_val (p[3])) < 0 ||
					(h4 = hex_val (p[4])) < 0) {
					NX_JSON_REPORT_ERROR(INVALID_UNICODE_ESCAPE, p - 1);
					return 0;
				}
				unsigned int codepoint = h1 << 12 | h2 << 8 | h3 << 4 | h4;
				if ((codepoint & 0xfc00) == 0xd800) { // high surrogate; need one more unicode to succeed
					p += 6;
					if (p[-1] != '\\' || *p != 'u' || (h1 = hex_val (p[1])) < 0 || (h2 = hex_val (p[2])) < 0 ||
						(h3 = hex_val (p[3])) < 0 || (h4 = hex_val (p[4])) < 0) {
						NX_JSON_REPORT_ERROR(INVALID_UNICODE_SURROGATE, ps);
						return 0;
					}
					unsigned int codepoint2 = h1 << 12 | h2 << 8 | h3 << 4 | h4;
					if ((codepoint2 & 0xfc00) != 0xdc00) {
						NX_JSON_REPORT_ERROR(INVALID_UNICODE_SURROGATE, ps);
						return 0;
					}
					codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (codepoint2 - 0xdc00);
				}
				if (!encoder (codepoint, d, &d)) {
					NX_JSON_REPORT_ERROR(INVALID_CODEPOINT, ps);
					return 0;
				}
				p += 5;
				break;
      }
			default:
				// leave untouched
				*d++ = c;
				break;
			}
		} else {
			*d++ = c;
		}
	}
	NX_JSON_REPORT_ERROR(MISSING_DOUBLE_EQUOTE, s);
	return 0;
}

static char *skip_block_comment(char *p) {
	// assume p[-2]=='/' && p[-1]=='*'
	char *ps = p - 2;
	if (!*p) {
		NX_JSON_REPORT_ERROR(ENDLESS_COMMENT, ps);
		return 0;
	}
	REPEAT:
	p = strchr (p + 1, '/');
	if (!p) {
		NX_JSON_REPORT_ERROR(ENDLESS_COMMENT, ps);
		return 0;
	}
	if (p[-1] != '*') goto REPEAT;
	return p + 1;
}

static char *parse_key(const char **key, char *p, nx_json_unicode_encoder encoder) {
	// on '}' return with *p=='}'
	char c;
	while ((c = *p++)) {
		if (c == '"') {
			*key = unescape_string (p, &p, encoder);
			if (!*key) return 0; // propagate error
			while (*p && IS_WHITESPACE(*p)) p++;
			if (*p == ':') return p + 1;
			NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p);
			return 0;
		} else if (IS_WHITESPACE(c) || c == ',') {
			// continue
		} else if (c == '}') {
			return p - 1;
		} else if (c == '/') {
			if (*p == '/') { // line comment
				char *ps = p - 1;
				p = strchr (p + 1, '\n');
				if (!p) {
					NX_JSON_REPORT_ERROR(ENDLESS_COMMENT, ps);
					return 0; // error
				}
				p++;
			} else if (*p == '*') { // block comment
				p = skip_block_comment (p + 1);
				if (!p) return 0;
			} else {
				NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p - 1);
				return 0; // error
			}
		} else {
			NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p - 1);
			return 0; // error
		}
	}
	NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p - 1);
	return 0; // error
}

static char *parse_value(nx_json *parent, const char *key, char *p, nx_json_unicode_encoder encoder) {
	nx_json *js;
	while (1) {
		switch (*p) {
		case '\0':
			NX_JSON_REPORT_ERROR(UNEXPECTED_EOT, p);
			return 0; // error
		case ' ':
		case '\t':
		case '\n':
		case '\r':
		case ',':
			// skip
			p++;
			break;
		case '{':
			js = create_json (NX_JSON_OBJECT, key, parent);
			p++;
			while (1) {
				const char *new_key = NULL;
				p = parse_key (&new_key, p, encoder);
				if (!p) return 0; // error
				if (*p == '}') return p + 1; // end of object
				p = parse_value (js, new_key, p, encoder);
				if (!p) return 0; // error
			}
		case '[':
			js = create_json (NX_JSON_ARRAY, key, parent);
			p++;
			while (1) {
				p = parse_value (js, 0, p, encoder);
				if (!p) return 0; // error
				if (*p == ']') return p + 1; // end of array
			}
		case ']':
			return p;
		case '"':
			p++;
			js = create_json (NX_JSON_STRING, key, parent);
			js->val.text = unescape_string (p, &p, encoder);
			if (!js->val.text) return 0; // propagate error
			return p;
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			js = create_json (NX_JSON_INTEGER, key, parent);
			char *pe;
			errno = 0;
			js->val.i = (nxjson_s64) strtoll (p, &pe, 0);
			if (pe == p || errno == ERANGE) {
				NX_JSON_REPORT_ERROR(INVALID_NUMBER, p);
				return 0; // error
			}
			if (*pe == '.' || *pe == 'e' || *pe == 'E') { // double value
				errno = 0;
				js->type = NX_JSON_DOUBLE;
				js->val.dbl = strtod (p, &pe);
				if (pe == p || errno == ERANGE) {
					NX_JSON_REPORT_ERROR(INVALID_NUMBER, p);
					return 0; // error
				}
			}
			return pe;
		}
		case 't':
			if (!strncmp (p, "true", 4)) {
				js = create_json (NX_JSON_BOOL, key, parent);
				js->val.u = 1;
				return p + 4;
			}
			NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p);
			return 0; // error
		case 'f':
			if (!strncmp (p, "false", 5)) {
				js = create_json (NX_JSON_BOOL, key, parent);
				js->val.u = 0;
				return p + 5;
			}
			NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p);
			return 0; // error
		case 'n':
			if (!strncmp (p, "null", 4)) {
				create_json (NX_JSON_NULL, key, parent);
				return p + 4;
			}
			NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p);
			return 0; // error
		case '/': // comment
			if (p[1] == '/') { // line comment
				char *ps = p;
				p = strchr(p + 2, '\n');
				if (!p) {
					NX_JSON_REPORT_ERROR(ENDLESS_COMMENT, ps);
					return 0; // error
				}
				p++;
			} else if (p[1] == '*') { // block comment
				p = skip_block_comment (p + 2);
				if (!p) return 0;
			} else {
				NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p);
				return 0; // error
			}
			break;
		default:
			NX_JSON_REPORT_ERROR(UNEXPECTED_CHARS, p);
			return 0; // error
		}
	}
}

const nx_json *nx_json_parse_utf8(char *text) {
	return nx_json_parse (text, unicode_to_utf8);
}

const nx_json *nx_json_parse(char *text, nx_json_unicode_encoder encoder) {
	nx_json js; // = {0};
  memset(&js, 0, sizeof(js));
	if (!parse_value (&js, 0, text, encoder)) {
		if (js.val.children.first) nx_json_free (js.val.children.first);
		return 0;
	}
	return js.val.children.first;
}

const nx_json *nx_json_get(const nx_json *json, const char *key) {
	nx_json *js;
	for (js = json->val.children.first; js; js = js->next) {
		if (js->key && !strcmp (js->key, key)) return js;
	}
	return NULL;
}

const nx_json *nx_json_item(const nx_json *json, int idx) {
	nx_json *js;
	for (js = json->val.children.first; js; js = js->next) {
		if (!idx--) return js;
	}
	return NULL;
}


#ifdef  __cplusplus
}
#endif

#endif  /* NXJSON_C */
