DESCRIPTION
-----------

NoteBook FanControl CLI Client

SYNOPSIS
--------

`nbfc` [OPTIONS] start | stop | restart | status | config | set

OPTIONS
-------

  `-h, --help`
    show this help message and exit

  `--version`
    show program's version number and exit


## start [OPTIONS]

Start the service

  `-h, --help`
    show this help message and exit

  `-e, --enabled`
    Start in enabled mode (default)

  `-r, --readonly`
    Start in read-only mode



## stop

Stop the service


## restart

Restart the service

  `-h, --help`
    show this help message and exit

  `-e, --enabled`
    Restart in enabled mode (default)

  `-r, --readonly`
    Restart in read-only mode



## status [OPTIONS]

Show the service status

  `-h, --help`
    show this help message and exit

  `-a, --all`
    Show service and fan status (default)

  `-s, --service`
    Show service status

  `-f, --fan FAN INDEX`
    Show fan status

  `-w, --watch SECONDS`
    Show status periodically


## config [OPTIONS]

List or apply configs

  `-h, --help`
    show this help message and exit

  `-l, --list`
    List all available configs (default)

  `-s, --set config`
    Set a config

  `-a, --apply config`
    Set a config and enable fan control

  `-r, --recommend`
    List configs which may work for your device




## set [OPTIONS]

Control fan speed

  `-h, --help`
    show this help message and exit

  `-a, --auto`
    Set fan speed to 'auto'

  `-s, --speed PERCENT`
    Set fan speed to PERCENT

  `-f, --fan FAN INDEX`
    Fan index (zero based)


FILES
-----

*/var/run/nbfc_service.pid*
  File containing the PID of current running nbfc\_service.

*/var/run/nbfc_service.state.json*
  State file of nbfc\_service. Updated every *EcPollInterval* miliseconds See nbfc\_service.json(5) for further details.

*/usr/share/nbfc/nbfc.json*
  The system wide configuration file. See [nbfc\_service.json(5)](nbfc_service.json.md) for further details.

*/etc/nbfc/configs/\*.json*
  Configuration files for various notebook models. See [nbfc\_service.json(5)](nbfc_service.json.md) for further details.

BUGS
----

Bugs to https://github.com/nbfc-linux/nbfc-linux

AUTHOR
------

Benjamin Abendroth (braph93@gmx.de)

SEE ALSO
--------

[nbfc\_service(1)](nbfc_service.md), [nbfc\_service.json(5)](nbfc_service.json.md), [ec\_probe(1)](ec_probe.md), fancontrol(1)
