#include "process.h"

#include "memory.h"

#include <errno.h>    // errno
#include <unistd.h>   // read, pipe, close, fork, dup2, execvp, _exit
#include <string.h>   // memcpy
#include <sys/wait.h> // waitpid, WIFEXITED, WIFSIGNALED, WEXITSTATUS, WTERMSIG

static char* Process_ReadAllFromFD(int fd)
{
  char buf[4096];
  size_t size = 0;
  char* out = NULL;

  for (;;) {
    ssize_t r = read(fd, buf, sizeof(buf));
    if (r <= 0)
      break;

    out = Mem_Realloc(out, size + r + 1);
    memcpy(out + size, buf, r);
    size += r;
  }

  if (out)
    out[size] = '\0';

  return out;
}

int Process_Capture(
    const char* file,
    char* const argv[],
    char** out_stdout,
    char** out_stderr)
{
  int out_pipe[2], err_pipe[2];
  pid_t pid;
  int status;

  if (pipe(out_pipe) < 0)
    return -1;
  if (pipe(err_pipe) < 0) {
    close(out_pipe[0]);
    close(out_pipe[1]);
    return -1;
  }

  pid = fork();
  if (pid < 0)
    goto fail;

  if (pid == 0) {
    dup2(out_pipe[1], STDOUT_FILENO);
    dup2(err_pipe[1], STDERR_FILENO);

    close(out_pipe[0]);
    close(out_pipe[1]);
    close(err_pipe[0]);
    close(err_pipe[1]);

    execvp(file, argv);
    _exit(127);
  }

  close(out_pipe[1]);
  close(err_pipe[1]);

  *out_stdout = Process_ReadAllFromFD(out_pipe[0]);
  *out_stderr = Process_ReadAllFromFD(err_pipe[0]);

  close(out_pipe[0]);
  close(err_pipe[0]);

  waitpid(pid, &status, 0);

  if (WIFEXITED(status))
    return WEXITSTATUS(status);

  if (WIFSIGNALED(status))
    return 128 + WTERMSIG(status);

  return -1;

fail:
  close(out_pipe[0]);
  close(out_pipe[1]);
  close(err_pipe[0]);
  close(err_pipe[1]);
  return -1;
}
