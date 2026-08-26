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
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    pebble,
    treefmt-nix,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
      treefmtEval = treefmt-nix.lib.evalModule pkgs ./treefmt.nix;
    in {
      devShells.default = pebble.pebbleEnv.${system} {};
      formatter = treefmtEval.config.build.wrapper;
      checks = {
        formatting = treefmtEval.config.build.check self;
        demarun-c-tests = pkgs.runCommand "demarun-c-tests" {} ''
          cd ${self}/apps/demarun
          mkdir -p $TMPDIR/bin
          ${pkgs.stdenv.cc}/bin/cc -Wall -Wextra -Werror -o $TMPDIR/bin/test_fmt \
            test/test_fmt.c src/c/fmt.c
          $TMPDIR/bin/test_fmt
          ${pkgs.stdenv.cc}/bin/cc -Wall -Wextra -Werror -o $TMPDIR/bin/test_run_state \
            test/test_run_state.c src/c/run_state.c
          $TMPDIR/bin/test_run_state
          touch $out
        '';
        demarun-js-tests = pkgs.runCommand "demarun-js-tests" {} ''
          cd ${self}/apps/demarun
          ${pkgs.nodejs}/bin/node --test 'test/**/*.test.js'
          touch $out
        '';
      };
    });
}
