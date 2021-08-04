#!/usr/bin/python3

import sys, argparse

#from argany import *
#a.bash_compgen = 'cd /etc/nbfc/; compgen -f -- $cur'

PROLOG = '''\
NBFC\_SERVICE 1 "MARCH 2021" Notebook FanControl
================================================

NAME
----

nbfc\_service - Notebook FanControl service

'''

argp = argparse.ArgumentParser(prog='nbfc_service', description='NoteBook FanControl service')
argp.add_argument('-r', '--readonly',            help='Start in read-only mode', action='store_true')
argp.add_argument('-f', '--fork',                help='Switch process to background after sucessfully started', action='store_true')
argp.add_argument('-d', '--debug',               help='Enable tracing of reads and writes of the embedded controller', action='store_true')
argp.add_argument('-c', '--config-file',         help='Use alternative config file (default /etc/nbfc/nbfc.json)', metavar='config')
argp.add_argument('-s', '--state-file',          help='Write state to an alternative file (default /var/run/nbfc_service.state.json)', metavar='state.json')
argp.add_argument('-e', '--embedded-controller', help='Specify embedded controller to use', metavar='EC', choices=['dummy','ec_linux', 'ec_sys_linux'])

argp.markdown_prolog = PROLOG
argp.markdown_epilog = '''
FILES
-----

*/var/run/nbfc_service.pid*
  File containing the PID of current running nbfc\_service.

*/var/run/nbfc_service.state.json*
  State file of nbfc\_service. Updated every *EcPollInterval* miliseconds See nbfc\_service.json(5) for further details.

*/etc/nbfc/configs/\*.json*
  Configuration files for various notebook models. See nbfc\_service.json(5) for further details.

EXIT STATUS
-----------

   - 0    Everything fine
   - 1    Generic error
   - 2    Command line error
   - 3    Initialization error
   - 5    Fatal error

BUGS
----

Bugs to https://github.com/nbfc-linux/nbfc-linux

AUTHOR
------

Benjamin Abendroth (braph93@gmx.de)

SEE ALSO
--------

nbfc(1), nbfc\_service.json(5), fancontrol(1)'''

