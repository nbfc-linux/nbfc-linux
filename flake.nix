{
  inputs = {
    nixpkgs.url = "github:nixOS/nixpkgs/release-23.05";
    utils.url = "github:numtide/flake-utils";
  };
  outputs = {
    self,
    nixpkgs,
    utils,
    ...
  }:
    utils.lib.eachSystem ["i686-linux" "x86_64-linux"]
    (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in rec {
      packages = {
        nbfc-linux = pkgs.stdenv.mkDerivation {
          name = "nbfc-linux";
          version = "0.3.19";

          src = nixpkgs.lib.cleanSource ./.;

          nativeBuildInputs = with pkgs; [
            autoreconfHook
            curl
          ];

          configureFlags = [
            "--bindir=${placeholder "out"}/bin"
            "--prefix=${placeholder "out"}"
            "--sysconfdir=/etc"
          ];
        };
        default = packages.nbfc-linux;
      };

      # This corrects the mismatch between the package name and the command name
      apps.nbfc-linux = {
        type = "app";
        program = "${self.packages.${system}.nbfc-linux}/bin/nbfc";
      };

      # This tells `nix run` to use this package
      defaultApp = self.apps.${system}.nbfc-linux;
    });
}
