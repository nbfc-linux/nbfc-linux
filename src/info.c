#include "info.h"

#include "nbfc.h"
#include "memory.h"
#include "error.h"
#include "stringbuf.h"
#include "macros.h"

#include <unistd.h>   // getpid
#include <sys/stat.h> // chmod

static const char* Info_File;

static const char* Json_EscapeString(char*, size_t, const char*);

Error* Info_Init(const char* file) {
  Info_File = file;

  int fd = open(Info_File, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (fd < 0)
    return err_stdlib(0, Info_File);

  close(fd);
  chmod(Info_File, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  return err_success();
}

void Info_Close() {
  unlink(NBFC_PID_FILE);
}

Error* Info_Write(ModelConfig* cfg, float temperature, bool readonly, array_of(Fan)* fans) {
  static const char Bool_ToStr[2][6] = {"false","true"};
  const pid_t pid = getpid();
  char buf[256];
  char result[4096];
  StringBuf  S = {result, 0, sizeof(result) - 1};
  StringBuf* s = &S;

  int fd = open(NBFC_PID_FILE, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (fd >= 0) {
    const int len = snprintf(buf, sizeof(buf), "%d", pid);
    write(fd, buf, len);
    close(fd);
  }

  StringBuf_Printf(s, "{\n"
    "\t\"pid\":         %d,\n"
    "\t\"config\":      \"%s\",\n"
    "\t\"readonly\":    %s,\n"
    "\t\"temperature\": %.2f,\n"
    "\t\"fans\": [\n",
    pid,
    Json_EscapeString(buf, sizeof(buf), cfg->NotebookModel),
    Bool_ToStr[readonly],
    temperature);

  size_t i = 0;
  for_each_array(Fan*, fan, *fans) {
    StringBuf_Printf(s, "\t\t{\n"
      "\t\t\t\"name\":          \"%s\",\n"
      "\t\t\t\"automode\":      %s,\n"
      "\t\t\t\"critical\":      %s,\n"
      "\t\t\t\"current_speed\": %.2f,\n"
      "\t\t\t\"target_speed\":  %.2f,\n"
      "\t\t\t\"speed_steps\":   %d\n"
      "\t\t}%c\n",
      Json_EscapeString(buf, sizeof(buf), fan->fanConfig->FanDisplayName),
      Bool_ToStr[fan->mode == Fan_ModeAuto],
      Bool_ToStr[fan->isCritical],
      Fan_GetCurrentSpeed(fan),
      Fan_GetTargetSpeed(fan),
      Fan_GetSpeedSteps(fan),
      (++i != fans->size ? ',' : ' ')
    );
  }

  StringBuf_Printf(s, "\t\t]\n}\n");
  if (! s->capacity)
    return (errno = ENOBUFS), err_stdlib(0, Info_File);

  fd = open(Info_File, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (fd < 0)
    return err_stdlib(0, Info_File);

  int nwritten = write(fd, s->s, s->size);
  close(fd);
  if (nwritten < 0)
    return err_stdlib(0, Info_File);

  return err_success();
}

// ", \, and control codes (anything less than U+0020).
static const char* Json_EscapeString(char* buf, const size_t n, const char* s) {
  size_t i = 0;
  for (; *s && i < n - 6 - 1; ++s)
    if (*s == '"' || *s == '\\' || *s < 0x20) {
      snprintf(&buf[i], 7, "\\u%.4X", *s);
      i += 6;
    }
    else
      buf[i++] = *s;
  buf[i] = '\0';
  return buf;
}

