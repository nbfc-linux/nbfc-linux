#!/usr/bin/python3

import sys, os, time, argparse, json

# Unused - depends on `dmidecode` module
#def dmidecode_py_get_system_product():
#    import dmidecode
#    r = dmidecode.system()
#    def find_product(d):
#        if type(d) is dict:
#            for key, value in d.items():
#                if key == 'Product Name':
#                    return str(value, encoding='UTF-8')
#                r = find_product(value)
#                if r:
#                    return r
#        else:
#            return None
#    return find_product(r)

def check_root():
    if os.geteuid() != 0:
        raise Exception('This operation has to be run as root')

def dmidecode_bin_get_system_product():
    import subprocess

    try:
        proc = subprocess.Popen(['dmidecode', '-s', 'system-product-name'], stdout=subprocess.PIPE)
        return str(proc.communicate()[0], encoding='UTF-8')
    except FileNotFoundError:
        raise Exception('Binary `dmidecode` not found. Make sure it is installed')

def get_system_product():
    return dmidecode_bin_get_system_product()

class NbfcService:
    CONFIG_DIR  = '/etc/nbfc'
    CONFIGS_DIR = '/usr/share/nbfc/configs'
    CONFIG_FILE = '/etc/nbfc/nbfc.json'
    STATE_FILE  = '/var/run/nbfc_service.state.json'
    PID_FILE    = '/var/run/nbfc_service.pid'

    def get_service_pid(self):
        with open(self.PID_FILE) as fh:
            return int(fh.read())

    def start(self, readonly=False):
        try:
            pid = self.get_service_pid()
            print('Service already running:', pid)
            return 0
        except:
            return os.system('nbfc_service -f%s' % ('-r' if readonly else ''))

    def stop(self):
        import signal
        os.kill(self.get_service_pid(), signal.SIGINT)
        os.remove(self.STATE_FILE)

    def restart(self, readonly=False):
        try:     self.stop()
        except:  pass
        time.sleep(1)
        self.start(readonly)

    def list_configs(self):
        return os.listdir(self.CONFIGS_DIR)

    def recommended_configs(self, product_name):
        def word_difference(a, b):
            a = a.lower()
            b = b.lower()
            if a == b: return 0
            diff = 0
            for a_c, b_c in zip(a, b):
                diff += abs(ord(a_c) - ord(b_c))
            return diff

        def words_difference(a, b):
            A = a.lower().split()
            B = b.lower().split()
            l = len(max(A, B))
            diff = 0
            for a, b in zip(A, B):
                diff += word_difference(a, b)
            diff /= l
            return diff

        product = get_system_product()
        if not product:
            raise Exception('Could not get product name')

        files = os.listdir(self.CONFIGS_DIR)
        files = [os.path.splitext(f)[0] for f in files]
        files = [(f, words_difference(product, f)) for f in files]
        files.sort(key=lambda f: f[1])
        return files

    def get_status(self):
        if not os.path.exists(self.STATE_FILE):
            raise Exception('Service not running')

        with open(self.STATE_FILE, 'r') as fh:
            return json.load(fh)

    def get_config(self):
        try:
            with open(self.CONFIG_FILE, 'r') as fh:
                return json.load(fh)
        except:
            return {}

    def set_config(self, cfg):
        with open(self.CONFIG_FILE, 'w') as fh:
            return json.dump(cfg, fh)

service = NbfcService()

def config(opts):
    if opts.list:
        files = service.list_configs()
        for f in files:
            print(os.path.splitext(f)[0])

    elif opts.recommend:
        check_root()
        files = service.recommended_configs(get_system_product())
        if len(files) and files[0][1] == 0:
            print(files[0][0])
        else:
            for f in files[:15]:
                print(f[0])

    elif opts.set or opts.apply:
        check_root()
        cfg = service.get_config()
        model = opts.set if opts.set else opts.apply

        if model == 'auto':
            files = service.recommended_configs(get_system_product())
            if len(files) and files[0][1] == 0:
                model = files[0][0]
            else:
                raise Exception("Try `nbfc config -r` for recommended configs")

        cfg['SelectedConfigId'] = model
        service.set_config(cfg)

        if opts.apply:
            service.restart()

def set(opts):
    check_root()
    cfg = service.get_config()

    if 'TargetFanSpeeds' not in cfg:
        cfg['TargetFanSpeeds'] = []
    targetFanSpeeds = cfg['TargetFanSpeeds']
    while True:
        try:
            targetFanSpeeds[opts.fan] = opts.speed
            break
        except IndexError:
            targetFanSpeeds.append(-1)

    service.set_config(cfg)
    service.restart()

def status(opts):
    def print_service_status(status):
        #print('Service enabled         :', status['enabled'])
        print('Read-only               :', status['readonly'])
        print('Selected config name    :', status['config'])
        print('Temperature             :', status['temperature'])

    def print_fan_status(fan):
        print('Fan display name        :', fan['name'])
        print('Auto control enabled    :', fan['automode'])
        print('Critical mode enabled   :', fan['critical'])
        print('Current fan speed       :', fan['current_speed'])
        print('Target fan speed        :', fan['target_speed'])
        print('Fan speed steps         :', fan['speed_steps'])

    while True:
        try:
            status = service.get_status()
            print_service_status(status)
            for fan in status['fans']:
                print()
                print_fan_status(fan)

        except KeyboardInterrupt:
            pass
        except Exception as e:
            print('Error:', e)
        finally:
            if opts.watch is None:
                return

            try:   time.sleep(opts.watch)
            except KeyboardInterrupt: return
            print()

