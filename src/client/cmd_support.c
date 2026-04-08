#include <stdio.h>  // printf, puts
#include <string.h> // strcmp, strlen
#include <stdlib.h> // exit

#include "../nbfc.h"
#include "../log.h"
#include "../memory.h"
#include "../nxjson_utils.h"

#include "check_root.h"
#include "client_global.h"
#include "curl_utils.h"

#define SUPPORT_FIRMWARE_UPLOAD_ENDPOINT_URL \
  "https://raw.githubusercontent.com/nbfc-linux/nbfc-linux/main/endpoints/firmware_upload_v1"

#define SUPPORT_PAYPAL_URL "https://paypal.me/BenjaminAbendroth"

#define SUPPORT_GITHUB_URL "https://github.com/nbfc-linux/nbfc-linux"

#define SUPPORT_ACPI_DSDT  "/sys/firmware/acpi/tables/DSDT"

#define SUPPORT_TEXT                                                           \
  "Thank you for using NBFC-Linux!\n"                                          \
  "\n"                                                                         \
  "If you'd like to support the project, you can:\n"                           \
  "\n"                                                                         \
  " - Send a donation via PayPal:\n"                                           \
  "     " SUPPORT_PAYPAL_URL "\n"                                              \
  "\n"                                                                         \
  " - Simply star the project on GitHub:\n"                                    \
  "     " SUPPORT_GITHUB_URL "\n"                                              \
  "\n"                                                                         \
  " - Upload your notebook firmware:\n"                                        \
  "     $ sudo nbfc support --upload-firmware\n"                               \
  "\n"                                                                         \
  "   Run 'nbfc support -h' for more information on firmware uploads.\n"       \
  ""

const struct cli99_Option support_options[] = {
  cli99_Options_Include(&main_options),
  {"--upload-firmware", Option_Support_Upload_Firmware, cli99_NoArgument},
  {"--print-command",   Option_Support_Print_Command,   cli99_NoArgument},
  cli99_Options_End()
};

enum Support_Action {
  Support_Action_None = 0,
  Support_Action_Upload_Firmware,
  Support_Action_Print_Command
};

struct {
  enum Support_Action action;
} Support_Options = {
  Support_Action_None
};

/**
 * Returns the real endpoint URL used for uploading the firmware.
 *
 * The actual URL is retrieved from the official GitHub repository to
 * ensure compatibility in case the upload endpoint changes.
 */
static char* Support_Get_Real_Firmware_Upload_Endpoint_URL() {
  CURL* curl = CurlWithMem_Create(SUPPORT_FIRMWARE_UPLOAD_ENDPOINT_URL, NULL);
  CURLcode code;
  long http_code;

  code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    Log_Download_Failed(SUPPORT_FIRMWARE_UPLOAD_ENDPOINT_URL, code);
    exit(NBFC_EXIT_FAILURE);
  }

  code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_getinfo() failed");
    exit(NBFC_EXIT_FAILURE);
  }

  if (http_code != 200) {
    Log_Error("Download failed: %s (server returned HTTP %ld)\n",
      SUPPORT_FIRMWARE_UPLOAD_ENDPOINT_URL, http_code);
    exit(NBFC_EXIT_FAILURE);
  }

  char* real_endpoint = CurlWithMem_StealData(curl);

  // Strip trailing whitespace
  size_t len = strlen(real_endpoint);
  while (len && real_endpoint[len] < 32)
    real_endpoint[len--] = '\0';

  CurlWithMem_Destroy(curl);
  return real_endpoint;
}

