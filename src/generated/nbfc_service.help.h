#define NBFC_SERVICE_HELP_TEXT "usage: %s [-h] [-r] [-f] [-d] [-c config] [-s state.json] [-e EC]\n"\
 "\n"\
 "NoteBook FanControl service\n"\
 "\n"\
 "optional arguments:\n"\
 "  -h, --help            show this help message and exit\n"\
 "  -r, --readonly        Start in read-only mode\n"\
 "  -f, --fork            Switch process to background after sucessfully started\n"\
 "  -d, --debug           Enable tracing of reads and writes of the embedded controller\n"\
 "  -c config, --config-file config\n"\
 "                        Use alternative config file (default /etc/nbfc/nbfc.json)\n"\
 "  -s state.json, --state-file state.json\n"\
 "                        Write state to an alternative file (default /var/run/nbfc_service.state.json)\n"\
 "  -e EC, --embedded-controller EC\n"\
 "                        Specify embedded controller to use\n"\
 ""
