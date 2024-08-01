#!/usr/bin/python3

import argparse

from argparse_shell_complete import argparse_mod

argp = argparse.ArgumentParser(prog='ec_probe', description='Probing tool for embedded controllers')

argp.add_argument('-e', '--embedded-controller', metavar='EC', choices=['acpi_ec', 'ec_sys', 'dev_port'],
                  help='Specify embedded controller to use')

subp = argp.add_subparsers(description='commands')

# =============================================================================
# Command 'dump'
# =============================================================================
cmdp = subp.add_parser('dump', help='Dump all EC registers')

cmdp.add_argument('-c', '--color', action='store_true',
                  help='Force colored output')

cmdp.add_argument('-C', '--no-color', action='store_true',
                  help='Disable colored output')

# =============================================================================
# Command 'load'
# =============================================================================
cmdp = subp.add_parser('load', help='Load a previously made dump into the embedded controller')

cmdp.add_argument('FILE').complete('file')

# =============================================================================
# Command 'read'
# =============================================================================
cmdp = subp.add_parser('read', help='Read a byte from a EC register')

cmdp.add_argument('-w', '--word', action='store_true',
                  help='Combine two registers into one')

cmdp.add_argument('register', type=int, metavar='REGISTER', choices=range(0, 255),
                  help='Register source')

# =============================================================================
# Command 'write'
# =============================================================================
cmdp = subp.add_parser('write', help='Write a byte to a EC register')

cmdp.add_argument('-w', '--word', action='store_true',
                  help='Combine two registers into one')

cmdp.add_argument('register', type=int, metavar='REGISTER', choices=range(0, 255),
                  help='Register destination')

cmdp.add_argument('value', type=int, metavar='VALUE',
                  help='Value to write')

# =============================================================================
# Command 'monitor'
# =============================================================================
cmdp = subp.add_parser('monitor', help='Monitor all EC registers for changes')

cmdp.add_argument('-i', '--interval', metavar='seconds', type=int,
                  help='Monitored timespan')

cmdp.add_argument('-t', '--timespan', metavar='seconds', type=int,
                  help='Set poll intervall')

cmdp.add_argument('-r', '--report',
                  help='Save all readings as a CSV file').complete('file')

cmdp.add_argument('-c', '--clearly', action='store_true',
                  help='Blanks out consecutive duplicate readings')

cmdp.add_argument('-d', '--decimal', action='store_true',
                  help='Output readings in decimal format instead of hexadecimal format')

# =============================================================================
# Command 'watch'
# =============================================================================
cmdp = subp.add_parser('watch', help='Monitor all EC registers for changes (alternative version')

cmdp.add_argument('-i', '--interval', type=float, metavar='seconds',
                  help='Sets the update interval in seconds')

cmdp.add_argument('-t', '--timespan', type=int, metavar='seconds',
                  help='Sets how many seconds the program will run')
