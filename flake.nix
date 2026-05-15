{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem
    (
      system: let
        pkgs = import nixpkgs {
          inherit system;
        };
      in
        with pkgs; {
          formatter = alejandra;
          devShells.default = mkShell.override {stdenv = llvmPackages_20.libcxxStdenv; } {
            buildInputs = [
              cmake
              zstd
              pkg-config
              clang
            ];
          };
        }
    );
}
