#include <pebble.h>

#include "comm.h"
#include "run_state.h"
#include "ui.h"

static Window *s_window;
static RunMachine s_machine;
static UiModel s_model;

static void prv_refresh(void) {
  s_model.state = s_machine.state;
  s_model.duration_s = s_machine.duration_s;
  ui_update(&s_model);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  run_machine_tick(&s_machine);
  prv_refresh();
}

static void prv_on_data(uint32_t distance_m, uint16_t pace_spm, uint8_t gps_status) {
  s_model.distance_m = distance_m;
  s_model.pace_spm = pace_spm;
  s_model.gps = (GpsStatus)gps_status;
  prv_refresh();
}

static void prv_on_status(bool bt_connected, bool data_stale) {
  s_model.bt_connected = bt_connected;
  if (data_stale) {
    s_model.pace_spm = 0;
    if (s_model.state == RUN_STATE_RUNNING) {
      s_model.gps = GPS_LOST;
    }
  }
  prv_refresh();
}

static void prv_window_load(Window *window) {
  ui_create(window);
  prv_refresh();
}

static void prv_window_unload(Window *window) {
  ui_destroy();
}

int main(void) {
  run_machine_init(&s_machine);
  s_model = (UiModel){.bt_connected = true, .gps = GPS_ACQUIRING};
  s_window = window_create();
  window_set_window_handlers(
      s_window, (WindowHandlers){.load = prv_window_load, .unload = prv_window_unload});
  window_stack_push(s_window, true);
  comm_init(prv_on_data, prv_on_status);
  tick_timer_service_subscribe(SECOND_UNIT, prv_tick_handler);
  app_event_loop();
  tick_timer_service_unsubscribe();
  comm_deinit();
  window_destroy(s_window);
  return 0;
}
