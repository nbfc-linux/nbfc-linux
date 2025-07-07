# Changelog

## nbfc-linux-0.3.20 (2025-06-06)
- Added support for calling ACPI methods

  NBFC-Linux can now call ACPI methods to read or write the fan speed.
  ACPI methods are also supported in RegisterWriteConfigurations.
  This is useful if the fan cannot be controlled via simple EC register
  manipulations.

  In `FanConfiguration`:
    - `ReadAcpiMethod`: Read the fan speed using an ACPI method
    - `WriteAcpiMethod`: Write the fan speed using an ACPI method
    - `ResetAcpiMethod`: Reset the fan using an ACPI method

  In `RegisterWriteConfiguration`:
    - `AcpiMethod` (requires `WriteMode` set to `Call`)
    - `ResetAcpiMethod` (requires `ResetWriteMode` set to `Call`)

  The `ec_probe` tool now supports calling methods using the `acpi_call` command.

  This requires the `acpi_call` module to be installed.

## nbfc-linux-0.3.18 (2025-05-24)
- New model configuration files

## nbfc-linux-0.3.17 (2025-05-22)
- Using state file to store fan speeds

  Previously, `TargetFanSpeeds` were stored in the main configuration file,
  which required rewriting the file whenever fan speeds changed.

  This could lead to conflicts if the configuration was being edited
  simultaneously via the GUI.

  Fan speeds are now stored in a separate state file to avoid these issues
  and ensure safe, concurrent updates.

## nbfc-linux-0.3.16 (2025-05-10)
- Added `nbfc sensors` command

## nbfc-linux-0.3.15 (2025-05-08)
- Added `nbfc warranty` and `nbfc donate` commands

## nbfc-linux-0.3.14 (2025-05-05)
- The GUI is no longer part of this project.
  It has been moved to https://github.com/nbfc-linux/nbfc-qt

## nbfc-linux-0.3.12 (2025-05-04)
- Fix possible bug while logging to syslog()
- Log to STDERR even when run as a daemon
- Hopefully fixing a bug while building with rpmbuild

## nbfc-linux-0.3.11 (2025-05-01)
- Fix incorrect error message if TemperatureThresholds are missing

## nbfc-linux-0.3.10 (2025-04-30)
- Added support for temperature monitoring using the proprietary NVIDIA
  kernel module through dynamic loading of `libnvidia-ml.so` via `dlopen()`.

  Users can now enable this sensor by specifying `nvidia-ml` as the sensor name.

  This sensor is also part of the `@GPU` sensor group.

## nbfc-linux-0.3.9 (2025-04-26)
- Support for logging though syslog facility

## nbfc-linux-0.3.8 (2025-04-26)
- Comments can now be added to any JSON object in the configuration files
  by using the "Comment" field.

## nbfc-linux-0.3.7 (2025-04-26)
- Improved the forking and daemonization process
- Dropped the `pipe(), write(), read()` logic that has been used to communicate
  between parent and child process.
- Using `setsid()` to fully daemonize on fork()
- Using `chdir()` to change to root directory ("/")

## nbfc-linux-0.3.6 (2025-04-26)
- Server now doesn't use threads to handle client connections. This saves some RAM.
- `select()` has been replaced by `poll()`.
- Incoming messages have a maximum size. This prevents the server of exiting
  in case of receiving huge messages.

## nbfc-linux-0.3.5 (2025-04-25)
- Server now runs in the main thread.
  This saves around 60KB of RAM.

## nbfc-linux-0.3.4 (2025-04-23)
- Now sensor groups can be specified as temperature sources.
  Available groups are `@CPU` and `@GPU`.
  Since notebook models may have different graphics cards, using `@GPU`
  provides a portable way to refer to the GPU temperature.

## nbfc-linux-0.3.3 (2025-04-21)
- FanConfigurations in the ModelConfig can now define their own sensors
  and specify the TemperatureAlgorithmType.

## nbfc-linux-0.3.2 (2025-04-17)
- Now user defined commands are allowed as sensors in `FanTemperatureSources`.
  If a sensor name is prefixed by `$`, its output will be used as temperature.

## nbfc-linux-0.3.1 (2025-04-17)
- Now user defined file paths are allowed as sensors in `FanTemperatureSources`

## nbfc-linux-0.3.0 (2025-04-16)
- Added command `nbfc update` to download new configuration files from the
  configuration file repository as needed.

## nbfc-linux-0.2.9 (2025-04-05)
- Added model compatibility database (`/usr/share/nbfc/model_support.json`):
  A file that maps each notebook model to its supported configuration.

## nbfc-linux-0.1.13 (2024-01-30)
- Added support for OpenRC and System-V-Init

## nbfc-linux-0.1.12 (2024-01-28)
- Added `LegacyTemperatureThresholdsBehaviour` option to model config

## nbfc-linux-0.1.11 (2024-01-23)
- Added `get_model_name` to NBFC client
- NBFC client now writes JSON in a beautified format
- NBFC client now doesn't depend on `dmidecode` anymore
- Manual pages, completion files and help texts of binaries are now handwritten
- HTML and Markdown documentation are now generated using pandoc from manual pages 
- NBFC's --readonly argument is renamed to --read-only
- `ec_probe watch` is now officially available
- NBFC service now logs build configuration on startup

## nbfc-linux-0.1.7 (2023-01-13)
- ec\_probe now respects the --embedded-controller option

## nbfc-linux-0.1.5 (2022-08-01)
- Updated nbfc.py to use /usr/share/nbfc/configs (fix for --recommend)

## nbfc-linux-0.1.4 (2022-08-01)
- Changed location of model configuration files to /usr/share/nbfc/configs
- Added files for installation in nixOS
- Fixed race-condition in systemd service file

## nbfc-linux-0.1.2 (2022-07-03)
- Added more debug information on service startup
- Maked `EmbeddedControllerType` configurable
- Dropped the usage of `lm_sensors.c` in favour of `fs_sensors.c` because of build problems on some platforms
- Moved `tools/argany` to a separate project: [argparse-tool](https://github.com/braph/argparse-tool)

## nbfc-linux-0.1.1 (2022-05-02)
- Fixed type `--config-fie` to `--config-file`
- Using default temperature thresholds if those in config are empty or not present.  [Issue #2](https://github.com/braph/nbfc-linux/issues/2#issue-897727519)

## nbfc-linux-0.1.0 (2021-05-19)
- Added CHANGELOG.md
- Added --version flags to all programs
- Added pre-commit hook
