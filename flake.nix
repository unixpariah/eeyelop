{
  description = "Eeyelop - Fancy keyboard driven notification daemon";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    zls.url = "github:zigtools/zls";
    hiv.url = "github:unixpariah/hiv";
    zig = {
      url = "github:mitchellh/zig-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    nixpkgs,
    zls,
    flake-utils,
    zig,
    hiv,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = import nixpkgs {
          inherit system;
        };
      in {
        devShell = pkgs.mkShell {
          packages = with pkgs; [
            pkg-config
            wayland
            wayland-scanner
            wayland-protocols
            libxkbcommon
            lua
            libGL
            glxinfo
            valgrind
            freetype
            fontconfig
            clang-tools
            cglm
            hiv.packages.${system}.default
            zls.packages.${system}.default
            zig.packages.${system}."0.13.0"
          ];
        };

        packages.default =
          pkgs.callPackage ./default.nix {};
      }
    );
}
