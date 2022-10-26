NixOS specific solution to get nbfc-linux installed

# Use a config-file (nbfc.json) in a writable path

This is to bypass this problem:
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

This usage of `--config-file` option of nbfc may also help on similar distros. [3]

# Set up service

include directive in configuration.nix:
```nix
{
  imports =
    [ # Include the results of the hardware scan.
      ./hardware-configuration.nix
      ./nbfc.nix #add this line
    ];
}
```

nbfc.nix:
Use either flake or fetchFromGitHub. If you use fetchFromGitHub, uncomment the commented lines accordingly.
```nix
#NoteBook FanControl
{ config, pkgs, ... }:

# Uncomment lines only if you don't use flakes to get nbfc-linux.
#
# flake vs fetchFromGitHub:
#
# - for disadvantages, + for advantages:
#
# - fetchFromGitHub: updates: have to manually change config file (revision, hash)
# + flake: updates: `nix profile upgrade` command
#
# flake install: `git clone https://github.com/nbfc-linux/nbfc-linux && cd nbfc-linux && nix --experimental-features 'nix-command flakes' profile install`
# Then the nbfc.nix only needs the `systemd.services` stuff, nothing else.

let
  myUser = "john"; #adjust this to your username
  command = "bin/nbfc_service --config-file '/home/${myUser}/.config/nbfc.json'";

# If you don't use flakes:
#  nbfc = pkgs.stdenv.mkDerivation {
#  name = "nbfc-linux";
#  version = "0.1.7";

#  src  = pkgs.fetchFromGitHub {
#    owner  = "nbfc-linux";
#    repo   = "nbfc-linux";
#    rev    = "4c2b75e4a875459e86a9892319889ff945e9cadf";
#    sha256 = "UxaL4V8FkA+eONCj7vTHAlRSJxoXqRB2aW7A/KJyvlY=";
#  };

#  buildFlags = [ "PREFIX=$(out)" "confdir=/etc" ];

#  installPhase =
#    let
#      installFlags = [ "PREFIX=$out" ];
#    in
#    ''
#      make ${builtins.concatStringsSep " " installFlags}\
#           install-core \
#           install-client-c\
#           install-configs\
#           install-docs\
#           install-completion
#    '';
# };
in
{

# If you don't use flakes:
#environment.systemPackages = with pkgs; [
#  nix-prefetch-github
#  nbfc
#];

# Needed for both flakes and fetchFromGitHub:
systemd.services.nbfc_service = {
  enable = true;
  description = "NoteBook FanControl service";
  serviceConfig.Type = "simple"; #related upstream config: https://github.com/nbfc-linux/nbfc-linux/blob/main/etc/systemd/system/nbfc_service.service.in
  path = [ pkgs.kmod ];
  # script = let package = nbfc.packages.${pkgs.system}.nbfc; in "${package}/${command}"; #for fetchFromGitHub
  script = "/home/${myUser}/.nix-profile/${command}"; #for flake
  wantedBy = [ "multi-user.target" ];
};

}
```

# Sources

The presented solution is [1] with these fixes to get it working:
- [2] (script line) (fix NixOS executable call)
- [3] (--config-file option) (fix nbfc "NotebookModel: Unknown option")
- [4] (modprobe) (fix NixOS causing "sh: modprobe: command not found")

[1]: configuration.nix, non-working attempt: https://www.reddit.com/r/NixOS/comments/wvk8rb/fan_control_for_laptop_from_github/
[2]: https://www.reddit.com/r/NixOS/comments/wvk8rb/comment/ilgitdk/
[3]: https://github.com/nbfc-linux/nbfc-linux/issues/54#issuecomment-1197342962
[4]: https://github.com/NixOS/nixpkgs/issues/9611#ref-commit-c726773 (later, "config.system.sbin.modprobe" was replaced by "pkgs.kmod", see blame of that file on master branch. Was hinted to by similar issue https://discourse.nixos.org/t/error-attribute-lib-missing/13534/2)
