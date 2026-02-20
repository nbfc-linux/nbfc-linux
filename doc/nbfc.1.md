# NAME

nbfc - Notebook FanControl client

# DESCRIPTION

This program is used to control the NoteBook FanControl service.

# SYNOPSIS

## SERVICE CONTROL COMMANDS

**nbfc** {**start** \| **stop** \| **restart** \| **status** \|
**config** \| **set** \| **sensors**} \[*OPTIONS*\]

## MISCELLANEOUS COMMANDS

**nbfc** {**update** \| **rate-config** \| **acpi-dump** \|
**get-model-name** \| **support** \| **warranty** \| **help**}
\[*OPTIONS*\]

# OPTIONS

**-h**, **\--help**

> Show help message and exit.

**\--version**

> Show program\'s version number and exit.

# COMMANDS

**start** \[*OPTIONS*\]

> Start the service.
>
> **-r**, **\--read-only**
>
> > Start in read-only mode.

**stop**

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
> > List all available configs.
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
> > Set a config and start the service.

**set** \[*OPTIONS*\]

> Control fan speed.
>
> **-a**, **\--auto**
>
> > Set fan speed to profile-based control.
>
> **-s**, **\--speed** *PERCENT*
>
> > Set fan speed to PERCENT.
>
> **-f**, **\--fan** *FAN INDEX*
>
> > Fan index (zero based). If not given, all available fans are set to
> > speed.

**sensors** {**list** \| **set** \| **show**} \[*OPTIONS*\]

> Configure fan sensors.
>
> **list**
>
> > List all available sensors and their temperature files.
>
> **show**
>
> > Show all available fans and their sensor configuration.
>
> **set** \[*OPTIONS*\]
>
> > Configure which sensors are attached to a fan.
> >
> > **-f**, **\--fan** *FAN INDEX*
> >
> > > Specifies the fan to configure.
> >
> > **-s**, **\--sensor** *SENSOR*
> >
> > > Sensor to add. Can be specified multiple times.
> >
> > **-a**, **\--algorithm** *ALGORITHM*
> >
> > > Algorithm (**Average**, **Min**, **Max**)
> >
> > **\--force**
> >
> > > Force applying a sensor if it could not be found.

**update** \[*OPTIONS*\]

> Download new configuration files from the internet.
>
> **-p**, **\--parallel** *NUM*
>
> > Set the number of parallel downloads (default: 10).
>
> **-q**, **\--quiet**
>
> > Set quiet mode.

**rate-config** \[*OPTIONS*\]

> Rates a configuration by analyzing whether it appears safe to execute
> on the current system.
>
> **-H**, **\--full-help**
>
> > Show help on how to interpret the results.
>
> **-a**, **\--all**
>
> > Rate all available configuration files.
>
> **-d**, **\--dsdt** *FILE*
>
> > Use an alternative DSDT file.
>
> **-j**, **\--json**
>
> > Use JSON output.
>
> **-m**, **\--min-score** *SCORE*
>
> > Set minimum rating threshold for configurations.
>
> **\--print-rules**
>
> > Print configuration rating rules.

**acpi-dump** {**registers** \| **ec-registers** \| **methods** \|
**dsl**} \[*OPTIONS*\]

> Dumps information of your ACPI DSDT.
>
> **registers**
>
> > List all available registers.
>
> **ec-registers**
>
> > List all available embedded controller registers.
>
> **methods**
>
> > List all available ACPI methods.
>
> **dsl**
>
> > Disassemble your DSDT.
>
> **-f**, **\--file** *FILE*
>
> > Use an alternative DSDT file.
>
> **-j**, **\--json**
>
> > Use JSON output.

**support** \[*OPTIONS*\]

> Show how to support the project.
>
> **\--upload-firmare**
>
> > Upload your notebook firmware without prompting.
>
> **\--print-command**
>
> > Print command for manual firmware upload.

**get-model-name**

> Print out the notebook\'s model name.

**warranty**

> Show warranty.

**help**

> Show help.

# FILES

*/etc/nbfc/nbfc.json*

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

# EXIT STATUS

> 0.  Everything fine
>
> 1.  Generic error
>
> 2.  Command-line error
>
> 3.  Initialization error
>
> 4.  Fatal error (memory allocation failure)

# BUGS

Report bugs to https://github.com/nbfc-linux/nbfc-linux

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc_service**(1), **nbfc_service.json**(5), **ec_probe**(1),
**fancontrol**(1)
