{
  description = "nbfc - NoteBook FanControl ported to Linux Nix Flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "i686-linux"
      ];
      forEachSystem = nixpkgs.lib.genAttrs systems;
      pkgsForEach = nixpkgs.legacyPackages;
    in
    {
      packages = forEachSystem (
        system:
        let
          pkgs = pkgsForEach.${system};
        in
        {
          nbfc = pkgs.stdenv.mkDerivation {
            name = "nbfc-linux";
            version = "0.5.2";
            src = nixpkgs.lib.cleanSource ./../..;
            nativeBuildInputs = with pkgs; [
              autoconf
              automake
              pkg-config
              lua5_4
              curl
              libxml2
              openssl
            ];
            preConfigure = ''
              	    ./autogen.sh
              	    '';
            configureFlags = [
              "--bindir=${placeholder "out"}/bin"
            ];
          };
          default = self.packages.${system}.nbfc;
        }
      );
    };
}
