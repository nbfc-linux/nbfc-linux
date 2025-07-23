# NAME

ec_probe - probing tool for the embedded controller

# DESCRIPTION

Probing tool for the embedded controller

# SYNOPSIS

**ec_probe** {**read** \| **write** \| **dump** \| **load** \|
**monitor** \| **watch** \| **acpi_call** \| **shell**} \[*OPTIONS*\]

# OPTIONS

**-h**, **\--help**

> Show the help message and exit

**\--version**

> Show program\'s version number and exit

**-e**,
**\--embedded-controller**=\[**dev_port**,**ec_sys**,**acpi_ec**\]

> Specify embedded controller to use
>
> -   **ec_sys**: Use the ec_sys kernel module for writing to the
>     embedded controller.
>
> -   **acpi_ec**: Use the acpi_ec kernel module for writing to the
>     embedded controller.
>
> -   **dev_port**: Write to the embedded controller using /dev/port.

# COMMANDS

**dump** \[*OPTIONS*\]

> Dump all EC registers
>
> **-c**, **\--color**
>
> > Force colored output.
>
> **-C**, **\--no-color**
>
> > Disable colored output.

**load** *FILE*

> Load a previously made dump into the embedded controller.

**read** \[*OPTIONS*\] *REGISTER*

> Reads *REGISTER* from the embedded controller and prints out the
> value.
>
> **-w**, **\--word**
>
> > If given, the value of *REGISTER* and the value of *REGISTER + 1* is
> > read and combined into a single value.

**write** \[*OPTIONS*\] *REGISTER* *VALUE*

> Writes *VALUE* to *REGISTER* of the embedded controller.
>
> **-w**, **\--word**
>
> > If given, *VALUE* is written into a combined register of *REGISTER*
> > and *REGISTER + 1*.

**monitor** \[*OPTIONS*\]

> Monitor all embedded controller registers for changes.
>
> **-i**, **\--interval** *SECONDS*
>
> > Sets the update interval in seconds.
>
> **-t**, **\--timespan** *SECONDS*
>
> > Sets how many seconds the program will run.
>
> **-r**, **\--report** *FILE*
>
> > Save the monitored results to a CSV file.
>
> **-c**, **\--clearly**
>
> > With **-r**\|**\--report**: Blanks out consecutive duplicate
> > readings when writing to the CSV file.
>
> **-d**, **\--decimal**
>
> > With **-r**\|**\--report**: Output results in decimal format instead
> > of hexadecimal format

**watch** \[*OPTIONS*\]

> Monitor all embedded controller registers for changes (alternative
> version).
>
> **-i**, **\--interval** *SECONDS*
>
> > Sets the update interval in seconds.
>
> **-t**, **\--timespan** *SECONDS*
>
> > Sets how many seconds the program will run.

**acpi_call** *METHOD* \[*ARGUMENT*\...\]

> Call an ACPI Method.
>
> The maximum number of arguments is 8.
>
> Example:
>
> > ec_probe acpi_call \'\\\_SB.PCI0.LPCB.EC0.SFSD\' 0xFF

**shell**

> Read commands from STDIN

All input values are interpreted as decimal numbers by default.
Hexadecimal values may be entered by prefixing them with \"0x\".

# BUGS

Report bugs to https://github.com/nbfc-/nbfc-linux

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc**(1), **nbfc_service**(1), **nbfc_service.json**(5),
**fancontrol**(1)
