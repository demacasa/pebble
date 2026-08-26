#include <stdio.h>
#include <string.h>

#include "../src/c/fmt.h"

static int failures = 0;

#define ASSERT_STREQ(expected, actual)                                                             \
  do {                                                                                             \
    if (strcmp((expected), (actual)) != 0) {                                                       \
      printf("FAIL %s:%d expected \"%s\" got \"%s\"\n", __FILE__, __LINE__, (expected), (actual)); \
      failures++;                                                                                  \
    }                                                                                              \
  } while (0)

#define ASSERT_EQ_U32(expected, actual)                                                            \
  do {                                                                                             \
    if ((uint32_t)(expected) != (uint32_t)(actual)) {                                              \
      printf("FAIL %s:%d expected %u got %u\n", __FILE__, __LINE__, (unsigned)(expected),          \
             (unsigned)(actual));                                                                  \
      failures++;                                                                                  \
    }                                                                                              \
  } while (0)

int main(void) {
  char buf[16];

  fmt_pace(581, buf, sizeof buf);
  ASSERT_STREQ("9:41", buf);
  fmt_pace(600, buf, sizeof buf);
  ASSERT_STREQ("10:00", buf);
  fmt_pace(59, buf, sizeof buf);
  ASSERT_STREQ("0:59", buf);
  fmt_pace(0, buf, sizeof buf);
  ASSERT_STREQ("--:--", buf);
  fmt_pace(3600, buf, sizeof buf);
  ASSERT_STREQ("--:--", buf);

  fmt_duration_hms(0, buf, sizeof buf);
  ASSERT_STREQ("00:00:00", buf);
  fmt_duration_hms(2537, buf, sizeof buf);
  ASSERT_STREQ("00:42:17", buf);
  fmt_duration_hms(3723, buf, sizeof buf);
  ASSERT_STREQ("01:02:03", buf);
  fmt_duration_hms(999999, buf, sizeof buf);
  ASSERT_STREQ("99:59:59", buf);

  ASSERT_EQ_U32(0, meters_to_tenths_mi(0));
  ASSERT_EQ_U32(1, meters_to_tenths_mi(161));   /* 0.100 mi rounds to 1 tenth */
  ASSERT_EQ_U32(10, meters_to_tenths_mi(1609)); /* ~1 mile */
  ASSERT_EQ_U32(32, meters_to_tenths_mi(5150)); /* 3.20 mi */

  fmt_tenths_mi(0, buf, sizeof buf);
  ASSERT_STREQ("0.0", buf);
  fmt_tenths_mi(32, buf, sizeof buf);
  ASSERT_STREQ("3.2", buf);
  fmt_tenths_mi(103, buf, sizeof buf);
  ASSERT_STREQ("10.3", buf);

  if (failures) {
    printf("%d failure(s)\n", failures);
    return 1;
  }
  printf("test_fmt: all tests passed\n");
  return 0;
}
