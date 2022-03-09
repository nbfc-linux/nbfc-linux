{
  inputs = {
    nixpkgs.url = github:nixOS/nixpkgs;
    utils.url = github:numtide/flake-utils;
  };
  outputs = { nixpkgs, utils, ... }:
    utils.lib.eachSystem [ "i686-linux" "x86_64-linux" ]
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
          lib = nixpkgs.lib;

          buildPackage = { client ? "py" }:
            assert lib.assertMsg (builtins.elem client [ "py" "python" "c" ]) "Client implementation is only available in Python and C";

            pkgs.stdenv.mkDerivation {
              name = "nbfc-linux";
              version = "0.1.6";
              src = lib.cleanSource ./.;

              buildInputs = lib.optional (builtins.elem client [ "py" "python" ]) pkgs.python3;
              buildFlags = [ "PREFIX=$(out)" "confdir=/etc" ];

              installPhase =
                let
                  installFlags = [ "PREFIX=$out" ];
                in
                ''
                  make ${builtins.concatStringsSep " " installFlags}\
                       install-core \
                       install-client-${if (client == "c") then "c" else "py" }\
                       install-configs\
                       install-docs\
                       install-completion
                '';
            };
        in
        rec {
          packages = rec {
            nbfc = buildPackage { client = "py"; };
            nbfc-client-c = buildPackage { client = "c"; };
          };
          defaultPackage = packages.nbfc;
        });
}
