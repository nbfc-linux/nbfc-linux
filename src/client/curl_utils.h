#ifndef NBFC_CURL_UTILS_H
#define NBFC_CURL_UTILS_H

#include <string.h> // strerror

#include <curl/curl.h>

#include "../log.h"
#include "../nbfc.h"

#define UserAgent \
  "NBFC-Linux/" NBFC_VERSION " libcurl/" LIBCURL_VERSION " (+https://github.com/nbfc-linux)"

// Data structure for receiving content via `Curl_Write_Memory_Callback`
struct CurlMemory {
  char*  data;
  size_t size;
  char*  url;
  char*  path; // (optional) stores a file path
};
typedef struct CurlMemory CurlMemory;

CURL* CurlWithMem_Create(const char* url, const char* path);
void  CurlWithMem_Destroy(CURL*);
int   CurlWithMem_WriteFile(CURL*);

static inline char* CurlWithMem_StealData(CURL* curl) {
  CurlMemory* mem;
  char* data;
  curl_easy_getinfo(curl, CURLINFO_PRIVATE, &mem);
  data = mem->data;
  mem->data = NULL;
  return data;
}

static inline void Log_Download_Finished(const char* url) {
  Log_Info("Finished downloading %s", url);
}

static inline void Log_Download_Failed(const char* url, CURLcode ret) {
  Log_Error("Download failed: %s (%s)", url, curl_easy_strerror(ret));
}

static inline void Log_Upload_Failed(const char* url, CURLcode ret) {
  Log_Error("Upload failed: %s (%s)", url, curl_easy_strerror(ret));
}

static inline void Log_Write_Failed(const char* path, int err) {
  Log_Error("Write failed: %s: %s", path, strerror(err));
}

#endif
