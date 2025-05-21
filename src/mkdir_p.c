#include "mkdir_p.h"

#include <errno.h>     // errno, EEXIST
#include <sys/stat.h>  // mkdir

#include "memory.h"

int mkdir_p(const char *path, mode_t mode) {
  char *tmp = Mem_Strdup(path);
  char *p = tmp;
  int ret = 0;

  if (tmp[0] == '/')
    p++;

  while (*p) {
    if (*p == '/') {
      *p = '\0';
      if (mkdir(tmp, mode) != 0) {
        if (errno != EEXIST) {
          ret = -1;
          break;
        }
      }
      *p = '/';
    }
    p++;
  }

  if (ret == 0 && mkdir(tmp, mode) != 0 && errno != EEXIST)
    ret = -1;

  Mem_Free(tmp);
  return ret;
}
