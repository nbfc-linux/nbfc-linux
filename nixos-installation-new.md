# Installing on NixOS

## For people on stable
Flakes must be enabled to follow this guide, if you do not want to enable flakes check [this installation guide](./nixos-installation.md)


Add this flake to inputs in your configuration:

```nix
# flake.nix
{
  inputs = {
    # ...
    nbfc-linux = {
      url = "github:nbfc-linux/nbfc-linux";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    # ...
  };

  outputs = {
    self,
    nixpkgs,
    home-manager,
    ...
  } @ inputs: {
    # ...
   };
  };
}

```

After this follow the common instructions

## For people on unstable

nbfc-linux is already available in the nix repo so just follow the common instructions

## Common instructions 
- Mention it in the packages + enable nbfc service

```nix
# nbfc.nix
{
  config,
  inputs,
  pkgs,
  ...
}: let
  myUser = "gaurav"; #adjust this to your username
  command = "bin/nbfc_service --config-file '/home/${myUser}/.config/nbfc.json'";
in {
  environment.systemPackages = with pkgs; [
    # if you are on stable uncomment the next line
    # inputs.nbfc-linux.packages.x86_64-linux.default
    # if you are on unstable uncomment the next line
    # nbfc-linux
  ];
  systemd.services.nbfc_service = {
    enable = true;
    description = "NoteBook FanControl service";
    serviceConfig.Type = "simple";
    path = [pkgs.kmod];

    # if you are on stable uncomment the next line
    #  script = "${inputs.nbfc-linux.packages.x86_64-linux.default}/${command}";
    # if you are on unstable uncomment the next line
    #script = "${pkgs.nbfc-linux}/${command}";
   
    wantedBy = ["multi-user.target"];
  };
}
```

- Import nbfc.nix into configuration.nix
```nix
# configuration.nix
 imports = [
     # ...
    ./nbfc.nix
  ];
```

### Chose configuration file

Now if you try to set config the normal way you will encounter the following issue
```bash
$ sudo nbfc config --set "Asus Zenbook Flip UX360"
Error: [Errno 30] Read-only file system: '/nix/store/is476j7dp9ggy8rrqcmx68dpj21n3v0f-nbfc-linux/etc/nbfc/nbfc.json'
```

Solution:
```json
{"SelectedConfigId": "<your notebook>"}
```
Write this to `~/.config/nbfc.json` (or your location of choice)

Insert the [file name of the config file](https://github.com/nbfc-linux/nbfc-linux/tree/main/share/nbfc/configs) (without ".json"), *not* the `NotebookModel` value inside that config file.

example:
```json
{"SelectedConfigId": "Asus Zenbook Flip UX360UAK"}
```

This usage of `--config-file` option of nbfc may also help on similar distros. 

