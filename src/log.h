#ifndef LOG_H_
#define LOG_H_

#include <stdbool.h>

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
void Log_Error(const char* fmt, ...);
void Log_Warn(const char* fmt, ...);
void Log_Info(const char* fmt, ...);
void Log_Debug(const char* fmt, ...);

#endif
