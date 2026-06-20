# GENERAL

## What is NBFC-Linux?

NBFC-Linux is a C port of the original but unmaintained C# software NBFC
(NoteBook FanControl). It allows to control the notebook\'s fans by
accessing the embedded controller (EC) or invoking ACPI methods. It
shares the same utilities and interfaces as the original NBFC.

## What is the difference between NBFC and NBFC-Linux?

NBFC-Linux is specifically designed and optimized for the Linux
ecosystem.

It also comes with features that the original NBFC does not have, such
as:

> -   Configuration rating (**nbfc rate-config**)
>
> -   Support for calling ACPI methods
>
> -   Lua scripting
>
> -   Online configuration fetching
>
> -   Firmware fingerprinting
>
> -   Shell autocompletion for Bash, Fish and Zsh.

# INSTALLATION

## Where to get NBFC-Linux from?

NBFC-Linux can be downloaded from
*https://github.com/nbfc-linux/nbfc-linux*.

Packages are available for **Ubuntu**, **Linux Mint**, **Debian**,
**Fedora**, **OpenSuse** and **Arch Linux**.

## Which kernel modules are required?

For accessing the embedded controller:

> -   **ec_sys** (most preferred)
>
> -   **acpi_ec** (needs to be manually installed, see
>     *https://github.com/saidsay-so/acpi_ec*).
>
> The embedded controller can also be accessed through */dev/port*
> (without any kernel module).

For invoking ACPI methods:

> -   **acpi_call** (manual compilation may be required for some
>     distributions)

# CONFIGURATION

## Updating configurations

To fetch the latest configuration files from the project repository,
run:

> **sudo nbfc update**

## Setting a configuration

To set a configuration, run:

> **sudo nbfc config \--set** *CONFIG*

Where *CONFIG* is the base filename of a configuration (without
**.json** file extension).

## Testing a configuration

When trying out configuration files, it is recommended to follow these
steps:

> -   Set the configuration using **sudo nbfc config \--set** *CONFIG*
>
> -   Start the service in read-only mode using **sudo nbfc restart
>     \--read-only**
>
> -   Let the fan spin up and down by stressing the CPU (using **stress
>     -c 8**)
>
> -   Check if the reported fan speed matches the real (physical) fan
>     speed using **nbfc status**
>
> -   If this is the case, restart the service in write-mode using
>     **sudo nbfc restart**
>
> -   Now you can change the fan speed. See **USAGE**

## Finding a suitable configuration

**Automatic detection**

> To automatically select a configuration that matches your notebook
> model name, run:
>
> > **sudo nbfc config \--set auto**
>
> If this command does not print out an error message, the service has
> been configured successfully.

**Configuration rating**

> To get a list of configurations that are safe to execute on the
> current system and may work on the device, run:
>
> > **sudo nbfc rate-config \--all**

**Configurations with similar names**

> To get a list of configuration files with a name similar to the
> current notebook model, run:
>
> > **nbfc config \--recommend**
>
> **WARNING:** A similar model name does not mean that the configuration
> can be safely used on the device. **It can actually be dangerous.**
> Please only use configurations from this output if you are absolutely
> sure it is appropriate for your device.

## Converting an XML model configuration from the original NBFC

If you found a configuration that was made for the original NBFC
project, you can convert it using:

> **nbfc xml2json** *INFILE.xml* **\>** *OUTFILE.json*

In order to use the configuration, it has to be copied to
*/usr/share/nbfc/configs/*.

Such configurations may be available in the **NBFC-Revive** project, see
*https://github.com/UraniumDonut/nbfc-revive*.

# ADVANCED CONFIGURATION

## Changing temperature sensors attached to a fan

If a model configuration does not configure sensors for a fan, then the
default will be used, which is **\@CPU**, meaning all CPU-associated
sensors.

You may have to change the sensors attached to a fan, e.g. setting a fan
to the **\@GPU** sensor group.

To override the default sensors, use this command:

> **sudo nbfc sensors set -f** *FAN* \[**-s** *SENSOR\...*\] \[**-a**
> *ALGORITHM*\]

*ALGORITHM* is one of:

> -   **Min** (take the minimum temperature among all specified sensors)
>
> -   **Max** (take the maximum temperature among all specified sensors)
>
> -   **Average** (calculate the average temperature among all specified
>     sensors; default)

*SENSOR* may either be a:

> -   **temperature file**
>
> -   **sensor name** (which will result in one or more temperature
>     files)
>
> -   **sensor group **(**\@CPU**, **\@GPU**)

**NOTE:** If you are writing model configuration files, use the
**Sensors** field. It is recommended to use sensor groups, so that the
configuration works on many systems regardless of the hardware.

## Showing sensors attached to a fan

To show all fans with their configured sensors, run:

> **nbfc sensors show**

## Finding all available sensors

To get a list of all available sensors, run:

> **nbfc sensors list**

# USAGE

## Getting the service status

To get information about the service, including the notebook\'s fans,
run:

> **nbfc status**

## Setting the fan speed

To set the speed of all available fans, run:

> **nbfc set -s** *PERCENT*

To set all available fans to profile based automatic control, run:

> **nbfc set -a**

To apply the fan speed to only a specific fan, run:

> **nbfc set -s** *PERCENT* **-f** *FAN*

**NOTE:** *FAN* is a number starting from zero.

# MISCELLANEOUS

## Starting NBFC-Linux automatically at boot time

NBFC-Linux comes with a **systemd** service file.

To enable NBFC-Linux to start on boot, run:

> **sudo systemctl enable nbfc_service.service**

## Resetting the notebook\'s fans to the default behaviour

If the model configuration file for the notebook has been written
properly, then

> **sudo nbfc stop**\
> - or -\
> **sudo systemctl stop nbfc_service**

should reset the notebook\'s fans to their default behaviour.

In 99% of cases, a reboot will also reset the embedded controller.

# TROUBLESHOOTING

## Error: No such configuration available

The selected config file (as referenced by **SelectedConfigId** in
*/etc/nbfc/nbfc.json)* does not exist.

