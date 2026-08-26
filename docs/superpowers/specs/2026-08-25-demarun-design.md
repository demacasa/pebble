# DemaRun — Pebble Time 2 Running App (Design Spec)

Date: 2026-08-25
Status: approved design, pre-implementation
UI mockups: https://claude.ai/code/artifact/b5366900-e48b-4199-9334-565c2926a9f5

## Purpose

A high-contrast, glanceable running app for the Pebble Time 2. While
running it shows current pace, distance, elapsed duration, and time of
day. It is the first project in the `demacasa/pebble` monorepo, which
will grow to hold other Pebble apps and watchfaces.

## Decisions already made

- **Data source**: phone GPS via PebbleKit JS. The Pebble Time 2 has no
  GPS; the phone must be carried on runs. No watch-only fallback in v1.
- **SDK**: Pebble C SDK for the watch app; PebbleKit JS for the phone
  side. Not the Alloy JS SDK.
- **Platforms**: emery (Pebble Time 2, 200×228 64-color e-paper) is the
  design target. Also compile for basalt and diorite best-effort for
  emulator convenience; they must build but do not drive layout.
- **Units**: imperial — pace in min/mi, distance in miles. No unit
  setting in v1.
- **Controls**: Select toggles start/pause/resume; long-press Select
  ends the run; Back exits, with a confirmation while a run is active.
- **VCS**: jujutsu (`jj`), pushing to https://github.com/demacasa/pebble.

## Repo structure (monorepo)

```
pebble/
├── flake.nix              # single root flake: dev shell, checks, formatter
├── .envrc                 # direnv: use flake
├── CLAUDE.md              # repo conventions, build/test/deploy commands
├── README.md
├── apps/
│   └── demarun/
│       ├── package.json   # pebble project manifest (targetPlatforms: emery, basalt, diorite)
│       ├── wscript
│       ├── src/
│       │   ├── c/         # watch app
│       │   └── pkjs/      # phone side (PebbleKit JS)
│       ├── resources/     # custom fonts
│       └── test/          # unit tests for this app
└── docs/
    └── superpowers/specs/ # design specs like this one
```

Future apps/watchfaces get sibling directories under `apps/`. Shared
code moves to `packages/` only when a second consumer appears.

## Architecture

**Phone (PebbleKit JS) owns GPS and geo math.**
- `navigator.geolocation.watchPosition` at ~1 Hz.
- Filters bad fixes: accuracy worse than a threshold (~50 m), implausible
  jumps (teleports), stale timestamps.
- Accumulates distance with the haversine formula over accepted fixes.
- Computes pace smoothed over roughly the last 3 accepted fixes
  (~3 seconds), as seconds-per-mile.
- Sends the watch a compact AppMessage about once per second:
  `{distance_m: uint32, pace_spm: uint16, gps_status: uint8}`.
  `pace_spm = 0` means "no pace" (display `--:--`).
  `gps_status`: 0 = acquiring, 1 = good fix, 2 = signal lost.
- Reacts to start/pause/resume/end commands from the watch
  (`{cmd: uint8}` messages) by starting/stopping the watch on position
  and resetting accumulators.

**Watch (C) owns run state, time, and rendering.**
- State machine: `IDLE → RUNNING ⇄ PAUSED → ENDED(→IDLE)`.
- Duration ticks locally via `tick_timer_service` (1 s) — never depends
  on Bluetooth, so the clock cannot stutter.
- Renders received distance/pace values; converts meters → miles on the
  watch (integer math, tenths of a mile).
- On Bluetooth disconnect or missing updates (>5 s without a message):
  pace shows `--:--`, GPS indicator shows lost; duration keeps running.
- Distance display holds its last value when GPS drops (distance simply
  stops accruing).

Rationale: the JS side is far easier to unit-test for geo math, the
watch stays a reliable dumb display, and the protocol is three fields
instead of a raw fix stream.

## UI (final, "Option E" from the mockups)

Single fullscreen window, black background. Real-pixel sizes for the
200×228 emery screen:

- **Status bar** (top): time of day left (17 px, gray `#AAAAAA`,
  formatted per the watch's 12/24 h setting), Bluetooth glyph and GPS
  arrow right. GPS arrow color: green `#00FF00` good fix, yellow
  `#FFFF00` acquiring or lost. BT glyph gray when connected, red
  `#FF0000` when disconnected.
- **Pace** (center, dominant): label `PACE /MI` (13 px caps, gray,
  letterspaced); value ~68 px white in a custom condensed numeric font.
  `--:--` (gray) when no pace.
- **Distance**: ~50 px white with a 21 px gray `MI` unit, above a 1 px
  `#555555` divider. One decimal place (tenths): `3.2`.
- **Duration** (bottom): label-free, stable `hh:mm:ss` (`00:42:17`),
  17 px white — same size as the clock; the zero-padded three-field
  format is what identifies it. Never changes shape, so the layout
  never shifts at the one-hour mark.
- **States**:
  - *Idle*: pace `--:--`, `PRESS SELECT TO START` hint (12 px white),
    `0.0` / `00:00:00` in gray, yellow GPS arrow while acquiring.
  - *Paused*: inverted full-width banner `PAUSED` (white background,
    black text) under the status bar; pace `--:--` gray; distance and
    duration keep their values in white.
  - *GPS lost*: pace `--:--` gray with `GPS SIGNAL LOST` (12 px yellow)
    beneath; distance holds; duration keeps running.
- All colors restricted to the 64-color palette (channel values
  00/55/AA/FF). Everything fits the screen; nothing scrolls.

**Fonts**: numerals use a bundled condensed font (Oswald or similar
openly-licensed condensed face) packaged at the two big sizes with a
`characterRegex` limited to `[0-9:.\-]` plus what the unit strings
need, to keep resource size small. Labels use system Gothic 14/18.

## Error handling

- No phone / Bluetooth down: BT glyph red, pace `--:--`; duration keeps
  counting; distance holds. Reconnection resumes updates seamlessly
  (phone JS keeps accumulating while disconnected if it is still
  running; AppMessage retries with exponential backoff).
- Location permission denied on phone: treated as GPS lost, status
  stays yellow; (v1 keeps this simple — no dedicated error screen).
- Watch app killed mid-run: run state is not persisted in v1; a
  restart returns to Idle.

## Testing

- **C unit tests**: pure logic in SDK-independent modules
  (`format_pace`, `format_duration_hms`, meters→tenths-of-mile
  conversion, run state machine transitions) compiled on the host with
  a minimal assert-based runner, run via `nix flake check`.
- **JS unit tests**: geo math (haversine, fix filtering, smoothing
  window, pace computation) as a pure module shared between pkjs and
  tests, run with Node's built-in `node --test` runner. No npm
  dependency tree.
- UI/SDK glue is exercised in the emulator (`pebble install --emulator
  emery`), not unit-tested.

## Tooling

- **Nix**: root `flake.nix` using pebble-dev's `pebble.nix` (Cachix
  binary cache) for the SDK dev shell; `pebble` CLI, emulator, and
  installs all work inside `nix develop`. Fallback if pebble.nix cannot
  target SDK 4.17/emery: a plain dev shell pinning `uv`-installed
  `pebble-tool`. `.envrc` for direnv.
- **Formatting**: treefmt wired into the flake — clang-format (C),
  prettier (JS), alejandra (nix). `nix fmt` formats the repo;
  `nix flake check` verifies formatting and runs tests.
- **Deploy** (documented in README/CLAUDE.md):
  1. `pebble install --cloudpebble` after `pebble login` + Dev Connect
     enabled in the Core Devices phone app (primary).
  2. `pebble install --phone <ip>` on the same LAN (faster loop when it
     works).
  3. Manual: send the built `.pbw` to the phone and open it with the
     Core Devices app.

## Out of scope (v1 follow-ups)

- Workout/health export (top follow-up)
- Auto-pause; laps/splits
- Metric units / unit setting
- Heart-rate display (PT2 has an HR sensor — natural v2 field)
- Run persistence across app restarts
- Watch-only distance estimation fallback