static char* Support_Do_Upload(const char* model, const char* firmware_file) {
  char* endpoint_url = NULL;
  char* response = NULL;
  CURL* curl = NULL;
  curl_mime* mime = NULL;
  curl_mimepart* part;
  CURLcode code;
  long http_code;

  // Get real endpoint URL
  endpoint_url = Support_Get_Real_Firmware_Upload_Endpoint_URL();

  // Perform upload
  curl = CurlWithMem_Create(endpoint_url, NULL);

  mime = curl_mime_init(curl);
  if (! mime) {
    Log_Error("curl_mime_init() failed");
    goto end;
  }

  part = curl_mime_addpart(mime);
  if (! part) {
    Log_Error("curl_mime_addpart() failed");
    goto end;
  }

  code = curl_mime_name(part, "model");
  if (code != CURLE_OK) {
    Log_Error("curl_mime_name() failed");
    goto end;
  }

  code = curl_mime_data(part, model, CURL_ZERO_TERMINATED);
  if (code != CURLE_OK) {
    Log_Error("curl_mime_data() failed");
    goto end;
  }

  part = curl_mime_addpart(mime);
  if (! part) {
    Log_Error("curl_mime_addpart() failed");
    goto end;
  }

  code = curl_mime_name(part, "file");
  if (code != CURLE_OK) {
    Log_Error("curl_mime_name() failed");
    goto end;
  }

  code = curl_mime_filedata(part, firmware_file);
  if (code != CURLE_OK) {
    Log_Error("curl_mime_filedata() failed");
    goto end;
  }

  code = curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_setopt(%s) failed", Curl_Get_EasyOpt_Name(CURLOPT_MIMEPOST));
    goto end;
  }

  code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    Log_Upload_Failed(endpoint_url, code);
    goto end;
  }

  code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_getinfo() failed");
    goto end;
  }

  if (http_code != 200) {
    Log_Error("Upload failed: %s (server returned HTTP %ld)\n",
      endpoint_url, http_code);
    goto end;
  }

  // Success
  response = CurlWithMem_StealData(curl);

end:
  Mem_Free(endpoint_url);
  CurlWithMem_Destroy(curl);
  return response;
}

static int Support_Handle_Response(char* response) {
  int ret = NBFC_EXIT_FAILURE;
  const nx_json* status = NULL;
  const nx_json* message = NULL;
  const nx_json* root = nx_json_parse_utf8(response);

  if (! root) {
    Log_Error("Received data is not valid JSON: %s", NX_JSON_MSGS[NX_JSON_ERROR]);
    goto end;
  }

  if (root->type != NX_JSON_OBJECT) {
    Log_Error("Received JSON is not an object");
    goto end;
  }

  nx_json_for_each(child, root) {
    if (! strcmp(child->key, "status")) {
      status = child;
      continue;
    }

    if (! strcmp(child->key, "message")) {
      message = child;
      continue;
    }
  }

  if (! status) {
    Log_Error("Missing \"status\" key in response");
    goto end;
  }

  if (! message) {
    Log_Error("Missing \"message\" key in response");
    goto end;
  }

  if (status->type != NX_JSON_STRING) {
    Log_Error("Data of \"status\" is not a string");
    goto end;
  }

  if (message->type != NX_JSON_STRING) {
    Log_Error("Data of \"message\" is not a string");
    goto end;
  }

  if (! strcmp(status->val.text, "error")) {
    Log_Error("Server returned: %s", message->val.text);
    goto end;
  }

  if (strcmp(status->val.text, "ok")) {
    Log_Error("Server returned invalid status: %s", status->val.text);
    goto end;
  }

  // Success
  printf("%s\n", message->val.text);
  ret = NBFC_EXIT_SUCCESS;

end:
  nx_json_free(root);
  return ret;
}

static int Support_Upload_Firmware() {
  // Accessing `SUPPORT_ACPI_DSDT` requires root
  check_root();

  // Do the upload
  char* response = Support_Do_Upload(DMI_Get_Model_Name(), SUPPORT_ACPI_DSDT);
  if (! response)
    return NBFC_EXIT_FAILURE;

  // Handle response
  int ret = Support_Handle_Response(response);
  Mem_Free(response);
  return ret;
}

static int Support_Print_Command() {
  char* endpoint_url = Support_Get_Real_Firmware_Upload_Endpoint_URL();

  printf(
    "Run the following command to upload your firmware:\n"
    "\n"
    "sudo curl -X POST '%s' -F 'file=@" SUPPORT_ACPI_DSDT "' -F 'model=%s'\n",
    endpoint_url,
    DMI_Get_Model_Name()
  );

  Mem_Free(endpoint_url);
  return NBFC_EXIT_SUCCESS;
}

int Support() {
  int ret = NBFC_EXIT_FAILURE;

  if (Support_Options.action == Support_Action_None) {
    puts(SUPPORT_TEXT);
    return NBFC_EXIT_SUCCESS;
  }

  if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
    Log_Error("curl_global_init() failed");
    return NBFC_EXIT_FAILURE;
  }

  if (Support_Options.action == Support_Action_Upload_Firmware)
    ret = Support_Upload_Firmware();
  else if (Support_Options.action == Support_Action_Print_Command)
    ret = Support_Print_Command();

#if STRICT_CLEANUP
  curl_global_cleanup();
#endif

  return ret;
}
