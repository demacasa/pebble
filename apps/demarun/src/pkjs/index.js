"use strict";

var geo = require("./geo");

var CMD_START = 1;
var CMD_PAUSE = 2;
var CMD_RESUME = 3;
var CMD_END = 4;

var GPS_ACQUIRING = 0;
var GPS_GOOD = 1;
var GPS_LOST = 2;

var SEND_INTERVAL_MS = 1000;
var LOST_AFTER_MS = 5000;

var tracker = new geo.RunTracker();
var watchId = null;
var tracking = false;
var lastAcceptedMs = 0;
var lastSentMs = 0;

function gpsStatus(nowMs) {
  if (lastAcceptedMs === 0) {
    return GPS_ACQUIRING;
  }
  return nowMs - lastAcceptedMs > LOST_AFTER_MS ? GPS_LOST : GPS_GOOD;
}

function send(nowMs) {
  if (nowMs - lastSentMs < SEND_INTERVAL_MS) {
    return;
  }
  lastSentMs = nowMs;
  Pebble.sendAppMessage(
    {
      DISTANCE_M: Math.round(tracker.distanceM),
      PACE_SPM:
        gpsStatus(nowMs) === GPS_GOOD ? tracker.paceSecondsPerMile() : 0,
      GPS_STATUS: gpsStatus(nowMs),
    },
    function () {},
    function (e) {
      console.log("send failed: " + JSON.stringify(e));
    }
  );
}

function onFix(pos) {
  if (!tracking) {
    return;
  }
  var accepted = tracker.addFix({
    lat: pos.coords.latitude,
    lon: pos.coords.longitude,
    accuracyM: pos.coords.accuracy,
    timestampMs: pos.timestamp,
  });
  var now = Date.now();
  if (accepted) {
    lastAcceptedMs = now;
  }
  send(now);
}

function onGpsError(err) {
  console.log("geolocation error: " + err.code + " " + err.message);
  send(Date.now());
}

function startWatch() {
  if (watchId === null) {
    watchId = navigator.geolocation.watchPosition(onFix, onGpsError, {
      enableHighAccuracy: true,
      maximumAge: 0,
      timeout: 10000,
    });
  }
}

function stopWatch() {
  if (watchId !== null) {
    navigator.geolocation.clearWatch(watchId);
    watchId = null;
  }
}

Pebble.addEventListener("ready", function () {
  console.log("DemaRun pkjs ready");
});

Pebble.addEventListener("appmessage", function (e) {
  var cmd = e.payload.CMD;
  if (cmd === CMD_START) {
    tracker.reset();
    lastAcceptedMs = 0;
    tracking = true;
    startWatch();
  } else if (cmd === CMD_PAUSE) {
    tracking = false;
    tracker.pause();
  } else if (cmd === CMD_RESUME) {
    tracking = true;
    startWatch();
  } else if (cmd === CMD_END) {
    tracking = false;
    stopWatch();
    tracker.reset();
    lastAcceptedMs = 0;
  }
});
