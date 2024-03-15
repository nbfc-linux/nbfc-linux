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
          name = "nbfc-linux";
          version = "0.1.15";

          src = nixpkgs.lib.cleanSource ./.;

          buildFlags = ["PREFIX=$(out)" "confdir=/etc"];
          installFlags = ["PREFIX=$(out)" "confdir=$(out)/etc"];
          installTargets = ["install-core" "install-configs" "install-docs" "install-completion"];
        };
        default = packages.nbfc;
      };
    });
}
