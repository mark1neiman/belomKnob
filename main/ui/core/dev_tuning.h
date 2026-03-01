#pragma once

/*
 * Dev tuning switches for quick UI testing.
 * Set UI_DEV_TEST_ENABLE to 1 to force values below.
 */
#define UI_DEV_TEST_ENABLE 0

/* Filter test values */
#define UI_DEV_TEST_FILTER_USED_HOURS 90
#define UI_DEV_TEST_FILTER_LIFETIME_HOURS 100

/* -1 = auto from used/lifetime, 0 = GOOD, 1 = WARNING, 2 = CRITICAL */
#define UI_DEV_TEST_FORCE_FILTER_STATUS 2

/* -1 = keep saved accent, 0..DISPLAY_ACCENT_COUNT-1 = force accent */
#define UI_DEV_TEST_FORCE_ACCENT_INDEX -1
