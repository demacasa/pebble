# pebble

Monorepo for demacasa's Pebble OS projects. Primary target: Pebble Time 2
("emery": 200×228 64-color e-paper). First app: `apps/demarun`, a running
app (phone GPS → watch display).

## VCS: jujutsu, not git

This repo uses jj. Never run git write commands.
- Commit: `jj describe -m "<msg>"` then `jj new`
- Status/log: `jj st`, `jj log`
- Push: `jj git push` (remote: github.com/demacasa/pebble)

## Environment

Everything runs inside the nix dev shell (direnv auto-enters it):
- `nix develop -c <cmd>` — one-off commands
- First-time setup: `nix develop -c pebble sdk install latest`

**Note:** The first `nix develop` may prompt about untrusted flake configuration. Accept it, pass `--accept-flake-config`, or run `cachix use pebble` beforehand; otherwise the pebble binary cache is skipped and the first build compiles the toolchain from source.

## Commands

| Action | Command |
| --- | --- |
| Build an app | `cd apps/<app> && pebble build` |
| Emulator | `pebble install --emulator emery` (add `--logs` for pkjs logs) |
| All tests + format check | `nix flake check` |
| Format everything | `nix fmt` |
| Deploy to watch (cloud) | `pebble login` once, enable Dev Connect in the Core Devices phone app, then `pebble install --cloudpebble` |
| Deploy to watch (LAN) | `pebble install --phone <phone-ip>` |

## Conventions

- Emery drives layout; basalt/diorite must build (best effort UI).
- Pure logic is SDK-independent: C modules (no `pebble.h`) tested on the
  host in `apps/<app>/test/*.c`; JS logic in plain CommonJS tested with
  `node --test` in `apps/<app>/test/*.test.js`. UI/SDK glue is verified
  in the emulator, not unit-tested.
- Watch colors must be 64-color palette-legal (channels 00/55/AA/FF),
  with `COLOR_FALLBACK` for black-and-white platforms.
- New apps/watchfaces are sibling dirs under `apps/`; shared code moves
  to `packages/` only when a second consumer appears.
- Design specs live in `docs/superpowers/specs/`; DemaRun's spec is
  `2026-08-25-demarun-design.md` (UI mockups linked there).
- prettier is configured with trailingComma: "es5" (treefmt.nix) because pkjs files must stay ES5 — do not add ES2015+ syntax under src/pkjs/.
