#!/usr/bin/python3 -B

import sys, config

def print_markdown(fh):
    p = lambda s: print(s, file=fh)

    p('''\
NBFC\_SERVICE 5 "MARCH 2021" Notebook FanControl
================================================

NAME
----

nbfc\_service.json - Notebook FanControl service configuration
''' )

    for name, struct in reversed(config.structs.items()):
        p(f'{name}')
        p( ('-' * len(name)) + '\n')
        p(f'{struct.help}\n')
        for field in struct:
            typ = field.type.replace('*', '\\*')
            if field.valid is not None:
                typ = field.valid.replace('parameter', typ)
            p(f'`"{field.name}"`: *{typ}*')
            help = field.help.replace('\n', '\n  ')
            p(f'  {help}\n')
        p('\n')

    p('''\
FILES
-----

*/var/run/nbfc_service.state.json*
  State file of nbfc\_service. Updated every *EcPollInterval* miliseconds.

*/etc/nbfc/nbfc.json*
  The system wide configuration file.

*/usr/share/nbfc/configs/\*.json*
  Configuration files for various notebook models.

AUTHOR
------

Benjamin Abendroth (braph93@gmx.de)

SEE ALSO
--------

[nbfc(1)](nbfc.md), [nbfc\_service(1)](nbfc_service.md), [ec\_probe(1)](ec_probe.md), fancontrol(1), sensors(1)
''')

if __name__ == '__main__':
    print_markdown(sys.stdout)

