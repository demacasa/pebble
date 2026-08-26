#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  RUN_STATE_IDLE,
  RUN_STATE_RUNNING,
  RUN_STATE_PAUSED,
} RunState;

typedef struct {
  RunState state;
  uint32_t duration_s;
} RunMachine;

void run_machine_init(RunMachine *m);

// Select button. IDLE->RUNNING, RUNNING->PAUSED, PAUSED->RUNNING. Returns true if state changed.
bool run_machine_select(RunMachine *m);

// Long select. Ends an active run (RUNNING or PAUSED -> IDLE, duration resets).
// Returns true if a run ended.
bool run_machine_long_select(RunMachine *m);

// 1 Hz tick. Increments duration only while RUNNING.
void run_machine_tick(RunMachine *m);
