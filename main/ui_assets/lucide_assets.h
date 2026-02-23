#pragma once

#include "lvgl.h"

/* Official Lucide icon font codepoints from lucide-static/font/lucide.css */
#define UI_ICON_LUCIDE_CHECK "\xEE\x81\xB0"         /* U+E070 */
#define UI_ICON_LUCIDE_CHEVRON_LEFT "\xEE\x81\xB2"  /* U+E072 */
#define UI_ICON_LUCIDE_FILTER "\xEE\x83\x95"        /* U+E0D5 */
#define UI_ICON_LUCIDE_GLOBE "\xEE\x83\xAB"         /* U+E0EB */
#define UI_ICON_LUCIDE_INFO "\xEE\x83\xBE"          /* U+E0FE */
#define UI_ICON_LUCIDE_LOCK "\xEE\x84\x8E"          /* U+E10E */
#define UI_ICON_LUCIDE_LOCK_OPEN "\xEE\x84\x8F"     /* U+E10F */
#define UI_ICON_LUCIDE_MOON "\xEE\x84\xA1"          /* U+E121 */
#define UI_ICON_LUCIDE_REFRESH_CW "\xEE\x85\x88"    /* U+E148 */
#define UI_ICON_LUCIDE_ROTATE_CW "\xEE\x85\x8C"     /* U+E14C */
#define UI_ICON_LUCIDE_SUN "\xEE\x85\xBB"           /* U+E17B */
#define UI_ICON_LUCIDE_ALERT_TRIANGLE "\xEE\x86\x92"/* U+E192 */
#define UI_ICON_LUCIDE_WIND "\xEE\x86\xAF"          /* U+E1AF */
#define UI_ICON_LUCIDE_ZAP "\xEE\x86\xB3"           /* U+E1B3 */
#define UI_ICON_LUCIDE_GAUGE "\xEE\x86\xBE"         /* U+E1BE */
#define UI_ICON_LUCIDE_PALETTE "\xEE\x87\x9C"       /* U+E1DC */
#define UI_ICON_LUCIDE_SHIELD_CHECK "\xEE\x87\xBE"  /* U+E1FE */
#define UI_ICON_LUCIDE_SHIELD_ALERT "\xEE\x87\xBD"  /* U+E1FD */

const lv_font_t *ui_font_lucide_20(void);
