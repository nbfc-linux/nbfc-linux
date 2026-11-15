#include "log.h"

#include "config.h"
#include "macros.h"
#include "program_name.h"

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#if ENABLE_SYSLOG
#include <syslog.h>
#endif

#define LOG_BUFFER_SIZE 4096

LogLevel Log_LogLevel = LogLevel_Info;

#if ENABLE_SYSLOG
static bool Log_UseSyslog = false;
#endif
static bool Log_UseJournalStderr = false;

static bool Log_DetectJournalStream(void) {
  // Source: https://systemd.io/JOURNAL_NATIVE_PROTOCOL/#automatic-protocol-upgrading
  struct stat st;
  uintmax_t dev, ino;
  const char* journal_stream = getenv("JOURNAL_STREAM");

  if (! journal_stream)
    return false;

  if (sscanf(journal_stream, "%ju:%ju", &dev, &ino) != 2)
    return false;  // Unexpected node identification format

  if (fstat(STDERR_FILENO, &st) != 0)
    return false;

  // Verify that stderr indeed points at a stream socket of matching
  // what is encoded in the $JOURNAL_STREAM environment variable
  // (avoids problems if a systemd service spawned us as a subprocess)
  return (uintmax_t) st.st_dev == dev && (uintmax_t) st.st_ino == ino;
}

bool Log_Init(bool use_syslog) {
  if (Log_DetectJournalStream()) {
    // Journald-enhanced stderr logging
    //
    // Like `syslog` but avoids https://github.com/systemd/systemd/issues/2913.
    Log_UseJournalStderr = true;
    return true;
  }

#if ENABLE_SYSLOG
  if (use_syslog) {
    openlog(Program_Name, LOG_PID | LOG_CONS, LOG_DAEMON);
    Log_UseSyslog = true;
    return false;
  }
#endif

  // Standard stderr logging
  return true;
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

  if (Log_UseJournalStderr) {
    // Severity prefix: Levels in enum correspond to correct levels
    char severity[4];
    snprintf(severity, sizeof(severity) / sizeof(char), "<%d>", level);

    // Print message line-wise, prefixing each line with severity
    char* save_ptr = NULL;
    char* line = strtok_r(buf, "\n", &save_ptr);
    while (line) {
      WriteToErr(severity);
      WriteToErr(line);
      WriteToErr("\n");
      line = strtok_r(NULL, "\n", &save_ptr);
    }

    goto cleanup;
  }

#if ENABLE_SYSLOG
  if (Log_UseSyslog) {
    switch (level) {
      case LogLevel_Error: syslog(LOG_ERR,     "%s", buf); goto cleanup;
      case LogLevel_Warn:  syslog(LOG_WARNING, "%s", buf); goto cleanup;
      case LogLevel_Info:  syslog(LOG_INFO,    "%s", buf); goto cleanup;
      case LogLevel_Debug: syslog(LOG_DEBUG,   "%s", buf); goto cleanup;
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

cleanup:
  errno = errno_old;
}
