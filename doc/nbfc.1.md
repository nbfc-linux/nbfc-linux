# NAME

nbfc - Notebook FanControl client

# DESCRIPTION

This program is used to control the NoteBook FanControl service.

# SYNOPSIS

## SERVICE CONTROL COMMANDS

**nbfc** {**start** \| **stop** \| **restart** \| **status** \|
**config** \| **set** \| **sensors**} \[*OPTIONS*\]

## MISCELLANEOUS COMMANDS

**nbfc** {**update** \| **rate-config** \| **test-config** \|
**acpi-dump** \| **get-model-name** \| **xml2json** \| **support** \|
**warranty** \| **faq** \| **help**} \[*OPTIONS*\]

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
> > List configs with a similar notebook model name.
>
> **-s**, **\--set** *CONFIG*
>
> > Set a config.
>
> If *CONFIG* is **auto**, the service will attempt to automatically
> select a matching configuration.
>
> *CONFIG* is the configuration filename without the **.json**
> extension, not the value of the **NotebookModel** field inside the
> configuration file.

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
> Downloaded files will be stored in */var/lib/nbfc/configs*.
>
> **-p**, **\--parallel** *NUM*
>
> > Set the number of parallel downloads (default: 10).
>
> **-q**, **\--quiet**
>
> > Set quiet mode.

**rate-config** \[*FILE*\] \[*OPTIONS*\]

> Rates a configuration by analyzing whether it appears safe to execute
> on the current system.
>
> By default, the DSDT and all SSDTs are read from
> /sys/firmware/acpi/tables.
>
> **-H**, **\--full-help**
>
> > Show help on how to interpret the results.
>
> **-a**, **\--all**
>
> > Rate all available configuration files.
>
> **-b**, **\--bad**
>
> > List only bad configs that are otherwise omitted.
>
> **-d**, **\--dsdt** *FILE*
>
> > Use an alternative DSDT file. Can be specified multiple times.
>
> **-D**, **\--dsdt-dir** *DIRECTORY*
>
> > Use an alternative DSDT directory.
>
> **-f**, **\--fan-count** *NUMBER*
>
> > Limit output to configurations with the specified number of fans.
>
> **-i**, **\--input** *FILE*
>
> > Read configuration files from *FILE*.
>
> **-j**, **\--json**
>
> > Use JSON output.
>
> **-m**, **\--min-score** *SCORE*
>
> > Set minimum rating threshold for displaying configurations (default:
> > 9.0).
>
> **-n**, **\--no-download**
>
> > Don\'t download configuration rules from the repository.
>
> **-r**, **\--rules** *FILE*
>
> > Use an alternative rules file.
>
> **-u**, **\--unverified**
>
> > Normally, only registers belonging to an **EmbeddedControl**
> > operation region are taken into account when rating a configuration.
> > With this option, registers from **SystemMemory** operation regions
> > that may be accessible through the embedded controller are also
> > considered. Their accessibility through the embedded controller
> > cannot be guaranteed.
>
> **-q**, **\--quiet**
>
> > Do not print register ratings.
>
> **\--print-rules**
>
> > Print configuration rating rules.

**test-config** {**run** \| **evaluate**} \[*OPTIONS*\]

> Test configurations automatically.
>
> This command should only be used in conjunction with **nbfc
> rate-config**.
>
> For example:
>
> > **sudo nbfc rate-config -aqq \| sudo nbfc test-config run**
>
> It should **not** be used to test arbitrary configurations.
>
> **run** \[*OPTIONS*\]
>
> > Reads a list of configuration files from an input file (default:
> > **stdin**).
> >
> > Configurations will be loaded in read-only mode.
> >
> > The notebook\'s fans will be spinned up by stressing the CPU/GPU
> > while reading back the fan speed for each configuration.
> >
> > The result will be written to **nbfc.test-config.result.json**.
> >
> > **-i**, **\--input** *FILE*
> >
> > > Specify input file.
> >
> > **-o**, **\--output** *FILE*
> >
> > > Specify output file.
> >
> > **-c**, **\--cpu** *NUMBER*
> >
> > > Specify number of CPU workers.
> >
> > **-g**, **\--gpu** *NUMBER*
> >
> > > Specify number of GPU workers (default: **no workers**).
> >
> > **-I**, **\--interval** *SECONDS*
> >
> > > Set sample interval (default: **0.5**).
> >
> > **-t**, **\--threshold** *DELTA*
> >
> > > Stop when temperature difference falls below *DELTA* (default:
> > > **2.0**).
> >
> > **-b**, **\--break** *SECONDS*
> >
> > > Stop after the temperature difference has remained below *DELTA*
> > > for *SECONDS* consecutive seconds (default: **10.0**).
>
> **evaluate** \[*OPTIONS*\]
>
> > Evaluates a previously generated result file. Default:
> > **nbfc.test-config.result.json**.
> >
> > **-i**, **\--input** *FILE*
> >
> > > Specify input file.

**acpi-dump** {**registers** \| **ec-registers** \| **methods** \|
**dsl** \| **map**} \[*OPTIONS*\]

> Dumps information of your ACPI tables.
>
> By default, the DSDT and all SSDTs are read from
> /sys/firmware/acpi/tables.
>
> **registers**
>
> > List all available registers.
>
> **ec-registers** \[**-u**\|**\--unverified**\]
>
> > List all available embedded controller registers.
> >
> > Normally, only registers belonging to an **EmbeddedControl**
> > operation region are printed. If **-u**\|**\--unverified** is
> > specified, also registers from **SystemMemory** operation regions
> > that may be accessible through the embedded controller are printed.
> > Their accessibility through the embedded controller cannot be
> > guaranteed.
>
> **methods**
>
> > List all available ACPI methods.
>
> **dsl**
>
> > Disassemble your ACPI tables.
>
> **map** \[**-u**\|**\--unverified**\]
>
> > Generate a map file that can be used by **ec_probe -m\|\--map**. The
> > map contains detected registers with their names and addresses as
> > well as ACPI method names.
> >
> > Normally, only registers belonging to an **EmbeddedControl**
> > operation region are printed. If **-u**\|**\--unverified** is
> > specified, also registers from **SystemMemory** operation regions
> > that may be accessible through the embedded controller are printed.
> > Their accessibility through the embedded controller cannot be
> > guaranteed.
>
> **-d**, **\--dsdt** *FILE*
>
> > Use an alternative DSDT file. Can be specified multiple times.
>
> **-D**, **\--dsdt-dir** *DIRECTORY*
>
> > Use an alternative DSDT directory.
>
> **-j**, **\--json**
>
> > Use JSON output.

**support** \[*OPTIONS*\]

> Show how to support the project.
>
> **\--upload-firmware**
>
> > Upload your notebook firmware without prompting.
>
> **\--print-command**
>
> > Print command for manual firmware upload.
>
> **\--create-archive** *FILE*
>
> > Creates a compressed **tar.gz** archive containing information
> > required for support and hardware analysis.
> >
> > The archive includes:
> >
> > - The output of **nbfc get-model-name**
> >
> > - The output of **nbfc sensors list**
> >
> > - Firmware DSDT (*/sys/firmware/acpi/tables/DSDT*)
> >
> > - Firmware SSDTs (*/sys/firmware/acpi/tables/SSDT\**)

**get-model-name**

> Print out the notebook\'s model name.

**xml2json** *FILE*

> Convert an XML configuration file to JSON.
>
> This command can be used to convert configurations from the original
> NBFC project into the NBFC-Linux format.

**warranty**

> Show warranty.

**faq**

> Show NBFC FAQ manual page.

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
