{
  description = "demacasa's Pebble OS projects";

  nixConfig = {
    extra-substituters = ["https://pebble.cachix.org"];
    extra-trusted-public-keys = ["pebble.cachix.org-1:aTqwT2hR6lGggw/rPISRcHZctDv2iF7ewsVxf3Hq6ow="];
  };

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    pebble.url = "github:pebble-dev/pebble.nix";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    pebble,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: {
      devShells.default = pebble.pebbleEnv.${system} {};
    });
}
