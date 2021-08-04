{
  inputs = {
    nixpkgs.url = github:nixos/nixpkgs/nixpkgs-unstable;
    utils.url = github:numtide/flake-utils;
  };
  outputs = { nixpkgs, utils, ... }:
  utils.lib.eachSystem [ "i686-linux" "x86_64-linux" ] 
  ( system:
  let 
    pkgs = import nixpkgs {
      inherit system;
    };
  in 
  {
    defaultPackage = pkgs.stdenv.mkDerivation {
      name = "nbfc-linux";
      src = ./.;
      buildInputs = with pkgs; [
        python3
      ];
      buildPhase = "make";
      installPhase = ''
        mkdir -p $out
        make DESTDIR=$out install
      '';
    };
  });
}
