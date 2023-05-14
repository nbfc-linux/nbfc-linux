#define EC_PROBE_HELP_TEXT "usage: %s [-h] [-e EC] {dump,read,write,monitor} ...\n"\
 "\n"\
 "Probing tool for embedded controllers\n"\
 "\n"\
 "optional arguments:\n"\
 "  -h, --help            show this help message and exit\n"\
 "  -e EC, --embedded-controller EC\n"\
 "                        Specify embedded controller to use\n"\
 "\n"\
 "subcommands:\n"\
 "  commands\n"\
 "\n"\
 "  {dump,read,write,monitor}\n"\
 "    dump                Dump all EC registers\n"\
 "    read                Read a byte from a EC register\n"\
 "    write               Write a byte to a EC register\n"\
 "    monitor             Monitor all EC registers for changes\n"\
 "\n"\
 "All input values are interpreted as decimal numbers by default. Hexadecimal values may be entered by prefixing them with \"0x\".\n"\
 ""
#define EC_PROBE_DUMP_HELP_TEXT "usage: %s [-h]\n"\
 "\n"\
 "Dump all EC registers\n"\
 "\n"\
 "optional arguments:\n"\
 "  -h, --help  show this help message and exit\n"\
 ""

#define EC_PROBE_READ_HELP_TEXT "usage: %s [-h] REGISTER\n"\
 "\n"\
 "Read a byte from a EC register\n"\
 "\n"\
 "positional arguments:\n"\
 "  REGISTER    Register source\n"\
 "\n"\
 "optional arguments:\n"\
 "  -h, --help  show this help message and exit\n"\
 "  -w, --word  read two registers as one word\n"\
 ""

#define EC_PROBE_WRITE_HELP_TEXT "usage: %s [-h] REGISTER VALUE\n"\
 "\n"\
 "Write a byte to a EC register\n"\
 "\n"\
 "positional arguments:\n"\
 "  REGISTER    Register destination\n"\
 "  VALUE       Value to write\n"\
 "\n"\
 "optional arguments:\n"\
 "  -h, --help  show this help message and exit\n"\
 "  -w, --word  write VALUE to two registers\n"\
 ""

#define EC_PROBE_MONITOR_HELP_TEXT "usage: %s [-h] [-i seconds] [-t seconds] [-r REPORT] [-c] [-d]\n"\
 "\n"\
 "Monitor all EC registers for changes\n"\
 "\n"\
 "optional arguments:\n"\
 "  -h, --help            show this help message and exit\n"\
 "  -i seconds, --interval seconds\n"\
 "                        Monitored timespan\n"\
 "  -t seconds, --timespan seconds\n"\
 "                        Set poll intervall\n"\
 "  -r REPORT, --report REPORT\n"\
 "                        Save all readings as a CSV file\n"\
 "  -c, --clearly         Blanks out consecutive duplicate readings\n"\
 "  -d, --decimal         Output readings in decimal format instead of hexadecimal format\n"\
 ""

