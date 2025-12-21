#ifndef LOG_H_
#define LOG_H_

#include <stdbool.h>
#include <unistd.h> // write
#include <string.h> // strlen

enum LogLevel {
  LogLevel_Quiet,
  LogLevel_Error,
  LogLevel_Warn,
  LogLevel_Info,
  LogLevel_Debug
};
typedef enum LogLevel LogLevel;

extern LogLevel Log_LogLevel;

void Log_Init(bool);
void Log_Close();
void Log_Log(LogLevel, const char* fmt, ...);

#define Log_Error(...) Log_Log(LogLevel_Error, __VA_ARGS__)
#define Log_Warn(...)  Log_Log(LogLevel_Warn, __VA_ARGS__)
#define Log_Info(...)  Log_Log(LogLevel_Info, __VA_ARGS__)
#define Log_Debug(...) Log_Log(LogLevel_Debug, __VA_ARGS__)

static inline void WriteToErr(const char* s) {
  write(STDERR_FILENO, s, strlen(s));
}

#endif