## Error: No working implementation found for accessing the embedded controller

Neither **ec_sys** nor **acpi_ec** kernel modules are available. Access
through */dev/port* is also not possible.

## Error: /sys/kernel/debug/ec/ec0/io: Invalid argument

The **ec_sys** kernel module is loaded, but **write_support** is
disabled.

Edit */etc/default/grub* and find the line
**GRUB_CMDLINE_LINUX_DEFAULT** then append **ec_sys.write_support=1** to
it.

For example, the line should look like this:
**GRUB_CMDLINE_LINUX_DEFAULT=\"quiet kvm.enable_virt_at_load=0
ec_sys.write_support=1\"**

Then run **sudo update-grub** and reboot your system.

## Error: Could not load kernel module \"acpi_call\"

The **acpi_call** kernel module is missing. Install or build it as
needed.

## Error: /proc/acpi/call: AE_NOT_FOUND

An unknown ACPI method was invoked. It usually indicates an incorrect or
incompatible configuration.

# NOTEBOOK FIRMWARE BACKGROUND

## What is an embedded controller (EC)?

The embedded controller is a microcontroller within modern notebook
computers. It manages system hardware functions such as fan speed,
battery charging, keyboard control, LCD backlight and many more.

## What is ACPI?

The Advanced Configuration and Power Interface (ACPI) is an open
industry standard for device configuration and power management. ACPI
methods allow the operating system to control various hardware
components.

# CONFIGURATION DEVELOPMENT

## Safety Considerations

Only interact with EC registers or ACPI methods you fully understand.
Misuse can damage hardware. If possible, remove the battery while
creating a configuration, as accidental modification to battery
registers may brick the battery permanently.

## Naming configuration files

Use the output of **nbfc get-model-name** as the configuration\'s
filename.

## How to analyze the firmware?

To get a list of all registers that are accessible through the embedded
controller, run:

> **sudo nbfc acpi-dump ec-registers**

To get a list of all embedded controller registers, run:

> **sudo nbfc acpi-dump registers**

