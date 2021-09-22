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

    buildPhase = ''
      make sharedir=$out/share -C src
    '';

    installPhase = ''
      mkdir -p $out/bin
      install nbfc.py           $out/bin/nbfc
      install src/nbfc_service  $out/bin/nbfc_service
      install src/ec_probe      $out/bin/ec_probe
  #   install src/nbfc          $out/bin/nbfc   #client written in c   
      mkdir -p $out/share/nbfc/configs
      cp -r share/nbfc/configs/* $out/share/nbfc/configs
      
      mkdir -p $out/share/man/man1
      mkdir -p $out/share/man/man5
      cp doc/ec_probe.1            $out/share/man/man1
      cp doc/nbfc.1                $out/share/man/man1
      cp doc/nbfc_service.1        $out/share/man/man1
      cp doc/nbfc_service.json.5   $out/share/man/man5
      
      mkdir -p $out/share/zsh/site-functions
      cp completion/zsh/_nbfc                $out/share/zsh/site-functions/
      cp completion/zsh/_nbfc_service        $out/share/zsh/site-functions/
      cp completion/zsh/_ec_probe            $out/share/zsh/site-functions/
      mkdir -p $out/share/bash-completion/completions
      cp completion/bash/nbfc                $out/share/bash-completion/completions/
      cp completion/bash/nbfc_service        $out/share/bash-completion/completions/
      cp completion/bash/ec_probe            $out/share/bash-completion/completions/
      mkdir -p $out/share/fish/completions
      cp completion/fish/nbfc.fish           $out/share/fish/completions/
      cp completion/fish/nbfc_service.fish   $out/share/fish/completions/
      cp completion/fish/ec_probe.fish       $out/share/fish/completions/
    '';
    };
  });
}
