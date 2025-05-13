#include "program_name.h"

const char* Program_Name = "unkown";

void Program_Name_Set(const char* path) {
  for (const char* c = path; *c; ++c)
    if (*c == '/' && *(c + 1))
      path = c + 1;

  Program_Name = path;
}
