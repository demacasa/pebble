# DemaRun

A high-contrast running watchapp for Pebble Time 2 (emery), also built for
basalt and diorite.

DemaRun shows a smoothed pace (min/mi), distance (mi, tenths), duration
(hh:mm:ss), and time of day. GPS fixes come from the phone via PebbleKit JS
(`src/pkjs/`); the watch itself (`src/c/`) has no GPS hardware.

## Controls

- **Select** — start / pause / resume the run
- **Long Select** — end the run
- **Back, twice, mid-run** — exit without ending the run

## Build, test, deploy

See the repo root [README](../../README.md) and [CLAUDE.md](../../CLAUDE.md)
for build, test, and deploy instructions.
