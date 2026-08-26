#pragma once

#include <pebble.h>

#include "run_state.h"

typedef enum {
  GPS_ACQUIRING = 0,
  GPS_GOOD = 1,
  GPS_LOST = 2,
} GpsStatus;

typedef struct {
  RunState state;
  uint32_t duration_s;
  uint32_t distance_m;
  uint16_t pace_spm; // 0 = unknown
  GpsStatus gps;
  bool bt_connected;
  bool show_exit_hint;
} UiModel;

void ui_create(Window *window);
void ui_destroy(void);
void ui_update(const UiModel *model);
