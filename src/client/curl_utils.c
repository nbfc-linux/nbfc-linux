#include "curl_utils.h"

#include <string.h>   // memcpy
#include <stdlib.h>   // exit
#include <fcntl.h>    // O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> // S_IRUSR, S_IRGRP, S_IROTH etc.

#include "../log.h"
#include "../memory.h"
#include "../file_utils.h"

/**
 * Return the name of a CURLoption as string.
 */
const char* Curl_Get_EasyOpt_Name(CURLoption id) {
  const struct curl_easyoption* opt = curl_easy_option_by_id(id);
  return opt ? opt->name : "?";
}

/**
 * Write callback used by curl_easy_perform() (CURLOPT_WRITEFUNCTION).
 * Uses CurlMemory as the clientp buffer, stores received data in `mem->data`,
 * and appends a trailing NUL terminator for safe string handling.
 */
static size_t Curl_Write_Memory_Callback(char* data, size_t size, size_t nmemb, void* clientp)
{
  const size_t realsize = size * nmemb;
  CurlMemory* mem = (CurlMemory*) clientp;

  mem->data = Mem_Realloc(mem->data, mem->size + realsize + 1);
  memcpy(&(mem->data[mem->size]), data, realsize);
  mem->size += realsize;
  mem->data[mem->size] = '\0';

  return realsize;
}

/**
 * Creates and initializes a CURL handle together with a CurlMemory attached
 * to it.
 */
CURL* CurlWithMem_Create(const char* url, const char* path) {
  CURL* curl;
  CURLcode code;
  CurlMemory* mem;

  curl = curl_easy_init();
  if (! curl) {
    Log_Error("curl_easy_init() failed");
    exit(NBFC_EXIT_FAILURE);
  }

  mem = Mem_Calloc(1, sizeof(*mem));
  mem->url = Mem_Strdup(url);
  if (path)
    mem->path = Mem_Strdup(path);

  code = curl_easy_setopt(curl, CURLOPT_URL, url);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_setopt(%s) failed", Curl_Get_EasyOpt_Name(CURLOPT_URL));
    exit(NBFC_EXIT_FAILURE);
  }

  code = curl_easy_setopt(curl, CURLOPT_USERAGENT, UserAgent);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_setopt(%s) failed", Curl_Get_EasyOpt_Name(CURLOPT_USERAGENT));
    exit(NBFC_EXIT_FAILURE);
  }

  code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Curl_Write_Memory_Callback);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_setopt(%s) failed", Curl_Get_EasyOpt_Name(CURLOPT_WRITEFUNCTION));
    exit(NBFC_EXIT_FAILURE);
  }

  code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) mem);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_setopt(%s) failed", Curl_Get_EasyOpt_Name(CURLOPT_WRITEDATA));
    exit(NBFC_EXIT_FAILURE);
  }

  code = curl_easy_setopt(curl, CURLOPT_PRIVATE, (void*) mem);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_setopt(%s) failed", Curl_Get_EasyOpt_Name(CURLOPT_PRIVATE));
    exit(NBFC_EXIT_FAILURE);
  }

  return curl;
}

/**
 * Destroy a CURL instance and free the attached CurlMemory.
 */
void CurlWithMem_Destroy(CURL* curl) {
  if (! curl)
    return;

  CurlMemory* mem;
  curl_easy_getinfo(curl, CURLINFO_PRIVATE, &mem);

  Mem_Free(mem->data);
  Mem_Free(mem->url);
  Mem_Free(mem->path);
  Mem_Free(mem);

  curl_easy_cleanup(curl);
}

/**
 * Writes the contents of a CurlMemory buffer to the file specified in the
 * CurlMemory structure.
 */
static int CurlMemory_WriteFile(const CurlMemory* mem) {
  const int open_flags = O_WRONLY|O_CREAT|O_TRUNC;
  const int mode_flags = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
  return write_file(mem->path, open_flags, mode_flags, mem->data, mem->size);
}

/**
 * Writes the contents of the CurlMemory buffer associated with a CURL easy
 * handle to its configured file path. The CurlMemory object is retrieved
 * from the CURL instance via CURLOPT_PRIVATE.
 */
int CurlWithMem_WriteFile(CURL* curl) {
  CurlMemory* mem;
  curl_easy_getinfo(curl, CURLINFO_PRIVATE, &mem);
  return CurlMemory_WriteFile(mem);
}
