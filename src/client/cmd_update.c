#include <stdio.h>        // snprintf
#include <string.h>       // strcmp
#include <linux/limits.h> // PATH_MAX
#include <openssl/sha.h>

#include "../nbfc.h"
#include "../log.h"
#include "../macros.h"
#include "../memory.h"
#include "../file_utils.h"
#include "../nxjson_utils.h"

#include "check_root.h"
#include "client_global.h"
#include "curl_utils.h"

#define UpdateParallelDefault 10

#define UpdateConfigVersion "1.0"

#define UpdateAPIContentsURL \
  "https://api.github.com/repos/nbfc-linux/configs/contents/" UpdateConfigVersion "/configs"

#define UpdateAPIModelSupportURL \
  "https://raw.githubusercontent.com/nbfc-linux/configs/main/" UpdateConfigVersion "/model_support.json"

const cli99_option update_options[] = {
  cli99_include_options(&main_options),
  {"-p|--parallel", Option_Update_Parallel, 1},
  {"-q|--quiet",    Option_Update_Quiet,    0},
  cli99_options_end()
};

struct {
  int  parallel;
  bool quiet;
} Update_Options = {
  UpdateParallelDefault,
  false
};

// Represents if a file is up to date or needs to be downloaded
enum FileState {
  FileState_UpToDate = 0,
  FileState_New,
  FileState_Changed
};
typedef enum FileState FileState;

// Data structued used for getting file listing using GitHub's API
struct GitHubFile {
  char* name;
  char* sha;
  char* download_url;
  FileState state;
};
typedef struct GitHubFile GitHubFile;
declare_array_of(GitHubFile);

// Compute SHA1 sum of `data` with the size of `len` and store a string
// representation of the hash in `out`
static void compute_sha1(const char* data, size_t len, char* out) {
  unsigned char hash[SHA_DIGEST_LENGTH];

  SHA1((const unsigned char*) data, len, hash);

  for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
    snprintf(out + i * 2, 3, "%02x", hash[i]);
  }
}

// Checks if `path` equals `sha1sum`.
// Note: This is actually the checksum of the contents of the file including
//       a header: `blob <SIZE>\0<CONTENT>`
static bool File_Equals_Git_SHA1_Sum(const char* path, const char* sha1sum) {
  char buf[NBFC_MAX_FILE_SIZE];
  char size_plus_content[NBFC_MAX_FILE_SIZE + 64];
  char hash[SHA_DIGEST_LENGTH * 2 + 1] = {0};

  if (slurp_file(buf, sizeof(buf), path) == -1) {
    Log_Error("Error reading file: %s: %s", path, strerror(errno));
    return false;
  }

  int len = snprintf(size_plus_content, sizeof(size_plus_content), "blob %lu%c%s",
    strlen(buf), 0, buf);

  compute_sha1(size_plus_content, len, hash);

  return !strcmp(sha1sum, hash);
}

// Checks if each file in `files` is either a new file (not present in any
// configuration directory) or is different to existing files or does not
// need an update at all.
static void Files_Set_FileState(array_of(GitHubFile)* files) {
  char path[PATH_MAX];

  for_each_array(GitHubFile*, file, *files) {
    snprintf(path, sizeof(path), "%s/%s", NBFC_MODEL_CONFIGS_DIR_MUTABLE, file->name);
    if (file_exists(path)) {
      if (File_Equals_Git_SHA1_Sum(path, file->sha)) {
        file->state = FileState_UpToDate;
        continue;
      }
      else {
        file->state = FileState_Changed;
        continue;
      }
    }

    snprintf(path, sizeof(path), "%s/%s", NBFC_MODEL_CONFIGS_DIR, file->name);
    if (file_exists(path)) {
      if (File_Equals_Git_SHA1_Sum(path, file->sha)) {
        file->state = FileState_UpToDate;
        continue;
      }
      else {
        file->state = FileState_Changed;
        continue;
      }
    }

    file->state = FileState_New;
  }
}

