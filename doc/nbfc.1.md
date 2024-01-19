# NAME

nbfc - Notebook FanControl client

# DESCRIPTION

This program is used to control the NoteBook FanControl service.

# SYNOPSIS

**nbfc** {**start** \| **stop** \| **restart** \| **status** \|
**config** \| **set** \| **help**} \[*OPTIONS*\]

# OPTIONS

**-h**, **\--help**

> Show this help message and exit.

**\--version**

> Show program\'s version number and exit.

# COMMANDS

**start** \[*OPTIONS*\]

> Start the service.
>
> **-r**, **\--read-only**
>
> > Start in read-only mode.

**stop** \[*OPTIONS*\]

> Stop the service.

**restart** \[*OPTIONS*\]

> Restart the service.
>
> **-r**, **\--read-only**
>
> > Start in read-only mode.

**status** \[*OPTIONS*\]

> Show the service status.
>
> **-a**, **\--all**
>
> > Show service and fan status (default).
>
> **-s**, **\--service**
>
> > Show service status.
>
> **-f**, **\--fan** *FAN INDEX*
>
> > Show fan status.
>
> **-w**, **\--watch** *SECONDS*
>
> > Show status periodically.

**config** \[*OPTIONS*\]

> List or apply configs.
>
> **-l**, **\--list**
>
> > List all available configs (default).
>
> **-r**, **\--recommend**
>
> > List configs which may work for your device.
>
> **-s**, **\--set** *CONFIG*
>
> > Set a config.
>
> **-a**, **\--apply** *CONFIG*
>
> > Set a config and enable fan control.

**set** \[*OPTIONS*\]

> Control fan speed.
>
> **-a**, **\--auto**
>
> > Set fan speed to \'auto\'.
>
> **-s**, **\--speed** *PERCENT*
>
> > Set fan speed to PERCENT.
>
> **-f**, **\--fan** *FAN INDEX*
>
> > Fan index (zero based).

**help**

> Show help.

# FILES

*/var/run/nbfc_service.pid*

> File containing the PID of current running nbfc_service.

*/var/run/nbfc_service.state.json*

> State file of nbfc_service. Updated every **EcPollInterval**
> milliseconds. See **nbfc_service.json**(5) for further details.

*/etc/nbfc.json*

> The system wide configuration file. See **nbfc_service.json**(5) for
> further details.

*/usr/share/nbfc/configs/\*.json*

> Configuration files for various notebook models. See
> **nbfc_service.json**(5) for further details.

# BUGS

Report bugs to https://github.com/nbfc-/nbfc-linux

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc_service**(1), **nbfc_service.json**(5), **ec_probe**(1),
**fancontrol**(1)
