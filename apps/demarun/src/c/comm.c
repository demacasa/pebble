#include "comm.h"

#define STALE_AFTER_MS 5000
#define RETRY_DELAY_MS 500
#define MAX_RETRIES 3

static CommDataHandler s_on_data;
static CommStatusHandler s_on_status;
static AppTimer *s_stale_timer;
static bool s_stale = true;
static RunCmd s_pending_cmd;
static int s_retries_left;

static void prv_notify_status(void) {
  if (s_on_status) {
    s_on_status(connection_service_peek_pebble_app_connection(), s_stale);
  }
}

static void prv_stale_timeout(void *ctx) {
  s_stale_timer = NULL;
  s_stale = true;
  prv_notify_status();
}

static void prv_inbox_received(DictionaryIterator *iter, void *ctx) {
  Tuple *cmd_ack = dict_find(iter, MESSAGE_KEY_CMD);
  Tuple *dist = dict_find(iter, MESSAGE_KEY_DISTANCE_M);
  Tuple *pace = dict_find(iter, MESSAGE_KEY_PACE_SPM);
  Tuple *gps = dict_find(iter, MESSAGE_KEY_GPS_STATUS);
  (void)cmd_ack;
  if (dist && pace && gps && s_on_data) {
    s_stale = false;
    if (s_stale_timer) {
      app_timer_reschedule(s_stale_timer, STALE_AFTER_MS);
    } else {
      s_stale_timer = app_timer_register(STALE_AFTER_MS, prv_stale_timeout, NULL);
    }
    s_on_data(dist->value->uint32, (uint16_t)pace->value->uint32, (uint8_t)gps->value->uint32);
    prv_notify_status();
  }
}

static void prv_do_send(void *ctx) {
  DictionaryIterator *out;
  if (app_message_outbox_begin(&out) != APP_MSG_OK) {
    if (s_retries_left-- > 0) {
      app_timer_register(RETRY_DELAY_MS, prv_do_send, NULL);
    }
    return;
  }
  dict_write_uint8(out, MESSAGE_KEY_CMD, (uint8_t)s_pending_cmd);
  app_message_outbox_send();
}

static void prv_outbox_failed(DictionaryIterator *iter, AppMessageResult reason, void *ctx) {
  if (s_retries_left-- > 0) {
    app_timer_register(RETRY_DELAY_MS, prv_do_send, NULL);
  }
}

static void prv_connection_changed(bool connected) {
  if (!connected) {
    s_stale = true;
  }
  prv_notify_status();
}

void comm_init(CommDataHandler on_data, CommStatusHandler on_status) {
  s_on_data = on_data;
  s_on_status = on_status;
  app_message_register_inbox_received(prv_inbox_received);
  app_message_register_outbox_failed(prv_outbox_failed);
  app_message_open(128, 64);
  connection_service_subscribe(
      (ConnectionHandlers){.pebble_app_connection_handler = prv_connection_changed});
  prv_notify_status();
}

void comm_deinit(void) {
  connection_service_unsubscribe();
  app_message_deregister_callbacks();
}

void comm_send_cmd(RunCmd cmd) {
  s_pending_cmd = cmd;
  s_retries_left = MAX_RETRIES;
  prv_do_send(NULL);
}
