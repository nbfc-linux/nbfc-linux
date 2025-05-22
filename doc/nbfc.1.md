# NAME

nbfc - Notebook FanControl client

# DESCRIPTION

This program is used to control the NoteBook FanControl service.

# SYNOPSIS

**nbfc** {**start** \| **stop** \| **restart** \| **status** \|
**config** \| **set** \| **update** \| **help**} \[*OPTIONS*\]

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
> > Fan index (zero based). If not given, all available fans are set to
> > speed.

**update** \[*OPTIONS*\]

> Download new configuration files.
>
> **-p**, **\--parallel** *NUM*
>
> > Set the number of parallel downloads.
>
> **-q**, **\--quiet**
>
> > Set quiet mode.

**help**

> Show help.

# FILES

*/etc/nbfc.json*

> The main configuration file for nbfc_service. See
> **nbfc_service.json**(5) for further details.

*/var/run/nbfc_service.pid*

> File containing the PID of current running nbfc_service.

*/var/run/nbfc_service.socket*

> Socket file of nbfc_service.

*/usr/share/nbfc/configs/\*.json*

> Configuration files for various notebook models. See
> **nbfc_service.json**(5) for further details.

*/usr/share/nbfc/model_support.json*

> Model support database. This file maps notebook model names to
> configuration files.

*/var/lib/nbfc/configs/\*.json*

> Configuration files downloaded from the internet. Files in this
> directory take precedence over those in */usr/share/nbfc/configs*.

*/var/lib/nbfc/model_support.json*

> Model support database downloaded from the internet. Both
> */var/lib/nbfc/model_support.json* and
> */usr/share/nbfc/model_support.json* are taken into account.

*/var/lib/nbfc/state.json*

> State file of nbfc_service. This holds the current fan speeds.

# BUGS

Report bugs to https://github.com/nbfc-/nbfc-linux

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc_service**(1), **nbfc_service.json**(5), **ec_probe**(1),
**fancontrol**(1)
