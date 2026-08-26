#pragma once

#include <stddef.h>
#include <stdint.h>

// Pace in seconds per mile as "M:SS" / "MM:SS". 0 or >= 3600 renders "--:--".
void fmt_pace(uint16_t seconds_per_mile, char *buf, size_t len);

// Elapsed seconds as zero-padded "HH:MM:SS", capped at "99:59:59".
void fmt_duration_hms(uint32_t total_seconds, char *buf, size_t len);

// Meters to tenths of a mile, rounded to nearest (1 mile = 1609.344 m).
uint32_t meters_to_tenths_mi(uint32_t meters);

// Tenths of a mile as "D.T", e.g. 32 -> "3.2".
void fmt_tenths_mi(uint32_t tenths, char *buf, size_t len);
