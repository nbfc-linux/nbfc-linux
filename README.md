NoteBook FanControl
===================

This is a C port of [Stefan Hirschmann's](https://github.com/hirschmann) [NoteBook FanControl](https://github.com/hirschmann/nbfc).

It provides the same utilities with the same interfaces as the original NBFC, although the implementation differs.

If you find my work helpful, you can show your appreciation by [buying me a coffee](https://paypal.me/BenjaminAbendroth).

Table of Contents
-----------------

- [Comparison of NBFC C# and NBFC Linux](#comparison-of-nbfc-c-and-nbfc-linux)
- [Installation](#installation)
- [Getting started with the GUI](#getting-started-with-the-gui)
- [Getting started without the GUI](#getting-started-without-the-gui)
- [Advanced configuration](#advanced-configuration)
- [Differences in detail](#differences-in-detail)
- [Troubleshooting](#troubleshooting)
- [Advanced Compilation](#advanced-compilation)
- [Shell autocompletion](#shell-autocompletion)
- [Contributing](#contributing)

Comparison of NBFC C# and NBFC Linux
------------------------------------

|What                             | NBFC Mono                             | NBFC Linux                                  |
|---------------------------------|---------------------------------------|----------------------------------------------
|Portability                      | Crossplatform                         | Linux                                       |
|Configuration files              | [XML](https://github.com/hirschmann/nbfc/tree/master/Configs) ![Files](https://img.shields.io/github/directory-file-count/hirschmann/nbfc/Configs) | [JSON](share/nbfc/configs) ![Files](https://img.shields.io/github/directory-file-count/nbfc-linux/nbfc-linux/share%2Fnbfc%2Fconfigs) |
|Model compatibility database     | No                                    | [Yes](share/nbfc/model_support.json)        |
|Runtime                          | Mono                                  | Native                                      |
|Memory consumption (ps\_mem)     | ~50MB                                 | ~280KB                                      |
|Package size (pkg.tar.gz)        | 448K                                  | 100K                                        |
|Fan control rights               | Any user                              | Any user                                    |
|Service control rights           | Any user                              | Only root                                   |
|IPC Concept                      | TCP/IP                                | Unix sockets                                |
|IPC Protocol                     | Binary                                | JSON                                        |
|Graphical User Interface         | Windows only                          | Linux only                                  |

The [service](doc/nbfc_service.1.md), the [client](doc/nbfc.1.md) and the [probing tool](doc/ec_probe.1.md) are written in C.

Installation
------------

- Arch Linux:
  - [Latest Version 0.3.14](https://github.com/nbfc-linux/nbfc-linux/releases/download/0.3.14/nbfc-linux-git-0.3.13.r1.g5a73269-1-x86_64.pkg.tar.zst)
  - Install curl: `pacman -S curl`
  - Install NBFC-Linux: `pacman -U ./nbfc-linux-git-0.3.13.r1.g5a73269-1-x86_64.pkg.tar.zst`
  - Or via AUR (`yay -S nbfc-linux`)
  - Or by using the PKGBUILD [nbfc-linux-git](pkgbuilds/nbfc-linux-git/PKGBUILD)

- Debian / Ubuntu:
  - [Latest Version 0.3.14](https://github.com/nbfc-linux/nbfc-linux/releases/download/0.3.14/nbfc-linux_0.3.14_amd64.deb)
  - Install libcurl: `apt install libcurl4`
  - Install NBFC-Linux: `dpkg -i ./nbfc-linux_0.3.14_amd64.deb`

- Fedora:
  - [Latest Version 0.3.14](https://github.com/nbfc-linux/nbfc-linux/releases/download/0.3.14/fedora-nbfc-linux-0.3.14-1.x86_64.rpm)
  - Install libcurl: `dnf install libcurl`
  - Install NBFC-Linux: `dnf install ./fedora-nbfc-linux-0.3.14-1.x86_64.rpm`

- OpenSuse (Tumbleweed)
  - [Latest Version 0.3.14](https://github.com/nbfc-linux/nbfc-linux/releases/download/0.3.14/opensuse-nbfc-linux-0.3.14-1.x86_64.rpm)
  - Install libcurl: `zypper install libcurl4`
  - Install NBFC-Linux: `zypper install ./opensuse-nbfc-linux-0.3.14-1.x86_64.rpm`

- NixOS:
  - [Declaratively](nixos-installation-new.md)
  - [Imperatively](nixos-installation.md) (with `nix profile install` or `nix-env -if .`)

- Fedora - this will not build the current working directory, but downloads a source archive from chosen release on github and builds an installable rpm:
  - ensure, that podman is installed
    - docker should work also, but is currently not tested
  - edit [pkgbuilds/rpm/nbfc-linux.spec](./pkgbuilds/rpm/nbfc-linux.spec) and
    - set wanted Version (e.g. 0.3.14)
    - set wanted Releasenumber (e.g. 1%{?dist} or 2%{?dist})
  - `./pkgbuilds/rpm/buildNBFC-LINUX`
  - the resulting rpm can be found in [pkgbuilds directory](./pkgbuilds/)

- In general:
  - `./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && sudo make install`

- For systems with OpenRC (gentoo):
  - `./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --with-init-system=openrc && make && sudo make install`

- For systems with System-V-Init:
  - `./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --with-init-system=systemv && make && sudo make install`

Getting started with the GUI
----------------------------

A graphical user interface (GUI) is available as a separate project.

You can find it on GitHub: [nbfc-qt](https://github.com/nbfc-linux/nbfc-qt)

Getting started without the GUI
-------------------------------

When running NBFC for the first time, you need to give it a configuration file for your laptop model.

First, you should fetch new configuration files using `sudo nbfc update`.

If you are lucky, `sudo nbfc config --set auto` will find a matching one and set it.

`sudo nbfc config --recommend` will compare your DMI system-product-name to the available configuration file names and print a list of descending similarity.

With `sudo nbfc config --set <MODEL>` a configuration is selected.

`sudo nbfc start` will start the service.

It can be queried by `nbfc status -a`.

If you wish `nbfc_service` to get started on boot, use `sudo systemctl enable nbfc_service`.

Advanced configuration
----------------------

NBFC-Linux allows you to specify which temperature sources to use for controlling fans and the algorithm to compute the temperature.

> NOTE: Since version 0.3.3 NBFC-Linux also allows you to specify sensors in FanConfigurations using the `Sensors` field

**Default Configuration**

If no configuration is specified, NBFC uses the "Average" algorithm and utilizes all sensor files named "coretemp", "k10temp", or "zenpower".

**Available Algorithms**

You can choose from three different algorithms to compute the temperature:

- *"Average"*: Computes the average temperature from all specified sources.
- *"Min"*: Selects the lowest temperature among all specified sources.
- *"Max"*: Selects the highest temperature among all specified sources.

**Specifying Temperature Sources**

You can specify temperature sources either by a
- *sensor name* (which may result in multiple temperature sources)
- *temperature file*: A file containing the temperature (usually named `temp*_input`)
- *shell command*: A command whose output is used as temperature (the command has to be prefixed by `$`)
- *sensor group*:
  - *@CPU*: Uses all sensors named "coretemp", "k10temp" or "zenpower"
  - *@GPU*: Uses all sensors named "amdgpu", "nvidia", "nouveau" or "radeon"

**Example Configuration**

Here is a fictional example demonstrating how to configure NBFC-Linux:

```
{
    "SelectedConfigId": "Asus G53SX",
    "TargetFanSpeeds": [ -1.000000 ],
    "FanTemperatureSources": [
        {
            "FanIndex": 0,
            "TemperatureAlgorithmType": "Min",
            "Sensors": [ "coretemp" ]
        },
        {
            "FanIndex": 1,
            "TemperatureAlgorithmType": "Average",
            "Sensors": [ "nouveau" ]
        },
        {
            "FanIndex": 2,
            "TemperatureAlgorithmType": "Average",
            "Sensors": [ "/sys/class/hwmon/hwmon4/temp2_input", "/sys/class/hwmon/hwmon4/temp3_input" ]
        },
        {
            "FanIndex": 3,
            "Sensors": [ "$ echo 42" ]
        },
        {
            "FanIndex": 4,
            "Sensors": [ "@GPU" ]
        }
    ]
}
```

In this example:

- *Fan 0* uses the "Min" algorithm with sensors named "coretemp".
- *Fan 1* uses the "Average" algorithm with sensors named "nouveau".
- *Fan 2* uses the "Average" algorithm with specific sensor file paths.
- *Fan 3* uses the output of `echo 42` as temperature
- *Fan 4* uses all sensors found in the `@GPU` group ("amdgpu", "nvidia", "nouveau" or "radeon")

Differences in detail
---------------------

|Files                            | NBFC Mono                             | NBFC Linux                                  |
|---------------------------------|---------------------------------------|----------------------------------------------
|Systemd service file             | nbfc.service                          | nbfc\_service.service                       |
|EC Probing tool                  | ec-probe                              | ec\_probe                                   |
|Notebook configuration files     | /opt/nbfc/Configs/\*.xml              | /usr/share/nbfc/configs/\*.json             |
|Service binary                   | /opt/nbfc/nbfcservice.sh              | /bin/nbfc\_service                          |
|PID File                         | /run/nbfc.pid                         | /run/nbfc\_service.pid                      |
|Config file                      | ?                                     | /etc/nbfc/nbfc.json                         |

- The original NBFC service adjusts the fan speeds in intervals of `EcPollIntervall` according to `TemperatureThresholds`. - NBFC Linux directly sets the fan speed (also according to `TemperatureThresholds`).

- The original NBFC service selects a TemperatureThreshold and applies its `FanSpeed` when the temperature exceeds its `UpThreshold`. In contrast, NBFC Linux will select the *next* TemperatureThreshold and apply its `FanSpeed` when the temperature exceeds the *current* `UpThreshold`. The provided config files have been reconfigured to account for this change, so that they provide the same behaviour as the original NBFC service. If you have a custom config file that works well with the original service, you can port it to NBFC Linux using the [provided tool](/tools/config_to_json.py) (requires python3-lxml).

- NBFC Linux dropped the `Autostart` option, since it relies on the systemd service file only.

Troubleshooting
---------------
The preferred way of running nbfc is using the `ECSysLinux` implementation, which depends on the `ec_sys` kernel module.
There is also an alternative implementation which uses `/dev/port`, called `dev_port`.
It can be specified on the commandline using `--embedded-controller=dev_port` and permanently set in `/etc/nbfc/nbfc.json` with `"EmbeddedControllerType": "dev_port"`.

Many Linux distributions do not provide the `ec_sys` module, and the module should be compiled manually. Alternatively, the [`acpi_ec`](https://github.com/MusiKid/acpi_ec) module can be used. The `acpi_ec` module comes with a DKMS config script, which automatically rebuilds the `acpi_ec` module when a new kernel is installed and supports running NBFC with Secure Boot and Lockdown Kernel. NBFC-Linux will try to use the `acpi_ec` module if available. It can be set with `"EmbeddedControllerType": "acpi_ec"`.

Advanced Compilation
--------------------

NBFC-Linux allows disabling features at compile time to produce an even smaller binary.

Pass the following flags to `./configure` to disable specific components.

- `--disable-syslog`: Don't log to syslog in daemon mode
- `--disable-ec-debug`: Disable debugging of reads/writes from/to the embedded controller
- `--disable-ec-dummy`: Disable the dummy embedded controller implementation
- `--disable-ec-dev-port`: Disable the /dev/port embedded controller implementation
- `--disable-ec-sys`: Disable the embedded controller implementation that relies on `ec_sys` kernel module
- `--disable-ec-acpi`: Disable the embedded controllern implementation that relies on the `acpi_ec` kernel module

Shell autocompletion
--------------------

NBFC-Linux comes with shell completion scripts for bash, fish and zsh.

The completion files are generated using [crazy-complete](https://github.com/crazy-complete/crazy-complete).

```
~ $ nbfc_service <TAB>
--config-file          -c  -- Use alternative config file (default /etc/nbfc/nbfc.json)
--debug                -d  -- Enable tracing of reads and writes of the embedded controller
--embedded-controller  -e  -- Specify embedded controller to use
--fork                 -f  -- Switch process to background after sucessfully started
--help                 -h  -- show this help message and exit
--read-only            -r  -- Start in read-only mode
--state-file           -s  -- Write state to an alternative file (default /var/run/nbfc_service.state.json)

~ $ nbfc <TAB>
config   -- List or apply configs
help     -- Show help
restart  -- Restart the service
set      -- Control fan speed
start    -- Start the service
status   -- Show the service status
stop     -- Stop the service
```

Contributing
------------

You can write a configuration file for your laptop model. See the [Wiki of the original NBFC](https://github.com/hirschmann/nbfc/wiki/How-to-create-a-NBFC-config) for more details.

Please **do not** create issues on GitHub requesting a configuration file for your laptop!

If you found a working configuration for your laptop model, you can register it in the [Compatibility Database](share/nbfc/model_support.json).

If you want to write a program that controls the NBFC service, see the [protocol](/PROTOCOL.md).

Developers are welcome to try optimizing the code of the service for memory usage. However, be warned that it is likely a waste of time, as the author has already tried every possible approach. It seems that every line of code is necessary for the current functionality.

See also the documentation about the [nbfc configuration](doc/nbfc_service.json.5.md).

