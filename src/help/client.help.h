#define CLIENT_HELP_HELP_TEXT                                                  \
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
 "    update              Download new configuration files\n"                  \
 "    wait-for-hwmon      Wait for /sys/class/hwmon/hwmon* files\n"            \
 "    get-model-name      Print out model name\n"                              \
 "    help                Show help\n"                                         \
 "    donate              Show how to support the project\n"                   \
 "    warranty            Show warranty\n"                                     \
 ""

#define CLIENT_START_HELP_TEXT                                                 \
 "Usage: nbfc start [-h] [-r]\n"                                               \
 "\n"                                                                          \
 "Start the NBFC service\n"                                                    \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -r, --read-only       Start in read-only mode\n"                           \
 "  -h, --help            Shows this message\n"                                \
 ""

#define CLIENT_RESTART_HELP_TEXT                                               \
 "Usage: nbfc restart [-h] [-r]\n"                                             \
 "\n"                                                                          \
 "Restart the NBFC service\n"                                                  \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -r, --read-only       Restart in read-only mode\n"                         \
 "  -h, --help            Shows this message\n"                                \
 ""

#define CLIENT_CONFIG_HELP_TEXT                                                \
 "Usage: nbfc config [-h] (-l | -s config | -a config | -r)\n"                 \
 "\n"                                                                          \
 "Set or list configurations for the NBFC service\n"                           \
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
 "Show status about the NBFC service.\n"                                       \
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

#define CLIENT_SENSORS_HELP_TEXT                                               \
 "Usage: nbfc sensors (list | set | show) [OPTIONS...]\n"                      \
 "\n"                                                                          \
 "Configure fan sensors\n"                                                     \
 "\n"                                                                          \
 "  list\n"                                                                    \
 "    List all available sensors and their temperature files.\n"               \
 "\n"                                                                          \
 "  show\n"                                                                    \
 "    Show all available fans and their sensor configuration.\n"               \
 "\n"                                                                          \
 "  set -f FAN INDEX [-s SENSOR...] [-a ALGORITHM]\n"                          \
 "    Configure sensors and algorithm for a fan.\n"                            \
 "\n"                                                                          \
 "      -f FAN INDEX, --fan FAN INDEX\n"                                       \
 "                        Fan to configure\n"                                  \
 "      -s SENSOR, --sensor SENSOR\n"                                          \
 "                        Sensor to add. Can be specified multiple times\n"    \
 "      -a ALGORITHM, --algorithm ALGORITHM\n"                                 \
 "                        Algorithm (Average, Min, Max)\n"                     \
 "      --force\n"                                                             \
 "                        Force applying sensors if not found\n"               \
 "\n"                                                                          \
 "    If no sensors are specified, the sensors in the fan config will be\n"    \
 "    cleared and defaults will be used.\n"                                    \
 "\n"                                                                          \
 "    If no algorithm is specified, the algorithm in the fan config will be\n" \
 "    cleared and the default will be used.\n"                                 \
 ""

#define CLIENT_SET_HELP_TEXT                                                   \
 "Usage: nbfc set [-h] (-a | -s PERCENT) [-f FAN INDEX]\n"                     \
 "\n"                                                                          \
 "Set the speed of fans.\n"                                                    \
 "If -f|--fan is not given, apply speed to all available fans.\n"              \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -a, --auto            Set fan speed to 'auto'\n"                           \
 "  -s PERCENT, --speed PERCENT\n"                                             \
 "                        Set fan speed to PERCENT\n"                          \
 "  -f FAN INDEX, --fan FAN INDEX\n"                                           \
 "                        Fan index (zero based)\n"                            \
 ""

#define CLIENT_STOP_HELP_TEXT                                                  \
 "Usage: nbfc stop [-h]\n"                                                     \
 "\n"                                                                          \
 "Stop the NBFC service.\n"                                                    \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

#define CLIENT_UPDATE_HELP_TEXT                                                \
 "Usage: nbfc update [-h] [-p NUM] [-q]\n"                                     \
 "\n"                                                                          \
 "Update the available configuration files and the model support database.\n"  \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -p, --parallel NUM    Set the number of parallel downloads\n"              \
 "  -q, --quiet           Quiet mode\n"                                        \
 ""

#define CLIENT_WAIT_FOR_HWMON_HELP_TEXT                                        \
 "Usage: nbfc wait-for-hwmon [-h]\n"                                           \
 "\n"                                                                          \
 "Wait until files in /sys/class/hwmon are populated.\n"                       \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

#define CLIENT_GET_MODEL_HELP_TEXT                                             \
 "Usage: nbfc get-model-name [-h]\n"                                           \
 "\n"                                                                          \
 "Print out model name.\n"                                                     \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

// TODO
#define CLIENT_SHOW_VARIABLE_HELP_TEXT                                         \
 "Usage: nbfc show-variable [-h] VARIABLE\n"                                   \
 "\n"                                                                          \
 "Print out a variable.\n"                                                     \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

#define CLIENT_COMPLETE_FANS_HELP_TEXT                                         \
 "Usage: nbfc complete-fans [-h]\n"                                            \
 "\n"                                                                          \
 "Used for completing shell command lines.\n"                                  \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

#define CLIENT_COMPLETE_SENSORS_HELP_TEXT                                      \
 "Usage: nbfc complete-sensors [-h]\n"                                         \
 "\n"                                                                          \
 "Used for completing shell command lines.\n"                                  \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

#define CLIENT_WARRANTY_HELP_TEXT                                              \
 "Usage: nbfc warranty [-h]\n"                                                 \
 "\n"                                                                          \
 "Print legal disclaimer and warranty info.\n"                                 \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""

#define CLIENT_DONATE_HELP_TEXT                                                \
 "Usage: nbfc donate [-h]\n"                                                   \
 "\n"                                                                          \
 "Displays information on how to support the project through a donation.\n"    \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Shows this message and exit\n"                       \
 ""
