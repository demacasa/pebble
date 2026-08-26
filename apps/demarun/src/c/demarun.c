#include <pebble.h>

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
  tick_timer_service_subscribe(SECOND_UNIT, prv_tick_handler);
  app_event_loop();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  return 0;
}
