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

// Check if `files` contains a config named `name`
bool Contains_Config(array_of(ConfigFile)* files, const char* name) {
  for_each_array(ConfigFile*, file, *files) {
    if (! strcmp(file->config_name, name))
      return true;
  }

  return false;
}

// Free an array of ConfigFile
void ConfigFiles_Free(array_of(ConfigFile)* files) {
  for_each_array(ConfigFile*, file, *files)
    Mem_Free(file->config_name);
  Mem_Free(files->data);
}

// Compare function for qsort
int compare_config_by_name(const void *a, const void *b) {
  return strcmp(((struct ConfigFile *)a)->config_name, ((struct ConfigFile *)b)->config_name);
}

// Compare function for qsort
int compare_config_by_diff(const void *a, const void *b) {
  return (((struct ConfigFile *)b)->diff > ((struct ConfigFile *)a)->diff)
       - (((struct ConfigFile *)b)->diff < ((struct ConfigFile *)a)->diff);
}

// Return an array of ConfigFile for each file in `path`
static array_of(ConfigFile) List_Configs_In_Directory(const char* path) {
  ssize_t capacity = 512;
  array_of(ConfigFile) files = {
    .data = Mem_Malloc(capacity * sizeof(ConfigFile)),
    .size = 0
  };

  DIR* directory = opendir(path);
  if (!directory) {
    Log_Error("Failed to open directory `%s': %s\n", path, strerror(errno));
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
      files.data = Mem_Realloc(files.data, capacity * sizeof(ConfigFile));
    }

    files.data[files.size++].config_name = Mem_Strdup(file->d_name);
  }

  closedir(directory);
  return files;
}

// Merges two arrays of ConfigFile into a single array,
// removing duplicates based on `config_name`.
static array_of(ConfigFile) Merge_Configs(array_of(ConfigFile)* a, array_of(ConfigFile)* b) {
  array_of(ConfigFile) files = {
    .data = Mem_Malloc((a->size + b->size) * sizeof(ConfigFile)),
    .size = 0
  };

  for_each_array(ConfigFile*, file, *a) {
    files.data[files.size++].config_name = Mem_Strdup(file->config_name);
  }

  for_each_array(ConfigFile*, file, *b) {
    if (! Contains_Config(&files, file->config_name)) {
      files.data[files.size++].config_name = Mem_Strdup(file->config_name);
    }
  }

  return files;
}

// List all configs (in the static config directory as well as in the mutable config directory).
array_of(ConfigFile) List_All_Configs() {
  array_of(ConfigFile) a = {0};
  array_of(ConfigFile) b = {0};

  a = List_Configs_In_Directory(NBFC_MODEL_CONFIGS_DIR);

  if (access(NBFC_MODEL_SUPPORT_FILE_MUTABLE, F_OK) == 0)
    b = List_Configs_In_Directory(NBFC_MODEL_CONFIGS_DIR_MUTABLE);
  else
    return a;

  array_of(ConfigFile) c = Merge_Configs(&a, &b);
  ConfigFiles_Free(&a);
  ConfigFiles_Free(&b);
  return c;
}

// List all configs (in the static config directory as well as in the mutable config directory).
// The `diff` field of the ConfigFile structure will also be set.
array_of(ConfigFile) List_Recommended_Configs() {
  const char* model_name = DMI_Get_Model_Name();
  array_of(ConfigFile) files = List_All_Configs();
  for_each_array(ConfigFile*, file, files) {
    file->diff = str_similarity(model_name, file->config_name);
  }
  qsort(files.data, files.size, sizeof(struct ConfigFile), compare_config_by_diff);
  return files;
}

/*
 * Retrive the supported config for `model_name`.
 *
 * This function searches a model support database (a JSON file) for the
 * given `model_name` and returns the compatible output config for it.
 *
 * The model support database is a JSON object where each key represents an
 * input model name and each value represents an output model name:
 *
 *  {
 *     "Input Model Name": "Output Config"
 *  }
 *
 * If the model is found in the support database, the function will check if
 * the corresponding output config exists in the provided `config_files`.
 * If a match is found, the output config is returned. Otherwise, a warning
 * is printed and `NULL` is returned.
 *
 * If the input model is not found in the support database, the function
 * attempts to find the config directly in `config_files` and returns it if
 * a match is found.
 *
 * If there is no match, `NULL` is returned.
 */
char* Get_Supported_Config_From_SupportFile(const char* support_file, array_of(ConfigFile)* config_files, const char* model_name) {
  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* root = NULL;
  char* config = NULL;

  Error* e = nx_json_parse_file(&root, buf, sizeof(buf), support_file);
  if (e) {
    Log_Warn("%s: %s\n", support_file, err_print_all(e));
    goto end;
  }

  if (root->type != NX_JSON_OBJECT) {
    Log_Warn("%s: Not a JSON object\n", support_file);
    goto end;
  }

  nx_json_for_each(model, root) {
    if (model->type != NX_JSON_STRING) {
      Log_Warn("%s: Invalid value for model `%s`: Not a string\n", support_file, model->key);
    }
    else if (!strcmp(model->key, model_name)) {
      if (config) {
        Log_Warn("%s: Duplicate model key: `%s`\n", support_file, model->key);
      }
      config = Mem_Strdup(model->val.text);
    }
  }

end:
  nx_json_free(root);

  if (config) {
    // Ensure that the model actually exists
    for_each_array(ConfigFile*, file, *config_files) {
      if (!strcmp(file->config_name, config)) {
        return config;
      }
    }

    Log_Warn("%s: The model `%s` was found in the support database, but the specified configuration file (`%s`) is missing\n",
        support_file, model_name, config);

    Mem_Free(config);
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

char* Get_Supported_Config(array_of(ConfigFile)* files, const char* model) {
  char* config = NULL;

  if (access(NBFC_MODEL_SUPPORT_FILE_MUTABLE, F_OK) == 0)
    config = Get_Supported_Config_From_SupportFile(NBFC_MODEL_SUPPORT_FILE_MUTABLE, files, model);

  if (! config)
    config = Get_Supported_Config_From_SupportFile(NBFC_MODEL_SUPPORT_FILE, files, model);

  return config;
}
