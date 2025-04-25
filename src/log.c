#include "log.h"

#include "program_name.h"

#include <stdio.h>
#include <stdarg.h>

LogLevel Log_LogLevel = LogLevel_Info;

void Log_Error(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Error)
    return;

  fprintf(stderr, "%s: ERROR: ", Program_Name);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void Log_Warn(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Warn)
    return;

  fprintf(stderr, "%s: WARNING: ", Program_Name);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void Log_Info(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Info)
    return;

  fprintf(stderr, "%s: INFO: ", Program_Name); \

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

void Log_Debug(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Debug)
    return;

  fprintf(stderr, "%s: DEBUG: ", Program_Name);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

