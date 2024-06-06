#include "pidfile.h"

#include "slurp_file.h"

#include <unistd.h> // getpid

Error* PID_Write(bool acquire) {
  int flags = 0;
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%d", getpid());

  if (acquire)
    flags = O_EXCL;

  if (write_file(NBFC_PID_FILE, flags|O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH, buf, len) == -1) {
    if (errno == EEXIST) {
      Error* e = err_string(0, "Failed to acquire lock file");
      return err_string(e, NBFC_PID_FILE);
    }

    return err_stdlib(0, NBFC_PID_FILE);
  }

  return err_success();
}

void PID_Cleanup() {
  unlink(NBFC_PID_FILE);
}

