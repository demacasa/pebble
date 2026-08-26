#include <stdio.h>

#include "../src/c/run_state.h"

static int failures = 0;

#define ASSERT_TRUE(cond)                                                                          \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond);                                        \
      failures++;                                                                                  \
    }                                                                                              \
  } while (0)

int main(void) {
  RunMachine m;
  run_machine_init(&m);
  ASSERT_TRUE(m.state == RUN_STATE_IDLE);
  ASSERT_TRUE(m.duration_s == 0);

  // Ticks in IDLE do nothing.
  run_machine_tick(&m);
  ASSERT_TRUE(m.duration_s == 0);

  // Select starts.
  ASSERT_TRUE(run_machine_select(&m));
  ASSERT_TRUE(m.state == RUN_STATE_RUNNING);
  run_machine_tick(&m);
  run_machine_tick(&m);
  ASSERT_TRUE(m.duration_s == 2);

  // Select pauses; ticks freeze.
  ASSERT_TRUE(run_machine_select(&m));
  ASSERT_TRUE(m.state == RUN_STATE_PAUSED);
  run_machine_tick(&m);
  ASSERT_TRUE(m.duration_s == 2);

  // Select resumes.
  ASSERT_TRUE(run_machine_select(&m));
  ASSERT_TRUE(m.state == RUN_STATE_RUNNING);
  run_machine_tick(&m);
  ASSERT_TRUE(m.duration_s == 3);

  // Long select ends and resets.
  ASSERT_TRUE(run_machine_long_select(&m));
  ASSERT_TRUE(m.state == RUN_STATE_IDLE);
  ASSERT_TRUE(m.duration_s == 0);

  // Long select in IDLE is a no-op.
  ASSERT_TRUE(!run_machine_long_select(&m));
  ASSERT_TRUE(m.state == RUN_STATE_IDLE);

  // Long select from PAUSED also ends.
  run_machine_select(&m);
  run_machine_tick(&m);
  run_machine_select(&m);
  ASSERT_TRUE(run_machine_long_select(&m));
  ASSERT_TRUE(m.state == RUN_STATE_IDLE && m.duration_s == 0);

  if (failures) {
    printf("%d failure(s)\n", failures);
    return 1;
  }
  printf("test_run_state: all tests passed\n");
  return 0;
}
