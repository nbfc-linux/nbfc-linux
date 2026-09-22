#include "stress.h"

#include "log.h"
#include "nbfc.h"
#include "macros.h"
#include "file_utils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STRESS_BIN          "stress-ng"
#define STRESS_PROC_CPUINFO "/proc/cpuinfo"

typedef enum {
  Stress_Type_CPU,
  Stress_Type_GPU,
} Stress_Type;

static pid_t Stress_CPU_PID = -1;
static pid_t Stress_GPU_PID = -1;

/*
 * Return the number of processors.
 *
 * Returns 0 on failure.
 */
size_t Stress_GetNumProcessors(void) {
  char content[NBFC_MAX_FILE_SIZE];
  size_t num = 0;

  content[0] = '\n';

  FileResult res = File_Read(content + 1, sizeof(content) - 1, STRESS_PROC_CPUINFO);
  if (! res.ok)
    return 0;

  const char* p = content;
  while ((p = strstr(p, "processor"))) {
    num += (p[-1] == '\n');
    p += STRLEN("processor");
  }

  return num;
}

Error Stress_IsInstalled(void) {
  if (system("type " STRESS_BIN " >/dev/null 2>/dev/null") == 0)
    return err_success();

  errno = ENOENT;
  return err_stdlib(STRESS_BIN);
}

static Error Stress_Start(pid_t *pid, Stress_Type type, size_t workers) {
  char workers_str[32];
  pid_t parent_pid;
  pid_t child;

  if (*pid != -1)
    return err_string("Stress already running");

  snprintf(workers_str, sizeof(workers_str), "%zu", workers);

  parent_pid = getpid();
  child = fork();

  if (child == -1)
    return err_stdlib("fork()");

  if (child == 0) {
    // Make stress-ng terminate if the parent process dies.
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1) {
      Log_Error("prctl(): %s", strerror(errno));
      _exit(1);
    }

    // Avoid the race where the parent dies between fork() and
    // PR_SET_PDEATHSIG.
    if (getppid() != parent_pid) {
      Log_Error("getppid() != parent_pid");
      _exit(1);
    }

    if (type == Stress_Type_CPU)
      execlp(STRESS_BIN, STRESS_BIN,
          "--cpu", workers_str,
          "--cpu-load", "100",
          "--timeout", "0",
          "--quiet",
          NULL);

    if (type == Stress_Type_GPU)
      execlp(STRESS_BIN, STRESS_BIN,
          "--gpu", workers_str,
          "--timeout", "0",
          "--quiet",
          NULL);

    Log_Error("execlp(): %s", strerror(errno));
    _exit(1);
  }

  *pid = child;
  return err_success();
}

static void Stress_End(pid_t *pid) {
  pid_t child;

  if (*pid == -1)
    return;

  child = *pid;
  *pid = -1;

  // SIGTERM allows stress-ng to clean up its workers.
  if (kill(child, SIGTERM) == -1 && errno != ESRCH)
    return;

  // Reap the child so that it does not become a zombie.
  while (waitpid(child, NULL, 0) == -1) {
    if (errno != EINTR)
      break;
  }
}

Error Stress_CPU_Start(size_t workers)
{
  return Stress_Start(&Stress_CPU_PID, Stress_Type_CPU, workers);
}

void Stress_CPU_End(void)
{
  Stress_End(&Stress_CPU_PID);
}

Error Stress_GPU_Start(size_t workers)
{
  return Stress_Start(&Stress_GPU_PID, Stress_Type_GPU, workers);
}

void Stress_GPU_End(void)
{
  Stress_End(&Stress_GPU_PID);
}
