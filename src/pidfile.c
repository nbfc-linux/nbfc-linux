#include "pidfile.h"

#include "nbfc.h"
#include "file_utils.h"

#include <errno.h>  // errno, EEXIST
#include <stdio.h>  // snprintf
#include <unistd.h> // getpid
#include <sys/stat.h>

Error* PID_Write(bool acquire_lock) {
  Error* e = NULL;
  int flags = 0;
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%d", getpid());

  if (acquire_lock)
    flags = O_EXCL;

  if (write_file(NBFC_PID_FILE, flags|O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH, buf, len) == -1) {
    e = err_stdlib(e, NBFC_PID_FILE);

    if (errno == EEXIST)
      e = err_string(e, "Failed to acquire lock file");
  }

  return e;
}

void PID_Cleanup() {
  unlink(NBFC_PID_FILE);
}
