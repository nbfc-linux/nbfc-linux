#include "log.h"

#include "config.h"
#include "macros.h"
#include "program_name.h"

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#if ENABLE_SYSLOG
#include <syslog.h>
#endif

#define LOG_BUFFER_SIZE 4096

LogLevel Log_LogLevel = LogLevel_Info;

#if ENABLE_SYSLOG
static bool Log_UseSyslog = false;
#endif

void Log_Init(bool use_syslog) {
#if ENABLE_SYSLOG
  if (use_syslog) {
    openlog(Program_Name, LOG_PID | LOG_CONS, LOG_DAEMON);
    Log_UseSyslog = true;
  }
#endif
}

void Log_Close(void) {
#if ENABLE_SYSLOG
  if (Log_UseSyslog)
    closelog();
#endif
}

void Log_Log(LogLevel level, const char* fmt, ...) {
  if (Log_LogLevel < level)
    return;

  int errno_old = errno;
  char buf[LOG_BUFFER_SIZE];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

#if ENABLE_SYSLOG
  if (Log_UseSyslog) {
    switch (level) {
      case LogLevel_Error: syslog(LOG_ERR,     "%s", buf); break;
      case LogLevel_Warn:  syslog(LOG_WARNING, "%s", buf); break;
      case LogLevel_Info:  syslog(LOG_INFO,    "%s", buf); break;
      case LogLevel_Debug: syslog(LOG_DEBUG,   "%s", buf); break;
      default: break;
    }
  }
#endif

  switch (level) {
    case LogLevel_Error:
      IO_WriteAll(STDERR_FILENO, "ERROR", STRLEN("ERROR"));
      break;

    case LogLevel_Warn:
      IO_WriteAll(STDERR_FILENO, "WARNING", STRLEN("WARNING"));
      break;

    case LogLevel_Info:
      IO_WriteAll(STDERR_FILENO, "INFO", STRLEN("INFO"));
      break;

    case LogLevel_Debug:
      IO_WriteAll(STDERR_FILENO, "DEBUG", STRLEN("DEBUG"));
      break;

    default:
      break;
  }

  IO_WriteAll(STDERR_FILENO, ": ", 2);
  IO_WriteAll(STDERR_FILENO, Program_Name, strlen(Program_Name));
  IO_WriteAll(STDERR_FILENO, ": ", 2);
  IO_WriteAll(STDERR_FILENO, buf, strlen(buf));
  IO_WriteAll(STDERR_FILENO, "\n", 1);
  errno = errno_old;
}
