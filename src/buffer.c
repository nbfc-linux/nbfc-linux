#include "buffer.h"

#include "memory.h"

#include <stdbool.h>

/*
 * NOTE: Static buffers are used instead of large stack allocations on purpose.
 *
 * Although stack memory is often considered "cheap", large stack frames
 * significantly increase the process' resident set size (RSS) on Linux,
 * because stack pages are faulted in eagerly and remain mapped for the
 * lifetime of the thread.
 *
 * In contrast, static (.bss / .data) buffers are backed by demand-paged
 * anonymous memory and only consume physical memory once actually touched,
 * without growing the per-thread stack. This results in a lower and more
 * predictable runtime memory footprint, especially for large temporary
 * buffers that are not used concurrently.
 */

static char Buffer_Buf[BUFFER_SIZE];
static bool Buffer_Used = false;

char* Buffer_Get() {
  if (Buffer_Used) {
    // Defensive malloc() fallback. Should never happen.
    return Mem_Malloc(BUFFER_SIZE);
  }

  Buffer_Used = true;
  return Buffer_Buf;
}

void Buffer_Release(char* buffer) {
  if (buffer == Buffer_Buf) {
    Buffer_Used = false;
  }
  else {
    Mem_Free(buffer);
  }
}
