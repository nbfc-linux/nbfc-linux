# Changelog

## nbfc-linux-0.1.5 (2012-08-01)
- Updated nbfc.py to use /usr/share/nbfc/configs (fix for --recommend)

## nbfc-linux-0.1.4 (2012-08-01)
- Changed location of model configuration files to /usr/share/nbfc/configs
- Added files for installation in nixOS
- Fixed race-condition in systemd service file

## nbfc-linux-0.1.2 (2012-07-03)
- Added more debug information on service startup
- Maked `EmbeddedControllerType` configurable
- Dropped the usage of `lm_sensors.c` in favour of `fs_sensors.c` because of build problems on some platforms
- Moved `tools/argany` to a separate project: [argparse-tool](https://github.com/braph/argparse-tool)

## nbfc-linux-0.1.1 (2012-05-02)
- Fixed type `--config-fie` to `--config-file`
- Using default temperature thresholds if those in config are empty or not present.  [Issue #2](https://github.com/braph/nbfc-linux/issues/2#issue-897727519)

## nbfc-linux-0.1.0 (2021-05-19)
- Added CHANGELOG.md
- Added --version flags to all programs
- Added pre-commit hook

## nbfc-linux-0.1.7 (2013-01-13)
- ec\_probe now respects the --embedded-controller option
