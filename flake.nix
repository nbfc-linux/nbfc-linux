{
  inputs = {
    nixpkgs.url = github:nixOS/nixpkgs/release-23.05;
    utils.url = github:numtide/flake-utils;
  };
  outputs = { nixpkgs, utils, ... }:
    utils.lib.eachSystem [ "i686-linux" "x86_64-linux" ]
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in
        rec {
          packages =
            let
              buildFlags = [ "PREFIX=$(out)" "confdir=/etc" ];
            in
            rec {
              nbfc = pkgs.stdenv.mkDerivation {
                inherit buildFlags;

                name = "nbfc-linux";
                version = "0.1.6";

                src = nixpkgs.lib.cleanSource ./.;

                installPhase =
                  let
                    installFlags = [ "PREFIX=$out" ];
                  in
                  ''
                    make ${builtins.concatStringsSep " " installFlags}\
                         install-core \
                         install-configs\
                         install-docs\
                         install-completion
                  '';
              };
            };
          defaultPackage = packages.nbfc;
        });
}
