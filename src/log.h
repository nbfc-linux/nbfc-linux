#include "program_name.h"

#define Log_Debug(...) do {\
  fprintf(stderr, "%s: DEBUG: ", Program_Name); \
  fprintf(stderr, __VA_ARGS__); \
} while (0)

#define Log_Info(...) do {\
  fprintf(stderr, "%s: INFO: ", Program_Name); \
  fprintf(stderr, __VA_ARGS__); \
} while (0)

#define Log_Warn(...) do {\
  fprintf(stderr, "%s: WARNING: ", Program_Name); \
  fprintf(stderr, __VA_ARGS__); \
} while (0)

#define Log_Error(...) do {\
  fprintf(stderr, "%s: ERROR: ", Program_Name); \
  fprintf(stderr, __VA_ARGS__); \
} while (0)
