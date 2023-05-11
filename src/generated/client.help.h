#define CLIENT_HELP_TEXT                                                       \
  "usage: nbfc [-h] [--version]\n"                                             \
  "            {start,stop,restart,status,config,set,wait-for-hwmon,help} "    \
  "...\n"                                                                      \
  "\n"                                                                         \
  "NoteBook FanControl CLI Client\n"                                           \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -h, --help            show this help message and exit\n"                  \
  "  --version             show program's version number and exit\n"           \
  "\n"                                                                         \
  "subcommands:\n"                                                             \
  "  commands\n"                                                               \
  "\n"                                                                         \
  "  {start,stop,restart,status,config,set,wait-for-hwmon,help}\n"             \
  "    start               Start the service\n"                                \
  "    stop                Stop the service\n"                                 \
  "    restart             Restart the service\n"                              \
  "    status              Show the service status\n"                          \
  "    config              List or apply configs\n"                            \
  "    set                 Control fan speed\n"                                \
  "    wait-for-hwmon      Wait for /sys/class/hwmon/hwmon* files\n"           \
  "    help                Show help\n"                                        \
  ""

#define CLIENT_START_HELP_TEXT                                                 \
  "usage: nbfc start [-h] [-r]\n"                                              \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -r, --readonly        Start in read-only mode\n"                          \
  "  -h, --help            Shows this message\n"                               \
  ""

#define CLIENT_RESTART_HELP_TEXT                                               \
  "usage: nbfc restart [-h] [-r]\n"                                            \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -r, --readonly        Restart in read-only mode\n"                        \
  "  -h, --help            Shows this message\n"                               \
  ""

#define CLIENT_CONFIG_HELP_TEXT                                                \
  "usage: nbfc config [-h] (-l | -s config | -a config | -r)\n"                \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -h, --help            show this help message and exit\n"                  \
  "  -l, --list            List all available configs (default)\n"             \
  "  -s config, --set config\n"                                                \
  "                        Set a config\n"                                     \
  "  -a config, --apply config\n"                                              \
  "                        Set a config and enable fan control\n"              \
  "  -r, --recommend       List configs which may work for your device\n"      \
  ""

#define CLIENT_STATUS_HELP_TEXT                                                \
  "usage: nbfc status [-h] (-a | -s | -f FAN INDEX) [-w SECONDS]\n"            \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -h, --help            show this help message and exit\n"                  \
  "  -a, --all             Show service and fan status (default)\n"            \
  "  -s, --service         Show service status\n"                              \
  "  -f FAN INDEX, --fan FAN INDEX (zero based)\n"                             \
  "                        Show fan status\n"                                  \
  "  -w SECONDS, --watch SECONDS\n"                                            \
  "                        Show status periodically\n"                         \
  ""

#define CLIENT_SET_HELP_TEXT                                                   \
  "usage: nbfc set [-h] (-a | -s PERCENT) [-f FAN INDEX]\n"                    \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -h, --help            show this help message and exit\n"                  \
  "  -a, --auto            Set fan speed to 'auto'\n"                          \
  "  -s PERCENT, --speed PERCENT\n"                                            \
  "                        Set fan speed to PERCENT\n"                         \
  "  -f FAN INDEX, --fan FAN INDEX\n"                                          \
  "                        Fan index (zero based)\n"                           \
  ""

#define CLIENT_DEFAULT_HELP(X)                                                 \
  "usage: nbfc " X " [-h]\n"                                                   \
  "\n"                                                                         \
  "optional arguments:\n"                                                      \
  "  -h, --help            Shows this message\n"                               \
  ""
