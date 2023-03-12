NBFC\_SERVICE 1 "MARCH 2021" Notebook FanControl
================================================

NAME
----

nbfc\_service - Notebook FanControl service

DESCRIPTION
-----------

NoteBook FanControl service

SYNOPSIS
--------

`nbfc_service` [OPTIONS]

OPTIONS
-------

  `-h, --help`
    show this help message and exit

  `-r, --readonly`
    Start in read-only mode

  `-f, --fork`
    Switch process to background after sucessfully started

  `-d, --debug`
    Enable tracing of reads and writes of the embedded controller

  `-c, --config-file config`
    Use alternative config file (default /etc/nbfc/nbfc.json)

  `-s, --state-file state.json`
    Write state to an alternative file (default /var/run/nbfc_service.state.json)

  `-e, --embedded-controller EC [dummy, ec_linux, ec_sys_linux, ec_acpi]`
    Specify embedded controller to use


FILES
-----

*/var/run/nbfc_service.pid*
  File containing the PID of current running nbfc\_service.

*/var/run/nbfc_service.state.json*
  State file of nbfc\_service. Updated every *EcPollInterval* miliseconds See [nbfc\_service.json(5)](nbfc_service.json.md) for further details.

*/usr/share/nbfc/configs/\*.json*
  Configuration files for various notebook models. See [nbfc\_service.json(5)](nbfc_service.json.md) for further details.

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

[nbfc(1)](nbfc.md), [nbfc\_service.json(5)](nbfc_service.json.md), [ec\_probe(1)](ec_probe.md), fancontrol(1), sensors(1)
