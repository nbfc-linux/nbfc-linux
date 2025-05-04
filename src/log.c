#include "log.h"

#include "config.h"
#include "program_name.h"

#include <stdio.h>
#include <stdarg.h>
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

void Log_Close() {
#if ENABLE_SYSLOG
  if (Log_UseSyslog)
    closelog();
#endif
}

void Log_Error(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Error)
    return;

#if ENABLE_SYSLOG
  if (Log_UseSyslog) {
    va_list args;
    va_start(args, fmt);

    char buf[LOG_BUFFER_SIZE];
    vsnprintf(buf, sizeof(buf), fmt, args);
    syslog(LOG_ERR, "%s", buf);

    va_end(args);
  }
  //else
#endif
  {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s: ERROR: ", Program_Name);
    vfprintf(stderr, fmt, args);

    va_end(args);
  }
}

void Log_Warn(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Warn)
    return;


#if ENABLE_SYSLOG
  if (Log_UseSyslog) {
    va_list args;
    va_start(args, fmt);

    char buf[LOG_BUFFER_SIZE];
    vsnprintf(buf, sizeof(buf), fmt, args);
    syslog(LOG_WARNING, "%s", buf);

    va_end(args);
  }
  //else
#endif
  {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s: WARNING: ", Program_Name);
    vfprintf(stderr, fmt, args);

    va_end(args);
  }
}

void Log_Info(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Info)
    return;

#if ENABLE_SYSLOG
  if (Log_UseSyslog) {
    va_list args;
    va_start(args, fmt);

    char buf[LOG_BUFFER_SIZE];
    vsnprintf(buf, sizeof(buf), fmt, args);
    syslog(LOG_INFO, "%s", buf);

    va_end(args);
  }
  //else
#endif
  {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s: INFO: ", Program_Name);
    vfprintf(stderr, fmt, args);

    va_end(args);
  }

}

void Log_Debug(const char* fmt, ...) {
  if (Log_LogLevel < LogLevel_Debug)
    return;

#if ENABLE_SYSLOG
  if (Log_UseSyslog) {
    va_list args;
    va_start(args, fmt);

    char buf[LOG_BUFFER_SIZE];
    vsnprintf(buf, sizeof(buf), fmt, args);
    syslog(LOG_DEBUG, "%s", buf);

    va_end(args);
  }
  //else
#endif
  {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s: DEBUG: ", Program_Name);
    vfprintf(stderr, fmt, args);

    va_end(args);
  }
}