// Print a short summary
static void Print_Summary(array_of(GitHubFile)* files) {
  int files_new = 0;
  int files_changed = 0;

  for_each_array(GitHubFile*, file, *files) {
    if (file->state == FileState_New)
      ++files_new;
    else if (file->state == FileState_Changed)
      ++files_changed;
  }

  Log_Info("New files: %d   Files changed: %d", files_new, files_changed);
}

// Iterates through an array of GitHubFile and returns a CURL instance
// for the next file that needs to be downloaded or NULL if there
// are no remaining files.
static CURL* Get_Next_Download(array_of(GitHubFile)* files, int* iter) {
next_file:
  if (*iter >= files->size)
    return NULL;

  if (files->data[*iter].state == FileState_UpToDate) {
    ++*iter;
    goto next_file;
  }

  const char* url  = files->data[*iter].download_url;
  const char* file = files->data[*iter].name;
  char path[PATH_MAX];
  snprintf(path, sizeof(path), "%s/%s", NBFC_MODEL_CONFIGS_DIR_MUTABLE, file);
  ++*iter;
  return CurlWithMem_Create(url, path);
}

static int Curl_Parallel_Download_Files(array_of(GitHubFile)* files, int parallel) {
  int ret = 0;
  int files_iter = 0;

  CURLM* multi = curl_multi_init();
  if (! multi) {
    Log_Error("curl_multi_init() failed");
    return -1;
  }

  for (int i = 0; i < parallel; ++i) {
    CURL* curl = Get_Next_Download(files, &files_iter);
    if (! curl)
      break;

    curl_multi_add_handle(multi, curl);
  }

  int still_running = 0;

  do {
    CURLMcode mc;
    int numfds;

    mc = curl_multi_perform(multi, &still_running);
    if (mc != CURLM_OK) {
      Log_Error("curl_multi_perform() failed");
      ret = -1;
      break;
    }

    mc = curl_multi_wait(multi, NULL, 0, 1000, &numfds);
    if (mc != CURLM_OK) {
      Log_Error("curl_multi_wait() failed");
      ret = -1;
      break;
    }

    CURLMsg *msg;
    int msgs_left;

    while ((msg = curl_multi_info_read(multi, &msgs_left))) {
      if (msg->msg == CURLMSG_DONE) {
        CURL *easy = msg->easy_handle;
        CURLcode code = msg->data.result;
        CurlMemory* mem;
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &mem);

        if (code != CURLE_OK) {
          Log_Download_Failed(mem->url, code);
          ret = -1;
        }
        else {
          if (! Update_Options.quiet)
            Log_Download_Finished(mem->url);

          if (CurlMemory_WriteFile(mem) == -1) {
            Log_Write_Failed(mem->path, errno);
            ret = -1;
          }
        }

        curl_multi_remove_handle(multi, easy);
        CurlWithMem_Destroy(easy);

        easy = Get_Next_Download(files, &files_iter);
        if (easy) {
          curl_multi_add_handle(multi, easy);
          ++still_running;
        }
      }
    }
  } while (still_running);

  curl_multi_cleanup(multi);
  return ret;
}

