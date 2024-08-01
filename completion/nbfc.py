#!/usr/bin/python3

import argparse

from argparse_shell_complete import argparse_mod

NBFC_CONFIGS_DIR = '@DATADIR@/nbfc/configs'

argp = argparse.ArgumentParser(prog='nbfc', description='NoteBook FanControl CLI Client')

argp.add_argument('--version', action='version', version='nbfc 0.1.6')

subp = argp.add_subparsers(description='commands')

# =============================================================================
# Command 'start'
# =============================================================================
cmdp = subp.add_parser('start', help='Start the service')

cmdp.add_argument('-r', '--read-only', action='store_true',
                  help='Start in read-only mode')

# =============================================================================
# Command 'stop'
# =============================================================================
cmdp = subp.add_parser('stop', help='Stop the service')

# =============================================================================
# Command 'restart'
# =============================================================================
cmdp = subp.add_parser('restart', help='Restart the service')

cmdp.add_argument('-r', '--read-only', action='store_true',
                  help='Restart in read-only mode')

# =============================================================================
# Command 'status'
# =============================================================================
cmdp = subp.add_parser('status', help='Show the service status')
agrp = cmdp.add_mutually_exclusive_group(required=True)

agrp.add_argument('-a', '--all', action='store_true',
                  help='Show service and fan status (default)')

agrp.add_argument('-s', '--service', action='store_true',
                  help='Show service status')

agrp.add_argument('-f', '--fan', type=int, metavar='FAN INDEX',
                  help='Show status of fan (zero based)')

cmdp.add_argument('-w', '--watch',
                  help='Show status periodically', type=float, metavar='SECONDS')

# =============================================================================
# Command 'config'
# =============================================================================
cmdp = subp.add_parser('config', help='List or apply configs')
agrp = cmdp.add_mutually_exclusive_group(required=True)

agrp.add_argument('-l', '--list', action='store_true',
                  help='List all available configs (default)')

agrp.add_argument('-s', '--set', metavar='config',
                  help='Set a config').complete('file', {'directory': NBFC_CONFIGS_DIR})

agrp.add_argument('-a', '--apply', metavar='config',
                  help='Set a config and enable fan control').complete('file', {'directory': NBFC_CONFIGS_DIR})

agrp.add_argument('-r', '--recommend', action='store_true',
                  help='List configs which may work for your device')

# =============================================================================
# Command 'set'
# =============================================================================
cmdp = subp.add_parser('set', help='Control fan speed')
agrp = cmdp.add_mutually_exclusive_group(required=True)

agrp.add_argument('-a', '--auto', action='store_const', dest='speed', const=-1,
                  help='Set fan speed to \'auto\'')

agrp.add_argument('-s', '--speed', type=float, dest='speed', metavar='PERCENT',
                  help='Set fan speed to PERCENT')

cmdp.add_argument('-f', '--fan',   type=int, metavar='FAN INDEX', default=0,
                  help='Fan index (zero based)')

# =============================================================================
# Command 'wait-for-hwmon'
# =============================================================================
cmdp = subp.add_parser('wait-for-hwmon', help='Wait for /sys/class/hwmon/hwmon* files')

# =============================================================================
# Command 'help'
# =============================================================================
cmdp = subp.add_parser('help', help='Show help')
