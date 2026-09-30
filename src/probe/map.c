#include "../file_utils.h"
#include "../memory.h"

#include <string.h> // strcmp

typedef struct {
  char    name[5];
  uint8_t addr;
} Map_Entry;
declare_array_of(Map_Entry);

static struct {
  array_of(Map_Entry) registers;
  array_size_t        registers_capacity;
  array_of(str)       methods;
  array_size_t        methods_capacity;
  bool                loaded;
} Map;

static Error Map_AddRegister(const char* name, uint8_t addr) {
  for_each_array(const Map_Entry*, entry, Map.registers) {
    if (! strcmp(entry->name, name))
      return err_stringf("Duplicate register name: %s", name);
  }

  if (Map.registers.size + 1 > Map.registers_capacity) {
    Map.registers_capacity += 256;
    array_realloc(Map_Entry, Map.registers, Map.registers_capacity);
  }

  snprintf(Map.registers.data[Map.registers.size].name, sizeof(Map.registers.data[0].name), "%s", name);
  Map.registers.data[Map.registers.size].addr = addr;
  Map.registers.size++;
  return err_success();
}

static Error Map_ParseMethodLine(const char* line) {
  if (Map.methods.size + 1 > Map.methods_capacity) {
    Map.methods_capacity += 1024;
    array_realloc(str, Map.methods, Map.methods_capacity);
  }

  Map.methods.data[Map.methods.size++] = Mem_Strdup(line);
  return err_success();
}

static Error Map_ParseRegisterLine(const char* line) {
  char register_name[5] = {0};
  const char* p = line;
  size_t n = 0;

  for (; n < 4; ++n, ++p) {
    const int ch = (unsigned char) *p;
    if (*p == '=' || ! *p)
      break;
    if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') && (ch < '0' || ch > '9') && ch != '_')
      return err_stringf("Invalid register name: %s", line);
    register_name[n] = *p;
  }

  if (n == 4 && *p != '=')
    return err_stringf("Name too long (max. 4 chars): %s", line);
  if (! n)
    return err_stringf("Empty register name: %s", line);
  if (*p != '=')
    return err_stringf("Expected NAME=ADDR: %s", line);

  ++p;

  const char* err;
  const int64_t a = parse_number(p, 0, 255, &err);
  if (err)
    return err_stringf("%s: %s", err, line);
  return Map_AddRegister(register_name, (uint8_t) a);
}

static Error Map_Load(const char* file) {
  Error e = err_success();
  char* content;

  if (Map.loaded)
    return err_success();

  const FileResult res = File_ReadDynamic(&content, file);
  if (! res.ok)
    return err_stdlib(NULL);

  for (char* p = content; p < content + res.len; ++p) {
    bool has_equal = false;
    char* line = p;
    for (;;) {
      switch (*p) {
        case '=':
          has_equal = true;
          break;
        case '\n':
          *p = '\0'; /* fall-through */
        case '\0':
          if (p > line) {
            e = has_equal ? Map_ParseRegisterLine(line) : Map_ParseMethodLine(line);
            if (e)
              goto error;
          }
      }
      if (*p == '\0')
        break;
      ++p;
    }
  }

  Map.loaded = true;

error:
  Mem_Free(content);
  return e;
}

static bool Map_LookupRegister(const char* name, uint8_t* out) {
  for_each_array(const Map_Entry*, entry, Map.registers) {
    if (! strcmp(entry->name, name)) {
      *out = entry->addr;
      return true;
    }
  }
  return false;
}