def start(opts):
    check_root()
    service.start(opts.readonly)

def stop(opts):
    check_root()
    service.stop()

def restart(opts):
    check_root()
    service.restart(opts.readonly)

def wait_for_hwmon(opts):
    HWMonNameFiles = ["/sys/class/hwmon/hwmon{}/name", "/sys/class/hwmon/hwmon{}/device/name"]
    LinuxTempSensorNames = ["coretemp", "k10temp", "zenpower"]

    for try_ in range(30):
        for name_file_fmt in HWMonNameFiles:
            for i in range(10):
                name_file = name_file_fmt.format(i)

                try:
                    with open(name_file, 'r') as fh:
                        name = fh.read().strip()
                        if name in LinuxTempSensorNames:
                            sys.exit(0)
                except FileNotFoundError:
                    pass

        time.sleep(1)
    sys.exit(1)


argp = argparse.ArgumentParser(prog='nbfc', description='NoteBook FanControl CLI Client')
argp.add_argument('--version', action='version', version='nbfc 0.1.6')
subp = argp.add_subparsers(description='commands')

cmdp = subp.add_parser('start',       help='Start the service')
cmdp.add_argument('-r', '--readonly', help='Start in read-only mode',         action='store_true')
cmdp.set_defaults(cmd=start)

cmdp = subp.add_parser('stop',        help='Stop the service')
cmdp.set_defaults(cmd=stop)

cmdp = subp.add_parser('restart',     help='Restart the service')
cmdp.add_argument('-r', '--readonly', help='Restart in read-only mode',         action='store_true')
cmdp.set_defaults(cmd=restart)

cmdp = subp.add_parser('status',      help='Show the service status')
agrp = cmdp.add_mutually_exclusive_group(required=True)
agrp.add_argument('-a', '--all',      help='Show service and fan status (default)', action='store_true')
agrp.add_argument('-s', '--service',  help='Show service status', action='store_true')
agrp.add_argument('-f', '--fan',      help='Show fan status',          type=int,   metavar='FAN INDEX')
cmdp.add_argument('-w', '--watch',    help='Show status periodically', type=float, metavar='SECONDS')
cmdp.set_defaults(cmd=status)

cmdp = subp.add_parser('config',        help='List or apply configs')
agrp = cmdp.add_mutually_exclusive_group(required=True)
agrp.add_argument('-l', '--list',       help='List all available configs (default)', action='store_true')
agrp.add_argument('-s', '--set',        help='Set a config', metavar='config')
agrp.add_argument('-a', '--apply',      help='Set a config and enable fan control', metavar='config')
agrp.add_argument('-r', '--recommend',  help='List configs which may work for your device', action='store_true')
cmdp.set_defaults(cmd=config)

cmdp = subp.add_parser('set',                  help='Control fan speed')
agrp = cmdp.add_mutually_exclusive_group(required=True)
agrp.add_argument('-a', '--auto',              help='Set fan speed to \'auto\'', action='store_const', dest='speed', const=-1)
agrp.add_argument('-s', '--speed', type=float, help='Set fan speed to PERCENT', dest='speed', metavar='PERCENT')
cmdp.add_argument('-f', '--fan',   type=int,   help='Fan index (zero based)',   metavar='FAN INDEX', default=0)
cmdp.set_defaults(cmd=set, speed=-1)

cmdp = subp.add_parser('wait-for-hwmon', help='Wait for /sys/class/hwmon/hwmon* files')
cmdp.set_defaults(cmd=wait_for_hwmon)

def show_help(opts):
    argp.print_help()
cmdp = subp.add_parser('help', help='Show help')
cmdp.set_defaults(cmd=show_help)
argp.set_defaults(cmd=show_help)

if __name__ == '__main__':
    if not sys.argv[0].startswith('/'):
        os.environ['PATH'] += ':.'
        os.environ['PATH'] += ':./src'

    if not os.path.isdir(service.CONFIG_DIR):
        os.mkdir(service.CONFIG_DIR)

    opts = argp.parse_args()

    try:
        opts.cmd(opts)
    except Exception as e:
        print("Error:", e, file=sys.stderr)

else:
    argp.markdown_prolog = '''\
NBFC\_SERVICE 1 "MARCH 2021" Notebook FanControl
================================================

NAME
----

nbfc\_service - Notebook FanControl service

'''

    argp.markdown_epilog = '''
FILES
-----

*/var/run/nbfc_service.pid*
  File containing the PID of current running nbfc\_service.

*/var/run/nbfc_service.state.json*
  State file of nbfc\_service. Updated every *EcPollInterval* miliseconds See nbfc\_service.json(5) for further details.

*/etc/nbfc/nbfc.json*
  The system wide configuration file. See nbfc\_service.json(5) for further details.

*/etc/nbfc/configs/\*.json*
  Configuration files for various notebook models. See nbfc\_service.json(5) for further details.

BUGS
----

Bugs to https://github.com/nbfc-linux/nbfc-linux

AUTHOR
------

Benjamin Abendroth (braph93@gmx.de)

SEE ALSO
--------

nbfc_service(1), nbfc\_service.json(5), ec_probe(1), fancontrol(1)'''
