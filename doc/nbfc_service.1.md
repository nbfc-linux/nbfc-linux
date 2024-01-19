# NAME

nbfc_service - Notebook FanControl service

# DESCRIPTION

NoteBook FanControl service

# SYNOPSIS

**nbfc_service** \[*OPTIONS*\]

# OPTIONS

**-h**, **\--help**

> Show this help message and exit.

**\--version**

> Show program\'s version number and exit.

**-r**, **\--read-only**

> Start in read-only mode.

**-f**, **\--fork**

> Switch process to background after sucessfully started.

**-d**, **\--debug**

> Enable tracing of reads and writes of the embedded controller.

**-c**, **\--config-file** *CONFIG*

> Use alternative config file (default /etc/nbfc/nbfc.json).

**-s**, **\--state-file** *STATE FILE*

> Write state to an alternative file (default
> /var/run/nbfc_service.state.json).

**-e, \--embedded-controller** \[*dummy*, *dev_port*, *ec_sys*,
*acpi_ec*\]

> Specify embedded controller to use.
>
> -   **ec_sys**: Use the ec_sys kernel module for writing to the
>     embedded controller.
>
> -   **acpi_ec**: Use the acpi_ec kernel module for writing to the
>     embedded controller.
>
> -   **dev_port**: Write to the embedded controller using /dev/port.
>
> -   **dummy**: Don\'t write to the embedded controller at all.

# FILES

*/var/run/nbfc_service.pid*

> File containing the PID of current running nbfc_service.

*/var/run/nbfc_service.state.json*

> State file of nbfc_service. Updated every **EcPollInterval**
> milliseconds. See **nbfc_service.json**(5) for further details.

*/usr/share/nbfc/configs/\*.json*

> Configuration files for various notebook models. See
> **nbfc_service.json**(5) for further details.

# EXIT STATUS

> -   0 Everything fine
>
> -   1 Generic error
>
> -   2 Command line error
>
> -   3 Initialization error
>
> -   5 Fatal error

# BUGS

Report bugs to https://github.com/nbfc-linux/nbfc-linux

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc**(1), **nbfc_service.json**(5), **ec_probe**(1),
**fancontrol**(1)