// Download `url`, parse the content as JSON and fill up the array specified
// in `out`.
//
// Return 0 on success and -1 on failure.
//
// Note: `out` has to be freed regardless of the return code.
static int GitHub_Get_Dir_Contents(const char* url, array_of(GitHubFile)* out) {
  int ret = 0;
  CURL* curl = NULL;
  char* data = NULL;
  const nx_json* root = NULL;
  ssize_t out_capacity = 512;

  out->data = Mem_Malloc(out_capacity * sizeof(GitHubFile));
  out->size = 0;

  curl = CurlWithMem_Create(url, NULL);

  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
  headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  CURLcode code = curl_easy_perform(curl);
  curl_slist_free_all(headers);

  if (code != CURLE_OK) {
    Log_Download_Failed(url, code);
    ret = -1;
    goto end;
  }

  if (! Update_Options.quiet)
    Log_Download_Finished(url);

  data = CurlWithMem_StealData(curl);
  CurlWithMem_Destroy(curl);

  root = nx_json_parse_utf8(data);

  if (! root) {
    Log_Error("Invalid JSON: %s", NX_JSON_MSGS[NX_JSON_ERROR]);
    ret = -1;
    goto end;
  }

  if (root->type != NX_JSON_ARRAY) {
    Log_Error("Received data is not a JSON array");
    ret = -1;
    goto end;
  }

  nx_json_for_each(file, root) {
    if (file->type != NX_JSON_OBJECT) {
      Log_Error("Item is not a JSON object");
      continue;
    }

    const char* name = NULL;
    const char* sha = NULL;
    const char* download_url = NULL;

    nx_json_for_each(member, file) {
      if (member->type == NX_JSON_STRING) {
        if (!strcmp(member->key, "name"))
          name = member->val.text;
        else if (!strcmp(member->key, "sha"))
          sha = member->val.text;
        else if (!strcmp(member->key, "download_url"))
          download_url = member->val.text;
      }
    }

    if (name == NULL) {
      Log_Error("Field missing: 'name'");
      continue;
    }

    if (download_url == NULL) {
      Log_Error("Field missing: 'download_url'");
      continue;
    }

    if (sha == NULL) {
      Log_Warn("Field missing: 'sha'");
      sha = "";
    }

    if (out->size == out_capacity) {
      out_capacity *= 2;
      out->data = Mem_Realloc(out->data, out_capacity * sizeof(GitHubFile));
    }

    out->data[out->size].name = Mem_Strdup(name);
    out->data[out->size].sha = Mem_Strdup(sha);
    out->data[out->size].download_url = Mem_Strdup(download_url);
    out->size++;
  }

end:
  nx_json_free(root);
  Mem_Free(data);
  return ret;
}

// Update compatibility database (model_config.json)
static int UpdateModelCompatibilityDatabase() {
  int ret = 0;
  CURL* curl = CurlWithMem_Create(UpdateAPIModelSupportURL, NBFC_MODEL_SUPPORT_FILE_MUTABLE);
  CURLcode code = curl_easy_perform(curl);

  if (code != CURLE_OK) {
    Log_Download_Failed(UpdateAPIModelSupportURL, code);
    ret = -1;
    goto end;
  }

  if (! Update_Options.quiet)
    Log_Download_Finished(UpdateAPIModelSupportURL);

  if (CurlWithMem_WriteFile(curl) == -1) {
    Log_Write_Failed(NBFC_MODEL_SUPPORT_FILE_MUTABLE, errno);
    ret = -1;
    goto end;
  }

end:
  CurlWithMem_Destroy(curl);
  return ret;
}

// Update configuration files
static int UpdateConfigurationFiles() {
  int ret = 0;
  array_of(GitHubFile) files = {0};

  // Get a list of configuration files in the GitHub repository
  if (GitHub_Get_Dir_Contents(UpdateAPIContentsURL, &files) == -1) {
    Log_Error("Failed to download configuration file list");
    ret = -1;
    goto end;
  }

  // Check which files shall be updated (setting GitHubFile->state)
  Files_Set_FileState(&files);

  // Print a summary
  Print_Summary(&files);

  // Download configuration files
  if (Curl_Parallel_Download_Files(&files, Update_Options.parallel) == -1) {
    Log_Error("Some configuration files could not be downloaded");
    ret = -1;
  }

end:
  for_each_array(GitHubFile*, file, files) {
    Mem_Free(file->name);
    Mem_Free(file->download_url);
    Mem_Free(file->sha);
  }
  Mem_Free(files.data);
  return ret;
}

int Update() {
  check_root();
  int ret = NBFC_EXIT_SUCCESS;

  if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
    Log_Error("curl_global_init() failed");
    return NBFC_EXIT_FAILURE;
  }

  Log_Info("Updating model compatibility database ...");
  if (UpdateModelCompatibilityDatabase() == -1)
    ret = NBFC_EXIT_FAILURE;

  Log_Info("Updating configuration files ...");
  if (UpdateConfigurationFiles() == -1)
    ret = NBFC_EXIT_FAILURE;
  
  curl_global_cleanup();

  return ret;
}
