#include "../aml_analysis.c"

#include <stdio.h>

int main(int argc, const char** argv) {
  Error e;
  AML_Analysis analysis;
  array_of(str) aml_files;
  const char* s;
  bool res;

  if (argc != 2) {
    printf("%s: Expected one argument\n", argv[0]);
    return 0;
  }

  e = Acpi_Analysis_Get_All_AML_Files(&aml_files);
  if (e) {
    printf("%s", err_print_all(e));
    return 1;
  }

  e = AML_Analysis_Init(&analysis, &aml_files);
  if (e) {
    printf("%s", err_print_all(e));
    return 2;
  }

  AML_Analysis_MatchFingerprint(&analysis, argv[1], &res);
  if (! res) {
    printf("No match: %s\n", argv[1]);
    AML_Analysis_Free(&analysis);
    return 1;
  }
  else {
    printf("Match: %s\n", argv[1]);
    AML_Analysis_Free(&analysis);
    return 0;
  }
}

#include "../error.c"
#include "../memory.c"
#include "../file_utils.c"
#include "../regex_utils.c"
#include "../process.c"
#include "../nxjson.c"
#include "../aml_lexer.c"
#include "../aml_parser.c"
#include "../acpi_analysis.c"
