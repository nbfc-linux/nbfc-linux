#include "config_files.h"

#include <stdlib.h> // exit
#include <string.h> // strcmp, strrchr, strerror
#include <dirent.h> // DIR, opendir, readdir, closedir
#include <errno.h>  // errno

#include "../nbfc.h"
#include "../log.h"
#include "../memory.h"
#include "../nxjson_utils.h"
#include "dmi.h"
#include "str_functions.h"

// get an array of config names
array_of(ConfigFile) get_configs() {
  size_t capacity = 512;
  array_of(ConfigFile) files = {
    .data = Mem_Malloc(capacity * sizeof(struct ConfigFile)),
    .size = 0
  };

  DIR* directory = opendir(NBFC_MODEL_CONFIGS_DIR);
  if (!directory) {
    Log_Error("Failed to open directory `" NBFC_MODEL_CONFIGS_DIR "': %s\n", strerror(errno));
    exit(NBFC_EXIT_FAILURE);
  }

  struct dirent* file;
  while ((file = readdir(directory)) != NULL) {
    if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
      continue;

    // remove .json extension
    char* dot = strrchr(file->d_name, '.');
    if (dot)
      *dot = '\0';

    if (files.size == capacity) {
      capacity *= 2;
      files.data = Mem_Realloc(files.data, capacity * sizeof(struct ConfigFile));
    }

    files.data[files.size++].config_name = Mem_Strdup(file->d_name);
  }

  closedir(directory);
  return files;
}

// compare function for qsort
int compare_config_by_name(const void *a, const void *b) {
  return strcmp(((struct ConfigFile *)a)->config_name, ((struct ConfigFile *)b)->config_name);
}

// compare function for qsort
int compare_config_by_diff(const void *a, const void *b) {
  return (((struct ConfigFile *)b)->diff > ((struct ConfigFile *)a)->diff)
       - (((struct ConfigFile *)b)->diff < ((struct ConfigFile *)a)->diff);
}

// recommend configs in a sorted array
array_of(ConfigFile) recommended_configs() {
  const char *product = get_model_name();
  array_of(ConfigFile) files = get_configs();
  for_each_array(ConfigFile*, file, files) {
    file->diff = str_similarity(product, file->config_name);
  }
  qsort(files.data, files.size, sizeof(struct ConfigFile), compare_config_by_diff);
  return files;
}

/*
 * Retrive the supported model for `model_name`.
 *
 * This function searches the model support database (a JSON file) for the
 * given `model_name` and returns the compatible output model for it.
 *
 * The model support database is a JSON object where each key represents an
 * input model name and each value represents an output model name:
 *
 *  {
 *     "Input Model Name": "Output Model Name"
 *  }
 *
 * If the model is found in the support database, the function will check if
 * the corresponding output model exists in the provided `config_files`.
 * If a match is found, the output model is returned. Otherwise, a warning
 * is printed and `NULL` is returned.
 *
 * If the input model is not found in the support database, the function
 * attempts to find the model name directly in `config_files` and returns it if
 * a match is found.
 *
 * If there is no match, `NULL` is returned.
 */
char* get_supported_model(array_of(ConfigFile)* config_files, const char* model_name) {
  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* root = NULL;
  char* output_model = NULL;

  Error* e = nx_json_parse_file(&root, buf, sizeof(buf), NBFC_MODEL_SUPPORT_FILE);
  if (e) {
    Log_Warn("%s: %s\n", NBFC_MODEL_SUPPORT_FILE, err_print_all(e));
    goto end;
  }

  if (root->type != NX_JSON_OBJECT) {
    Log_Warn("%s: Not a JSON object\n", NBFC_MODEL_SUPPORT_FILE);
    goto end;
  }

  nx_json_for_each(model, root) {
    if (model->type != NX_JSON_STRING) {
      Log_Warn("%s: Invalid value for model `%s`: Not a string\n", NBFC_MODEL_SUPPORT_FILE, model->key);
    }
    else if (!strcmp(model->key, model_name)) {
      if (output_model) {
        Log_Warn("%s: Duplicate model key: `%s`\n", NBFC_MODEL_SUPPORT_FILE, model->key);
      }
      output_model = Mem_Strdup(model->val.text);
    }
  }

end:
  nx_json_free(root);

  if (output_model) {
    // Ensure that the model actually exists
    for_each_array(ConfigFile*, file, *config_files) {
      if (!strcmp(file->config_name, output_model)) {
        return output_model;
      }
    }

    Log_Warn("%s: The model `%s` was found in the support database, but the specified configuration file (`%s`) is missing\n",
        NBFC_MODEL_SUPPORT_FILE, model_name, output_model);

    Mem_Free(output_model);
  }
  else {
    // Not found in support database, try a direct match on `config_files`
    for_each_array(ConfigFile*, file, *config_files) {
      if (!strcmp(file->config_name, model_name)) {
        return Mem_Strdup(model_name);
      }
    }
  }

  return NULL;
}

