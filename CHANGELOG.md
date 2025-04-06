# Changelog

## nbfc-linux-0.2.9 (2025-05-04)
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
