"use strict";
const test = require("node:test");
const assert = require("node:assert");
const geo = require("../src/pkjs/geo.js");

// 0.001° of latitude ≈ 111.19 m everywhere.
test("haversineM: small latitude delta", () => {
  const d = geo.haversineM(45.0, 7.0, 45.001, 7.0);
  assert.ok(Math.abs(d - 111.19) < 0.5, `got ${d}`);
});

test("haversineM: zero distance", () => {
  assert.strictEqual(geo.haversineM(45, 7, 45, 7), 0);
});

function fix(lat, lon, tSeconds, accuracyM = 5) {
  return { lat, lon, accuracyM, timestampMs: tSeconds * 1000 };
}

test("RunTracker accumulates distance over accepted fixes", () => {
  const t = new geo.RunTracker();
  t.addFix(fix(45.0, 7.0, 0));
  t.addFix(fix(45.001, 7.0, 30)); // ~111 m in 30 s (~3.7 m/s, plausible)
  t.addFix(fix(45.002, 7.0, 60));
  assert.ok(Math.abs(t.distanceM - 222.4) < 1, `got ${t.distanceM}`);
});

test("RunTracker rejects poor-accuracy fixes", () => {
  const t = new geo.RunTracker();
  assert.strictEqual(t.addFix(fix(45.0, 7.0, 0, 80)), false);
  assert.strictEqual(t.distanceM, 0);
});

test("RunTracker treats implausible speed as teleport: no distance, re-anchors", () => {
  const t = new geo.RunTracker();
  t.addFix(fix(45.0, 7.0, 0));
  assert.strictEqual(t.addFix(fix(45.01, 7.0, 1)), false); // ~1112 m in 1 s
  assert.strictEqual(t.distanceM, 0);
  t.addFix(fix(45.011, 7.0, 31)); // plausible from the new anchor, ~111 m
  assert.ok(t.distanceM > 100);
});

test("paceSecondsPerMile: ~3 m/s -> ~536 s/mi", () => {
  const t = new geo.RunTracker();
  t.addFix(fix(45.0, 7.0, 0));
  t.addFix(fix(45.0008094, 7.0, 30)); // ~90 m per 30 s = 3 m/s
  t.addFix(fix(45.0016188, 7.0, 60));
  const pace = t.paceSecondsPerMile();
  assert.ok(Math.abs(pace - 536) <= 2, `got ${pace}`);
});

test("paceSecondsPerMile: 0 when insufficient samples", () => {
  const t = new geo.RunTracker();
  assert.strictEqual(t.paceSecondsPerMile(), 0);
  t.addFix(fix(45.0, 7.0, 0));
  assert.strictEqual(t.paceSecondsPerMile(), 0);
});

test("pause() clears the anchor so paused movement is not counted", () => {
  const t = new geo.RunTracker();
  t.addFix(fix(45.0, 7.0, 0));
  t.addFix(fix(45.001, 7.0, 30));
  const before = t.distanceM;
  t.pause();
  t.addFix(fix(45.05, 7.0, 600)); // far away after pause: just re-anchors
  assert.strictEqual(t.distanceM, before);
  assert.strictEqual(t.paceSecondsPerMile(), 0); // window cleared
});
