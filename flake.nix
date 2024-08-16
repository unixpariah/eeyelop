{
  description = "Seto - hardware accelerated keyboard driven screen selection tool";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    zls.url = "github:zigtools/zls";
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
            wayland-protocols
            libxkbcommon
            lua
            libGL
            glxinfo
            freetype
            fontconfig
            clang-tools
            zls.packages.${system}.default
            zig.packages.${system}."0.13.0"
          ];
        };

        packages.default =
          pkgs.callPackage ./default.nix {};
      }
    );
}
