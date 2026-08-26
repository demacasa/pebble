#include "fmt.h"

#include <stdio.h>

void fmt_pace(uint16_t seconds_per_mile, char *buf, size_t len) {
  if (seconds_per_mile == 0 || seconds_per_mile >= 3600) {
    snprintf(buf, len, "--:--");
    return;
  }
  snprintf(buf, len, "%u:%02u", seconds_per_mile / 60, seconds_per_mile % 60);
}

void fmt_duration_hms(uint32_t total_seconds, char *buf, size_t len) {
  if (total_seconds > 359999u) {
    total_seconds = 359999u;
  }
  snprintf(buf, len, "%02u:%02u:%02u", (unsigned)(total_seconds / 3600),
           (unsigned)((total_seconds / 60) % 60), (unsigned)(total_seconds % 60));
}

uint32_t meters_to_tenths_mi(uint32_t meters) {
  // tenths = round(meters / 160.9344) computed in integer math.
  return (uint32_t)(((uint64_t)meters * 10000u + 804672u) / 1609344u);
}

void fmt_tenths_mi(uint32_t tenths, char *buf, size_t len) {
  snprintf(buf, len, "%u.%u", (unsigned)(tenths / 10), (unsigned)(tenths % 10));
}
