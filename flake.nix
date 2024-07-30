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

          nativeBuildInputs = with pkgs; [
            autoreconfHook
            libsForQt5.qt5.wrapQtAppsHook
            libsForQt5.qt5.qtbase
          ];
          propagatedBuildInputs = with pkgs; [
            (python3.withPackages (pythonPackages: with pythonPackages; [
              pyqt5
            ]))
          ];
          dontWrapQtApps = true;

          preFixup = ''
            wrapQtApp "$out/bin/nbfc-qt"
          '';

          configureFlags = [
            "--prefix=${placeholder "out"}"
            "--sysconfdir=${placeholder "out"}/etc"
            "--bindir=${placeholder "out"}/bin"
          ];
        };
        default = packages.nbfc;
      };
    });
}
