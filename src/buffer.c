#include "buffer.h"

#include "memory.h"

#include <sys/mman.h>

/*
 * Why mmap() is used instead of stack or static allocations:
 *
 * - Stack allocations permanently increase the maximum stack usage of the
 *   process, even if the memory is only needed temporarily. Large stack
 *   frames can force the kernel to commit additional stack pages and raise
 *   the process' resident memory footprint.
 *
 * - Static/global allocations live for the entire lifetime of the process.
 *   Even though they may be lazily paged in, they reserve address space
 *   unconditionally and cannot be released back to the operating system.
 *
 * - mmap() provides on-demand, page-granular memory:
 *   Physical memory is only committed when the pages are actually touched.
 *   Memory can be fully returned to the kernel via munmap().
 *   Large temporary buffers do not permanently inflate RSS or stack usage.
 *
 * Using mmap() for large or temporary allocations keeps memory usage low,
 * avoids unnecessary stack growth, and allows the kernel to reclaim memory
 * when it is no longer needed.
 */

char* Buffer_Get(size_t size) {
  void* p = mmap(
    NULL,
    size,
    PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
  );

  if (p == MAP_FAILED)
    Memory_FatalError();

  return (char*) p;
}

void Buffer_Release(char* buffer, size_t size) {
  munmap(buffer, size);
}
