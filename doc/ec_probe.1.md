# NAME

ec_probe - probing tool for the embedded controller

# DESCRIPTION

Probing tool for the embedded controller

# SYNOPSIS

**ec_probe** {**read** \| **write** \| **dump** \| **monitor**}
\[*OPTIONS*\]

# OPTIONS

**-h**, **\--help**

> Show the help message and exit

**\--version**

> Show program\'s version number and exit

**-e**,
**\--embedded-controller**=\[**ec_linux**,**ec_sys_linux**,**ec_acpi**,**dummy**\]

> Specify embedded controller to use
>
> -   **ec_sys_linux**: Use the ec_sys kernel module for writing to the
>     embedded controller.
>
> -   **ec_acpi**: Use the acpi_ec kernel module for writing to the
>     embedded controller.
>
> -   **ec_linux**: Write to the embedded controller using /dev/port.
>
> -   **dummy**: Don\'t write to the embedded controller at all.

# COMMANDS

**dump** \[*OPTIONS*\]

> Dump all EC registers

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

All input values are interpreted as decimal numbers by default.
Hexadecimal values may be entered by prefixing them with \"0x\".

# BUGS

Report bugs to https://github.com/nbfc-/nbfc-linux

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc**(1), **nbfc_service**(1), **nbfc_service.json**(5),
**fancontrol**(1)
