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

#define EXIT_HINT_MS 3000

static AppTimer *s_exit_hint_timer;

static void prv_exit_hint_expired(void *ctx) {
  s_exit_hint_timer = NULL;
  s_model.show_exit_hint = false;
  prv_refresh();
}

static void prv_clear_exit_hint(void) {
  if (s_exit_hint_timer) {
    app_timer_cancel(s_exit_hint_timer);
    s_exit_hint_timer = NULL;
  }
  s_model.show_exit_hint = false;
}

static void prv_select_click(ClickRecognizerRef recognizer, void *context) {
  prv_clear_exit_hint();
  RunState before = s_machine.state;
  if (run_machine_select(&s_machine)) {
    if (before == RUN_STATE_IDLE) {
      s_model.distance_m = 0;
      s_model.pace_spm = 0;
      s_model.gps = GPS_ACQUIRING;
      comm_send_cmd(CMD_START);
    } else if (before == RUN_STATE_RUNNING) {
      comm_send_cmd(CMD_PAUSE);
    } else {
      comm_send_cmd(CMD_RESUME);
    }
    prv_refresh();
  }
}

static void prv_select_long_click(ClickRecognizerRef recognizer, void *context) {
  prv_clear_exit_hint();
  if (run_machine_long_select(&s_machine)) {
    comm_send_cmd(CMD_END);
    s_model.distance_m = 0;
    s_model.pace_spm = 0;
    s_model.gps = GPS_ACQUIRING;
    prv_refresh();
  }
}

static void prv_back_click(ClickRecognizerRef recognizer, void *context) {
  if (s_machine.state == RUN_STATE_IDLE || s_model.show_exit_hint) {
    prv_clear_exit_hint();
    comm_send_cmd(CMD_END);
    window_stack_pop_all(true);
    return;
  }
  s_model.show_exit_hint = true;
  prv_refresh();
  s_exit_hint_timer = app_timer_register(EXIT_HINT_MS, prv_exit_hint_expired, NULL);
}

static void prv_click_config(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, prv_select_long_click, NULL);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click);
}

int main(void) {
  run_machine_init(&s_machine);
  s_model = (UiModel){.bt_connected = true, .gps = GPS_ACQUIRING};
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config);
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
