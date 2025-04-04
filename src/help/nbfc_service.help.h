#define NBFC_SERVICE_HELP_TEXT                                                 \
 "Usage: %s [-h] [-r] [-f] [-d] [-c config] [-s state.json] [-e EC]\n"         \
 "\n"                                                                          \
 "NoteBook FanControl service\n"                                               \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -r, --read-only       Start in read-only mode\n"                           \
 "  -f, --fork            Switch process to background after sucessfully started\n"\
 "  -d, --debug           Enable tracing of reads and writes of the embedded controller\n"\
 "  -c CONFIG, --config-file CONFIG\n"                                         \
 "                        Use alternative config file (default " SYSCONFDIR "/nbfc/nbfc.json)\n"\
 "  -e EC, --embedded-controller EC\n"                                         \
 "                        Specify embedded controller to use\n"                \
 ""
