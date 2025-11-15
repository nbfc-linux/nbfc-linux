# NixOS Installation

## Config file

Create a config with a writeable path of your choice, for example `~/.config/nbfc/nbfc.json`. Pick a name from the [list of configs](https://github.com/nbfc-linux/nbfc-linux/tree/main/share/nbfc/configs) (without ".json"), _not_ the `NotebookModel` value inside that config file. For example:

```json
{ "SelectedConfigId": "Asus Zenbook Flip UX360UAK" }
```

## Service

Include the following lines to set up `nbfc-linux` and the accompanying systemd service.

```nix
# Add nbfc-linux to system packages
environment.systemPackages = [pkgs.nbfc-linux];

# Point systemd service to the config-file
systemd.services.nbfc_service = {
  enable = true;
  description = "NoteBook FanControl service";
  serviceConfig = {
    Type = "simple";
    ExecStart = "${pkgs.nbfc-linux}/bin/nbfc_service --config-file /path/to/config.json";
    Restart = "on-failure";
  };
  path = [pkgs.kmod];
  wantedBy = ["multi-user.target"];
};
```