**WARNING:** Do not try to access those registers through **ec_probe**,
as they are not exposed through the embedded controller. Only write to
registers that are listed by **sudo nbfc acpi-dump ec-registers**.

To get a list of all ACPI methods, run:

> **sudo nbfc acpi-dump methods**

To disassemble all the ACPI firmware tables, run:

> **sudo nbfc acpi-dump dsl**

## How to identify fan registers?

Search for registers named like **FAN**, **RPM**, **PWM**, etc.

Examples: **SFAN CFAN PFAN FAN1 FSW1 FRDC FTGC RPM1 RPM2**

## How to identify ACPI methods?

Search for methods named like **FAN**, **RPM**, **PWM**, etc.

Examples: **FANG FANW GFSD SFSD SFNV FRSP STMM GFRM**

## Accessing the embedded controller

For reading a register:

> **sudo ec_probe read** *REGISTER*

For reading a 16-bit register:

> **sudo ec_probe read \--word** *REGISTER*

For writing to a register:

> **sudo ec_probe write** *REGISTER* *VALUE*

For writing to a 16-bit register:

> **sudo ec_probe write \--word** *REGISTER* *VALUE*

## Invoking ACPI methods

To call an ACPI method, run:

> **sudo ec_probe acpi_call \'\\METHOD ARGS\...\'**

## Tracing embedded controller changes

It may be useful to trace the registers of the embedded controller as
the fan spins up and down.

To do so, make sure the fan is at its lowest speed, then run:

> **sudo ec_probe monitor -r** *OUTFILE.csv*

While recording, stress the CPU, run:

> **stress -c 8**

When the fan reached its maximum speed, stop the **stress** command and
wait until the fan is at its lowest speed.

You now can kill the **ec_probe monitor** command and inspect the
*OUTFILE.csv* for register changes.

You can also visualize the *OUTFILE.csv* by running:

> **ec_probe graph** *OUTFILE.csv*

## Writing complex configurations with Lua

Sometimes a firmware must be accessed in a more complex way than the
NBFC software supports. For example, ACPI commands have to be
constructed or the readings of the embedded controller have to be
modified.

See **nbfc_service.json**(5) for details.

## Firmware Fingerprinting

Firmware fingerprinting is especially useful if a model configuration
file uses **ACPI WMI** methods. Since these methods often have the same
names but not the same implementation it is necessary to look for
characteristics in the firmware that confirm it is the intended
firmware.

See **nbfc_service.json**(5) for details.

## Getting Help

See **nbfc_service.json**(5) for details.

For more information, see the **Creating Model Configuration HowTo**:
*https://nbfc-linux.github.io/creating-config/*.

For further assistance, visit the project at GitHub:
*https://github.com/nbfc-linux/nbfc-linux*.

# CONTRIBUTING

## Contributing a configuration file

When contributing a configuration file, please also attach your
firmware. You can create an archive containing your firmware by running:

> **sudo nbfc support \--create-archive** *ARCHIVE.tar.gz*

This helps the developer validate your configuration and keep the
register-name database up to date.

## Contributing code

Before contributing code, please open an issue and ask if the
contribution is wanted. This helps avoid frustration and prevents
unnecessary work.

## Uploading your firmware

Uploading your notebook firmware helps the developer to understand
existing configuration files.

The following data will be transmitted:

> -   The **laptop manufacturer**
>     (*/sys/devices/virtual/dmi/id/sys_vendor*)
>
> -   The **laptop model** (*/sys/devices/virtual/dmi/id/product_name*)
>
> -   The **ACPI DSDT** firmware table
>     (*/sys/firmware/acpi/tables/DSDT*)
>
> -   The **ACPI SSDT** firmware tables
>     (*/sys/firmware/acpi/tables/SSDT\**)

The preferred method for uploading your notebook firmware is to use:

> **sudo nbfc support \--upload-firmware**

However, if you\'re paranoid, you can upload it manually using **curl**:

> **nbfc support \--print-command**

# AUTHOR

Benjamin Abendroth (braph93@gmx.de)

# SEE ALSO

**nbfc**(1), **nbfc_service**(1), **nbfc_service.json**(5),
**ec_probe**(1)
