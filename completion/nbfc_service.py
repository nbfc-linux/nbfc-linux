#!/usr/bin/python3

import argparse

from argparse_shell_complete import argparse_mod

argp = argparse.ArgumentParser(prog='nbfc_service', description='NoteBook FanControl service')

argp.add_argument('-r', '--read-only', action='store_true',
                  help='Start in read-only mode')

argp.add_argument('-f', '--fork', action='store_true',
                  help='Switch process to background after sucessfully started')

argp.add_argument('-d', '--debug', action='store_true',
                  help='Enable tracing of reads and writes of the embedded controller')

argp.add_argument('-c', '--config-file', metavar='config',
                  help='Use alternative config file (default @SYSCONFDIR@/nbfc/nbfc.json)').complete('file')

argp.add_argument('-e', '--embedded-controller', metavar='EC', choices=['dummy', 'dev_port', 'ec_sys', 'acpi_ec'],
                  help='Specify embedded controller to use')

