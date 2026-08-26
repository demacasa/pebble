# pebble

Pebble OS projects, built for the Pebble Time 2 (emery).

## Apps

- **DemaRun** (`apps/demarun`) — high-contrast running app: smoothed
  pace (min/mi), distance (mi), duration, and time of day. GPS comes
  from the phone via PebbleKit JS; the watch owns run state and always
  keeps ticking even if Bluetooth drops.

## Developing

Requires nix (with flakes) and optionally direnv.

    direnv allow          # or: nix develop
    pebble sdk install latest   # first time only
    cd apps/demarun && pebble build
    pebble install --emulator emery

**Note:** The first `nix develop` may prompt about untrusted flake configuration. Accept it, pass `--accept-flake-config`, or run `cachix use pebble` beforehand; otherwise the pebble binary cache is skipped and the first build compiles the toolchain from source.

Tests and formatting: `nix flake check` / `nix fmt`.

## Installing on a watch

1. `pebble login` (Rebble account), enable **Dev Connect** in the Core
   Devices phone app, then `pebble install --cloudpebble`.
2. Same LAN alternative: `pebble install --phone <phone-ip>`.
3. Manual: send `apps/demarun/build/demarun.pbw` to your phone and open
   it with the Core Devices app.
