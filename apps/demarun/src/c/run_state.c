#include "run_state.h"

void run_machine_init(RunMachine *m) {
  m->state = RUN_STATE_IDLE;
  m->duration_s = 0;
}

bool run_machine_select(RunMachine *m) {
  switch (m->state) {
  case RUN_STATE_IDLE:
    m->duration_s = 0;
    m->state = RUN_STATE_RUNNING;
    return true;
  case RUN_STATE_RUNNING:
    m->state = RUN_STATE_PAUSED;
    return true;
  case RUN_STATE_PAUSED:
    m->state = RUN_STATE_RUNNING;
    return true;
  }
  return false;
}

bool run_machine_long_select(RunMachine *m) {
  if (m->state == RUN_STATE_IDLE) {
    return false;
  }
  run_machine_init(m);
  return true;
}

void run_machine_tick(RunMachine *m) {
  if (m->state == RUN_STATE_RUNNING) {
    m->duration_s++;
  }
}
