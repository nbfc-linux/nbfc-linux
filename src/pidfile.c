#include "pidfile.h"

#include "nbfc.h"
#include "file_utils.h"

#include <errno.h>  // errno, EEXIST
#include <stdio.h>  // snprintf
#include <unistd.h> // getpid
#include <sys/stat.h>

Error PID_Write(enum PID_LockMode lock_mode) {
  Error e = err_success();
  const int flags = ((int) lock_mode)|O_CREAT|O_WRONLY|O_TRUNC;
  const mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%d", getpid());

  if (! write_file(NBFC_PID_FILE, flags, mode, buf, (size_t) len).ok) {
    e = err_stdlib(NBFC_PID_FILE);

    if (errno == EEXIST)
      e = err_chain_string(e, "Failed to acquire lock file");
  }

  return e;
}

void PID_Cleanup(void) {
  unlink(NBFC_PID_FILE);
}
