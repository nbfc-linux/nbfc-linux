#include "process.h"

#include "memory.h"

#include <errno.h>    // errno
#include <fcntl.h>
#include <unistd.h>   // read, pipe, close, fork, dup2, execvp, _exit, faccessat
#include <string.h>   // memcpy, strchr
#include <stdlib.h>   // getenv
#include <sys/wait.h> // waitpid, WIFEXITED, WIFSIGNALED, WEXITSTATUS, WTERMSIG
#include <poll.h>     // poll
#include <signal.h>   // SIGKILL

static int Process_Append(char** out, size_t* size,
                          const char* data, size_t len)
{
  char* tmp = Mem_Realloc(*out, *size + len + 1);
  memcpy(tmp + *size, data, len);
  *size += len;
  tmp[*size] = '\0';
  *out = tmp;
  return 0;
}

int Process_Capture(
    const char* file,
    char* const argv[],
    char** out_stdout,
    char** out_stderr)
{
  int out_pipe[2], err_pipe[2];
  char* stdout_data = NULL;
  char* stderr_data = NULL;
  size_t stdout_size = 0, stderr_size = 0;
  pid_t pid;
  int status;
  int result = -1;

  *out_stdout = NULL;
  *out_stderr = NULL;

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
    if (dup2(out_pipe[1], STDOUT_FILENO) < 0 ||
        dup2(err_pipe[1], STDERR_FILENO) < 0)
      _exit(126);

    close(out_pipe[0]);
    close(out_pipe[1]);
    close(err_pipe[0]);
    close(err_pipe[1]);

    execvp(file, argv);
    _exit(127);
  }

  close(out_pipe[1]);
  close(err_pipe[1]);

  int out_fd = out_pipe[0];
  int err_fd = err_pipe[0];

  int flags = fcntl(out_fd, F_GETFL);
  if (flags < 0 || fcntl(out_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    goto parent_fail;

  flags = fcntl(err_fd, F_GETFL);
  if (flags < 0 || fcntl(err_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    goto parent_fail;

  struct pollfd fds[2] = {
    { .fd = out_fd, .events = POLLIN },
    { .fd = err_fd, .events = POLLIN }
  };

  char buf[4096];
  int open_pipes = 2;

  while (open_pipes > 0) {
    int n;
    do {
      n = poll(fds, 2, -1);
    } while (n < 0 && errno == EINTR);

    if (n < 0)
      goto parent_fail;

    for (int i = 0; i < 2; ++i) {
      if (fds[i].fd < 0 ||
          !(fds[i].revents & (POLLIN | POLLHUP | POLLERR)))
        continue;

      for (;;) {
        ssize_t r = read(fds[i].fd, buf, sizeof(buf));

        if (r > 0) {
          char** data = i == 0 ? &stdout_data : &stderr_data;
          size_t* size = i == 0 ? &stdout_size : &stderr_size;

          if (Process_Append(data, size, buf, (size_t)r) < 0)
            goto parent_fail;

          continue;
        }

        if (r < 0 && errno == EINTR)
          continue;

        if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
          break;

        if (r < 0)
          goto parent_fail;

        close(fds[i].fd);
        fds[i].fd = -1;
        --open_pipes;
        break;
      }
    }
  }

  close(out_fd);
  close(err_fd);

  while (waitpid(pid, &status, 0) < 0) {
    if (errno != EINTR)
      goto cleanup;
  }

  *out_stdout = stdout_data;
  *out_stderr = stderr_data;
  stdout_data = stderr_data = NULL;

  if (WIFEXITED(status))
    result = WEXITSTATUS(status);
  else if (WIFSIGNALED(status))
    result = 128 + WTERMSIG(status);

  goto cleanup;

parent_fail:
  kill(pid, SIGKILL);
  close(out_fd);
  close(err_fd);
  while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {}

cleanup:
  free(stdout_data);
  free(stderr_data);
  return result;

fail:
  close(out_pipe[0]);
  close(out_pipe[1]);
  close(err_pipe[0]);
  close(err_pipe[1]);
  return -1;
}

bool Process_HasProgram(const char* cmd) {
  const char* path = getenv("PATH");
  if (!path)
    return false;

  bool found = false;
  char* path_copy = Mem_Strdup(path);

  for (char* dir = path_copy;;) {
    char* separator = strchr(dir, ':');

    if (separator)
      *separator = '\0';

    int fd = open((*dir ? dir : "."), O_RDONLY | O_DIRECTORY);
    if (fd >= 0) {
      found = faccessat(fd, cmd, X_OK, 0) == 0;
      close(fd);
    }

    if (found || !separator)
      break;

    dir = separator + 1;
  }

  Mem_Free(path_copy);
  return found;
}
