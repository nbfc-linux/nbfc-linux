#define CLIENT_HELP_TEXT                                                       \
 "Usage: nbfc [-h] [--version] COMMAND [...]\n"                                \
 "\n"                                                                          \
 "NoteBook FanControl CLI Client\n"                                            \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  --version             Show program's version number and exit\n"            \
 "\n"                                                                          \
 "Commands:\n"                                                                 \
 "    start               Start the service\n"                                 \
 "    stop                Stop the service\n"                                  \
 "    restart             Restart the service\n"                               \
 "    status              Show the service status\n"                           \
 "    config              List or apply configs\n"                             \
 "    set                 Control fan speed\n"                                 \
 "    wait-for-hwmon      Wait for /sys/class/hwmon/hwmon* files\n"            \
 "    get-model-name      Print out model name\n"                              \
 "    help                Show help\n"                                         \
 ""

#define CLIENT_START_HELP_TEXT                                                 \
 "Usage: nbfc start [-h] [-r]\n"                                               \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -r, --read-only       Start in read-only mode\n"                           \
 "  -h, --help            Shows this message\n"                                \
 ""

#define CLIENT_RESTART_HELP_TEXT                                               \
 "Usage: nbfc restart [-h] [-r]\n"                                             \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -r, --read-only       Restart in read-only mode\n"                         \
 "  -h, --help            Shows this message\n"                                \
 ""

#define CLIENT_CONFIG_HELP_TEXT                                                \
 "Usage: nbfc config [-h] (-l | -s config | -a config | -r)\n"                 \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -l, --list            List all available configs (default)\n"              \
 "  -s config, --set config\n"                                                 \
 "                        Set a config\n"                                      \
 "  -a config, --apply config\n"                                               \
 "                        Set a config and enable fan control\n"               \
 "  -r, --recommend       List configs which may work for your device\n"       \
 ""

#define CLIENT_STATUS_HELP_TEXT                                                \
 "Usage: nbfc status [-h] (-a | -s | -f FAN INDEX) [-w SECONDS]\n"             \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -a, --all             Show service and fan status (default)\n"             \
 "  -s, --service         Show service status\n"                               \
 "  -f FAN INDEX, --fan FAN INDEX (zero based)\n"                              \
 "                        Show fan status\n"                                   \
 "  -w SECONDS, --watch SECONDS\n"                                             \
 "                        Show status periodically\n"                          \
 ""

#define CLIENT_SET_HELP_TEXT                                                   \
 "Usage: nbfc set [-h] (-a | -s PERCENT) [-f FAN INDEX]\n"                     \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -a, --auto            Set fan speed to 'auto'\n"                           \
 "  -s PERCENT, --speed PERCENT\n"                                             \
 "                        Set fan speed to PERCENT\n"                          \
 "  -f FAN INDEX, --fan FAN INDEX\n"                                           \
 "                        Fan index (zero based)\n"                            \
 ""

#define CLIENT_DEFAULT_HELP(COMMAND)                                           \
 "Usage: nbfc " COMMAND " [-h]\n"                                              \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""
