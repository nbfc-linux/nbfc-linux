## DESCRIPTION

Probing tool for embedded controllers

## SYNOPSIS

`ec_probe` [OPTIONS] dump | read | write | monitor

## OPTIONS

  `-h, --help`
    show this help message and exit

  `-e, --embedded-controller EC [ec_linux, ec_sys_linux, ec_acpi]`
    Specify embedded controller to use

### dump

Dump all EC registers

### read REGISTER

Read a byte from a EC register

### write REGISTER VALUE

Write a byte to a EC register

### monitor [OPTIONS]

Monitor all EC registers for changes

  `-h, --help`
    show this help message and exit

  `-i, --interval seconds`
    Monitored timespan

  `-t, --timespan seconds`
    Set poll intervall

  `-r, --report str`
    Save all readings as a CSV file

  `-c, --clearly`
    Blanks out consecutive duplicate readings

  `-d, --decimal`
    Output readings in decimal format instead of hexadecimal format


All input values are interpreted as decimal numbers by default.
Hexadecimal values may be entered by prefixing them with "0x".

## SEE ALSO

[nbfc(1)](nbfc.md), [nbfc\_service(1)](nbfc_service.md), [nbfc\_service.json(5)](nbfc_service.json.md), fancontrol(1)
