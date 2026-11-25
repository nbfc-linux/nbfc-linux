#ifndef NBFC_LOG_H_
#define NBFC_LOG_H_

#include "macros.h"
#include "io_utils.h"

#include <stdbool.h>
#include <unistd.h> // write
#include <string.h> // strlen

enum LogLevel {
  LogLevel_Quiet,

  // Message log levels corresponding to kernel printk() log levels
  LogLevel_Error = 3,
  LogLevel_Warn = 4,
  LogLevel_Info = 6,
  LogLevel_Debug = 7,
};
typedef enum LogLevel LogLevel;

extern LogLevel Log_LogLevel;

bool Log_Init(bool);  // Returns whether to keep stderr open
void Log_Close(void);
void Log_Log(LogLevel, const char* fmt, ...) NBFC_PRINTF_LIKE(2, 3);

#define Log_Error(...) Log_Log(LogLevel_Error, __VA_ARGS__)
#define Log_Warn(...)  Log_Log(LogLevel_Warn, __VA_ARGS__)
#define Log_Info(...)  Log_Log(LogLevel_Info, __VA_ARGS__)
#define Log_Debug(...) Log_Log(LogLevel_Debug, __VA_ARGS__)

static inline void WriteToErr(const char* s) {
  IO_WriteAll(STDERR_FILENO, s, strlen(s));
}

static inline void WriteToOut(const char* s) {
  IO_WriteAll(STDOUT_FILENO, s, strlen(s));
}

#endif
