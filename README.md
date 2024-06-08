NoteBook FanControl
===================

This is a C port of [Stefan Hirschmann's](https://github.com/hirschmann) [NoteBook FanControl](https://github.com/hirschmann/nbfc).

It provides the same utilities with the same interfaces as the original NBFC, although the implementation differs.

If you find my work helpful, you can show your appreciation by [buying me a coffee](https://paypal.me/BenjaminAbendroth).

Comparison of NBFC C# and NBFC Linux
------------------------------------

|What                             | NBFC Mono                             | NBFC Linux                                  |
|---------------------------------|---------------------------------------|----------------------------------------------
|Portability                      | Crossplatform                         | Linux                                       |
|Configuration files              | XML (956KB)                           | [JSON](share/nbfc/configs) (840KB)          |
|Runtime                          | Mono                                  | Native                                      |
|Memory consumption (ps\_mem)     | ~50MB                                 | ~350KB                                      |
|Package size (pkg.tar.gz)        | 448K	                              | 100K                                        |
|Service control rights           | Any user                              | Only root                                   |
|IPC Concept                      | TCP/IP                                | TCP/IP                                      |
|IPC Protocol                     | Binary                                | JSON                                        |

All programs, the [service](doc/nbfc_service.1.md), the [client](doc/nbfc.1.md) and the [probing tool](doc/ec_probe.1.md) are written in C.

Installation
------------

- Arch Linux:
  - Either via AUR (`yaourt -S nbfc-linux`)
  - Or by using the PKGBUILD [nbfc-linux-git](pkgbuilds/nbfc-linux-git/PKGBUILD)

- Debian / Ubuntu (untested):
  - [Latest Version 0.2.6](https://github.com/nbfc-linux/nbfc-linux/releases/download/0.2.6/nbfc-linux_0.2.6_amd64.deb)
  - See [GitHub.com/nbfc-linux/packages/deb](https://github.com/nbfc-linux/packages/tree/main/deb) for all versions.

- Fedora (untestet)
  - [Latest Version 0.2.6](https://github.com/nbfc-linux/nbfc-linux/releases/download/0.2.6/nbfc-linux-0.2.6-1.x86_64.rpm)
  - See [GitHub.com/nbfc-linux/packages/rpm](https://github.com/nbfc-linux/packages/tree/main/rpm) for all versions.

- NixOS:
  - [Declaratively](nixos-installation-new.md)(with nix flakes)
  - [Imperatively](nixos-installation.md) (with `nix profile install` or `nix-env -if .`)

- Fedora - this will not build the current working directory, but downloads a source archive from chosen release on github and builds an installable rpm:
  - ensure, that podman is installed
    - docker should work also, but is currently not tested
  - edit [pkgbuilds/rpm/nbfc-linux.spec](./pkgbuilds/rpm/nbfc-linux.spec) and 
    - set wanted Version (e.g. 0.1.15)
    - set wanted Releasenumber (e.g. 1%{?dist} or 2%{?dist})
  - `./pkgbuilds/rpm/buildNBFC-LINUX`
  - the resulting rpm can be found in [pkgbuilds directory](./pkgbuilds/)

- In general:
  - `./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc && make && sudo make install`

- For systems with OpenRC (gentoo):
  - `./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --with-init-system=openrc && make && sudo make install`

- For systems with System-V-Init:
  - `./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --with-init-system=systemv && make && sudo make install`

Getting started
---------------

When running NBFC for the first time, you need to give it a configuration file for your laptop model.

If you are lucky, `sudo nbfc config --set auto` will find a matching one and set it.

`sudo nbfc config --recommend` will compare your DMI system-product-name to the available configuration file names and print a list of descending similarity.

With `sudo nbfc config --set <MODEL>` a configuration is selected.

`sudo nbfc start` will start the service.

It can be queried by `sudo nbfc status -a`.

If you wish `nbfc_service` to get started on boot, use `sudo systemctl enable nbfc_service`.


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

Shell autocompletion
--------------------

NBFC-Linux comes with shell completion scripts for bash, fish and zsh.

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

If you want to write a program that controls the NBFC service, see [protocol](/PROTOCOL.md).

See also the documentation about the [nbfc configuration](doc/nbfc_service.json.5.md).
