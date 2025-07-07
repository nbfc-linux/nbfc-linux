#define EC_PROBE_HELP_TEXT                                                     \
 "Usage: %s [-h] [-e EC] COMMAND [...]\n"                                      \
 "\n"                                                                          \
 "Probing tool for the embedded controller\n"                                  \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -e EC, --embedded-controller EC\n"                                         \
 "                        Specify embedded controller to use\n"                \
 "\n"                                                                          \
 "Commands:\n"                                                                 \
 "  dump                  Dump all EC registers\n"                             \
 "  load                  Load a previously made dump\n"                       \
 "  read                  Read a byte from a EC register\n"                    \
 "  write                 Write a byte to a EC register\n"                     \
 "  monitor               Monitor all EC registers for changes\n"              \
 "  watch                 Monitor all EC registers for changes (alternative version)\n"\
 "  acpi_call             Call an ACPI method\n"                               \
 "\n"                                                                          \
 "All input values are interpreted as decimal numbers by default. Hexadecimal values may be entered by prefixing them with \"0x\".\n"\
 ""

#define EC_PROBE_DUMP_HELP_TEXT                                                \
 "Usage: %s dump [-h]\n"                                                       \
 "\n"                                                                          \
 "Dump all EC registers\n"                                                     \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help  Show this help message and exit\n"                             \
 ""

#define EC_PROBE_LOAD_HELP_TEXT                                                \
 "Usage: %s load [-h] FILE\n"                                                  \
 "\n"                                                                          \
 "Load a dump and write it to the EC registers\n"                              \
 "\n"                                                                          \
 "Positional arguments:\n"                                                     \
 "  FILE        Dump file\n"                                                   \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help  Show this help message and exit\n"                             \
 ""

#define EC_PROBE_READ_HELP_TEXT                                                \
 "Usage: %s read [-h] REGISTER\n"                                              \
 "\n"                                                                          \
 "Read a byte from a EC register\n"                                            \
 "\n"                                                                          \
 "Positional arguments:\n"                                                     \
 "  REGISTER    Register source\n"                                             \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help  Show this help message and exit\n"                             \
 "  -w, --word  Read two registers as one word\n"                              \
 "\n"                                                                          \
 "All input values are interpreted as decimal numbers by default. Hexadecimal values may be entered by prefixing them with \"0x\".\n"\
 ""

#define EC_PROBE_WRITE_HELP_TEXT                                               \
 "Usage: %s write [-h] REGISTER VALUE\n"                                       \
 "\n"                                                                          \
 "Write a byte to a EC register\n"                                             \
 "\n"                                                                          \
 "Positional arguments:\n"                                                     \
 "  REGISTER    Register destination\n"                                        \
 "  VALUE       Value to write\n"                                              \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help  Show this help message and exit\n"                             \
 "  -w, --word  Write VALUE to two registers\n"                                \
 "\n"                                                                          \
 "All input values are interpreted as decimal numbers by default. Hexadecimal values may be entered by prefixing them with \"0x\".\n"\
 ""

#define EC_PROBE_MONITOR_HELP_TEXT                                             \
 "Usage: %s monitor [-h] [-i seconds] [-t seconds] [-r FILE] [-c] [-d]\n"      \
 "\n"                                                                          \
 "Monitor all EC registers for changes\n"                                      \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -i seconds, --interval SECONDS\n"                                          \
 "                        Sets the update interval in seconds\n"               \
 "  -t seconds, --timespan SECONDS\n"                                          \
 "                        Sets how many seconds the program will run\n"        \
 "  -r FILE, --report FILE\n"                                                  \
 "                        Save all readings as a CSV file\n"                   \
 "  -c, --clearly         Blanks out consecutive duplicate readings\n"         \
 "  -d, --decimal         Output readings in decimal format instead of hexadecimal format\n"\
 ""

#define EC_PROBE_WATCH_HELP_TEXT                                               \
 "Usage: %s watch [-h] [-i seconds] [-t seconds]\n"                            \
 "\n"                                                                          \
 "Monitor all EC registers for changes (alternative version)\n"                \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -i seconds, --interval SECONDS\n"                                          \
 "                        Sets the update interval in seconds\n"               \
 "  -t seconds, --timespan SECONDS\n"                                          \
 "                        Sets how many seconds the program will run\n"        \
 ""

#define EC_PROBE_ACPI_CALL_HELP_TEXT                                           \
 "Usage: %s acpi_call [-h] METHOD [ARGUMENT...]\n"                             \
 "\n"                                                                          \
 "Call an ACPI method.\n"                                                      \
 "\n"                                                                          \
 "The maximum number of arguments is 8.\n"                                     \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 ""

#define EC_PROBE_SHELL_HELP_TEXT                                               \
 "Usage: %s shell [-h]\n"                                                      \
 "\n"                                                                          \
 "Read commands from STDIN\n"                                                  \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 ""
