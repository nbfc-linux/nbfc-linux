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
 "    sensors             Show or configure temperature sensors\n"             \
 "    config              List or apply configs\n"                             \
 "    set                 Control fan speed\n"                                 \
 "    update              Download new configuration files\n"                  \
 "    wait-for-hwmon      Wait for /sys/class/hwmon/hwmon* files\n"            \
 "    get-model-name      Print out model name\n"                              \
 "    acpi-dump           Dumps information of your ACPI DSDT\n"               \
 "    rate-config         Rate a configuration file\n"                         \
 "    help                Show help\n"                                         \
 "    support             Show how to support the project\n"                   \
 "    warranty            Show warranty\n"                                     \
 ""

#define CLIENT_START_HELP_TEXT                                                 \
 "Usage: nbfc start [-h] [-r]\n"                                               \
 "\n"                                                                          \
 "Start the NBFC service.\n"                                                   \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -r, --read-only       Start in read-only mode\n"                           \
 "  -h, --help            Shows this message\n"                                \
 ""

#define CLIENT_RESTART_HELP_TEXT                                               \
 "Usage: nbfc restart [-h] [-r]\n"                                             \
 "\n"                                                                          \
 "Restart the NBFC service.\n"                                                 \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -r, --read-only       Restart in read-only mode\n"                         \
 "  -h, --help            Shows this message\n"                                \
 ""

#define CLIENT_CONFIG_HELP_TEXT                                                \
 "Usage: nbfc config [-h] (-l | -s config | -a config | -r)\n"                 \
 "\n"                                                                          \
 "Set or list configurations for the NBFC service.\n"                          \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -l, --list            List all available configs\n"                        \
 "  -s CONFIG, --set CONFIG\n"                                                 \
 "                        Set a config\n"                                      \
 "  -a CONFIG, --apply CONFIG\n"                                               \
 "                        Set a config and start the service\n"                \
 "  -r, --recommend       List configs which may work for your device\n"       \
 "\n"                                                                          \
 "If CONFIG is \"auto\", the service will attempt to automatically select\n"   \
 "a matching configuration.\n"                                                 \
 ""

#define CLIENT_RATE_CONFIG_HELP_TEXT                                           \
 "Usage: nbfc rate-config [-h|-H] [-d|--dsdt=FILE] [-j] (-a | <FILE>)\n"       \
 "\n"                                                                          \
 "Rates a configuration by analyzing whether it appears safe to execute\n"     \
 "on the current system.\n"                                                    \
 "\n"                                                                          \
 "Please run `nbfc rate-config --full-help` for a full explanation of how\n"   \
 "to interpret these results.\n"                                               \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -H, --full-help       Show full help\n"                                    \
 "  -a, --all             Rate all available configuration files\n"            \
 "  -d, --dsdt FILE       Use an alternative DSDT file\n"                      \
 "  -j, --json            Use JSON output\n"                                   \
 "  -m, --min-score SCORE Set minimum rating threshold for configurations\n"   \
 "  -n, --no-download     Don't download rules from the repository\n"          \
 "  -r, --rules FILE      Use an alternative rules file\n"                     \
 "  --print-rules         Print configuration rating rules\n"                  \
 ""

#define CLIENT_ACPI_DUMP_HELP_TEXT                                             \
 "Usage: nbfc acpi-dump [-h] [-j|--json] [-f|--file=FILE] <COMMAND>\n"         \
 "\n"                                                                          \
 "Dumps information of your ACPI DSDT.\n"                                      \
 "\n"                                                                          \
 "Commands:\n"                                                                 \
 "    registers           List all available registers\n"                      \
 "    ec-registers        List all available EC registers\n"                   \
 "    methods             List all available ACPI methods\n"                   \
 "    dsl                 Disassemble your DSDT\n"                             \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -f, --file FILE       Use an alternative DSDT file\n"                      \
 "  -j, --json            Use JSON output\n"                                   \
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
 "Configure fan sensors.\n"                                                    \
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
 "\n"                                                                          \
 "If -f|--fan is not given, apply speed to all available fans.\n"              \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  -h, --help            Show this help message and exit\n"                   \
 "  -a, --auto            Set fan speed to profile-based control\n"            \
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

#define CLIENT_SHOW_VARIABLE_HELP_TEXT                                         \
 "Usage: nbfc show-variable [-h] VARIABLE\n"                                   \
 "\n"                                                                          \
 "Print out a compile time variable.\n"                                        \
 "\n"                                                                          \
 "Available variables:\n"                                                      \
 "  - config_file\n"                                                           \
 "  - socket_file\n"                                                           \
 "  - pid_file\n"                                                              \
 "  - model_configs_dir\n"                                                     \
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

#define CLIENT_SUPPORT_HELP_TEXT                                               \
 "Usage: nbfc support [-h] [--upload-firmware] [--print-command]\n"            \
 "\n"                                                                          \
 "Displays information on how to support the project.\n"                       \
 "\n"                                                                          \
 "Optional arguments:\n"                                                       \
 "  --upload-firmware     Upload your notebook firmware without prompting\n"   \
 "  --print-command       Print command for manual firmware upload\n"          \
 "  -h, --help            Shows this message and exit\n"                       \
 "\n"                                                                          \
 "Uploading your notebook firmware helps the developer to create new\n"        \
 "configuration files.\n"\
 "\n"                                                                          \
 "The following data will be transmitted:\n"                                   \
 "- The laptop manufacturer (/sys/devices/virtual/dmi/id/sys_vendor)\n"        \
 "- The laptop model (/sys/devices/virtual/dmi/id/product_name)\n"             \
 "- The ACPI DSDT firmware table (/sys/firmware/acpi/tables/DSDT)\n"           \
 "\n"                                                                          \
 "The preferred method for uploading your notebook firmware is to use:\n"      \
 "  $ sudo nbfc support --upload-firmware\n"                                   \
 "\n"                                                                          \
 "However, if you're paranoid, you can upload it manually using curl:\n"       \
 "  $ nbfc support --print-command\n"                                          \
 "\n"                                                                          \
 ""
