{
  inputs = {
    nixpkgs.url = "github:nixOS/nixpkgs/release-23.05";
    utils.url = "github:numtide/flake-utils";
  };
  outputs = {
    nixpkgs,
    utils,
    ...
  }:
    utils.lib.eachSystem ["i686-linux" "x86_64-linux"]
    (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in rec {
      packages = {
        nbfc = pkgs.stdenv.mkDerivation {
          name = "nbfc";
          version = "0.3.19";

          src = nixpkgs.lib.cleanSource ./.;

          nativeBuildInputs = with pkgs; [
            autoreconfHook
            curl
          ];

          configureFlags = [
            "--prefix=${placeholder "out"}"
            "--sysconfdir=/etc"
            "--bindir=${placeholder "out"}/bin"
          ];
        };
        default = packages.nbfc;
      };
    });
}
