"use strict";

var EARTH_RADIUS_M = 6371000;
var METERS_PER_MILE = 1609.344;

function haversineM(lat1, lon1, lat2, lon2) {
  var toRad = Math.PI / 180;
  var dLat = (lat2 - lat1) * toRad;
  var dLon = (lon2 - lon1) * toRad;
  var a =
    Math.sin(dLat / 2) * Math.sin(dLat / 2) +
    Math.cos(lat1 * toRad) *
      Math.cos(lat2 * toRad) *
      Math.sin(dLon / 2) *
      Math.sin(dLon / 2);
  return 2 * EARTH_RADIUS_M * Math.asin(Math.sqrt(a));
}

// Accumulates run distance and a short smoothed pace window from geolocation fixes.
function RunTracker(opts) {
  opts = opts || {};
  this.maxAccuracyM = opts.maxAccuracyM || 50;
  this.maxSpeedMps = opts.maxSpeedMps || 12; // faster than a world-record sprint => bad fix
  this.windowSize = opts.windowSize || 3;
  this.reset();
}

RunTracker.prototype.reset = function () {
  this.distanceM = 0;
  this.last = null;
  this.samples = [];
};

// Drop the anchor and smoothing window (e.g. on pause) without touching distance.
RunTracker.prototype.pause = function () {
  this.last = null;
  this.samples = [];
};

// fix: {lat, lon, accuracyM, timestampMs}. Returns true if the fix advanced the run.
RunTracker.prototype.addFix = function (fix) {
  if (fix.accuracyM > this.maxAccuracyM) {
    return false;
  }
  if (!this.last) {
    this.last = fix;
    return true;
  }
  var dtS = (fix.timestampMs - this.last.timestampMs) / 1000;
  if (dtS <= 0) {
    return false;
  }
  var dM = haversineM(this.last.lat, this.last.lon, fix.lat, fix.lon);
  if (dM / dtS > this.maxSpeedMps) {
    this.last = fix; // teleport: re-anchor, do not accrue
    return false;
  }
  this.distanceM += dM;
  this.samples.push({ dtS: dtS, dM: dM });
  if (this.samples.length > this.windowSize) {
    this.samples.shift();
  }
  this.last = fix;
  return true;
};

// Smoothed pace in whole seconds per mile; 0 when unknown or slower than 60 min/mi.
RunTracker.prototype.paceSecondsPerMile = function () {
  var dt = 0;
  var d = 0;
  for (var i = 0; i < this.samples.length; i++) {
    dt += this.samples[i].dtS;
    d += this.samples[i].dM;
  }
  if (this.samples.length < 2 || d < 1) {
    return 0;
  }
  var spm = Math.round((dt / d) * METERS_PER_MILE);
  return spm >= 3600 ? 0 : spm;
};

module.exports = { haversineM: haversineM, RunTracker: RunTracker };
