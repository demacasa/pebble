#pragma once

#include <pebble.h>

typedef enum {
  CMD_START = 1,
  CMD_PAUSE = 2,
  CMD_RESUME = 3,
  CMD_END = 4,
} RunCmd;

typedef void (*CommDataHandler)(uint32_t distance_m, uint16_t pace_spm, uint8_t gps_status);
typedef void (*CommStatusHandler)(bool bt_connected, bool data_stale);

void comm_init(CommDataHandler on_data, CommStatusHandler on_status);
void comm_deinit(void);
void comm_send_cmd(RunCmd cmd);
