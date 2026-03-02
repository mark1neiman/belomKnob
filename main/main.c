#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/pulse_cnt.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lvgl.h"
#include "iot_knob.h"
#include "iot_button.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_sh8601.h"
#include "esp_lcd_touch_cst820.h"
#include "ui_assets/lucide_assets.h"
#include "ui/core/dev_tuning.h"

LV_FONT_DECLARE(lv_font_inter_medium_130_num);
LV_FONT_DECLARE(lv_font_inter_medium_145_num);
LV_FONT_DECLARE(lv_font_lucide_20_icons);
LV_FONT_DECLARE(lv_font_lucide_40_filter_btn_icons);
LV_FONT_DECLARE(lv_font_lucide_64_filter_reset_icons);
LV_FONT_DECLARE(lv_font_inter_bold_180_num);
LV_FONT_DECLARE(lv_font_lucide_96_icons);
LV_FONT_DECLARE(lv_font_lucide_180_icons);
LV_FONT_DECLARE(lv_font_lucide_150_icons);
LV_FONT_DECLARE(lv_font_lucide_162_lock);
LV_FONT_DECLARE(lv_font_lucide_48_v2_icons);
LV_FONT_DECLARE(lv_font_lucide_120_v2_icons);
LV_FONT_DECLARE(lv_font_inter_bold_30_caps);
LV_FONT_DECLARE(lv_font_inter_medium_22_caps);
LV_FONT_DECLARE(lv_font_inter_medium_22_ui);
LV_FONT_DECLARE(lv_font_inter_semibold_28_ui);
LV_FONT_DECLARE(lv_font_inter_bold_60_lang);
LV_FONT_DECLARE(lv_font_inter_bold_72_lang);
LV_FONT_DECLARE(lv_font_inter_medium_20_caps);
LV_FONT_DECLARE(lv_font_inter_medium_12_caps);
LV_FONT_DECLARE(lv_font_inter_medium_16_ui);
#if LVGL_VERSION_MAJOR >= 9
extern const lv_image_dsc_t belom_logo_white;
#else
extern const lv_img_dsc_t belom_logo_white;
#endif

//***************** */

// extern  esp_err_t lvgl_port_indev_init(void);

//*************************************************** */




static const char *TAG = "example";

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_timer_handle_t boot_timer = NULL;
static esp_timer_handle_t sleep_timer = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_HOST (SPI2_HOST)
#define EXAMPLE_LCD_BITS_PER_PIXEL (16)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE (1)
#define EXAMPLE_LCD_LVGL_AVOID_TEAR (1)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT (60)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_LCD_BK_PIN_IS_POWER_ENABLE 1
#define EXAMPLE_PIN_NUM_LCD_CS (GPIO_NUM_12)
#define EXAMPLE_PIN_NUM_LCD_PCLK (GPIO_NUM_10)
#define EXAMPLE_PIN_NUM_LCD_DATA0 (GPIO_NUM_13)
#define EXAMPLE_PIN_NUM_LCD_DATA1 (GPIO_NUM_11)
#define EXAMPLE_PIN_NUM_LCD_DATA2 (GPIO_NUM_14)
#define EXAMPLE_PIN_NUM_LCD_DATA3 (GPIO_NUM_9)
#define EXAMPLE_PIN_NUM_LCD_RST (GPIO_NUM_8)
#define EXAMPLE_PIN_NUM_BK_LIGHT (GPIO_NUM_17)

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES 472 // 466
#define EXAMPLE_LCD_V_RES 466 // 466

#if CONFIG_LV_COLOR_DEPTH == 32
#define LCD_BIT_PER_PIXEL (24)
#elif CONFIG_LV_COLOR_DEPTH == 16
#define LCD_BIT_PER_PIXEL (16)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your touch spec ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CONFIG_EXAMPLE_LCD_TOUCH_ENABLED 0
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
#define EXAMPLE_TOUCH_HOST (I2C_NUM_0)
#define EXAMPLE_TOUCH_I2C_NUM (0)
#define EXAMPLE_TOUCH_I2C_CLK_HZ (400000)

#define EXAMPLE_PIN_NUM_TOUCH_SCL (GPIO_NUM_3)
#define EXAMPLE_PIN_NUM_TOUCH_SDA (GPIO_NUM_1)
#define EXAMPLE_PIN_NUM_TOUCH_RST (GPIO_NUM_2)
#define EXAMPLE_PIN_NUM_TOUCH_INT (GPIO_NUM_4)

esp_lcd_touch_handle_t tp = NULL;
#endif

typedef enum
{
    BSP_BTN_PRESS = GPIO_NUM_0,
} bsp_button_t;

#define BSP_ENCODER_A (GPIO_NUM_6)
#define BSP_ENCODER_B (GPIO_NUM_5)
#define BSP_FAN_PWM (GPIO_NUM_45)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// UI/UX configuration ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FAN_SPEED_MIN_PERCENT 0
#define FAN_SPEED_NORMAL_MAX_PERCENT 100
#define FAN_SPEED_OVERDRIVE_MAX_PERCENT 110
#define FAN_SPEED_DEFAULT_PERCENT 0
#define UI_SEGMENT_COUNT 40
#define UI_RING_RADIUS 204
#define UI_SEGMENT_W 10
#define UI_SEGMENT_H 22
#define UI_MAIN_DIAL_SIZE 320
#define UI_MAIN_STATUS_ROW_Y 88
#define UI_MAIN_OVERDRIVE_Y -124
#define UI_BOOT_DIAL_SIZE EXAMPLE_LCD_V_RES
#define UI_BOOT_RING_RADIUS ((EXAMPLE_LCD_V_RES / 2) - (UI_SEGMENT_H / 2) - 1)
#define UI_BOOT_CENTER_OFFSET_X 8
#define UI_BOOT_CENTER_OFFSET_Y 10
#define UI_MAIN_CENTER_OFFSET_X UI_BOOT_CENTER_OFFSET_X
#define UI_MAIN_CENTER_OFFSET_Y UI_BOOT_CENTER_OFFSET_Y
#define UI_SPEED_CENTER_OFFSET_X (UI_MAIN_CENTER_OFFSET_X - 8)
#define UI_SPEED_CENTER_OFFSET_Y (UI_MAIN_CENTER_OFFSET_Y + 5)
#define UI_BOOT_LOGO_TARGET_WIDTH 320
#define UI_BOOT_ANIM_TIME_US (2500ULL * 1000ULL)
#define UI_BOOT_PAUSE_US (500ULL * 1000ULL)
#define UI_BOOT_TOTAL_TIME_US (UI_BOOT_ANIM_TIME_US + UI_BOOT_PAUSE_US)
#define UI_BOOT_LOOP_TEST 0
#define UI_BOOT_ENABLE_DARK_THEME_TRANSITION 0
#define UI_BOOT_THEME_TRANSITION_TIME_MS 420
#define UI_ENABLE_INVALIDATE_ROUNDER 1
#define UI_LVGL_LOCK_TIMEOUT_MS 100
#define UI_ACCENT_COLOR_HEX 0xE10600
#define UI_BOOT_BG_LIGHT_HEX 0xFFFFFF
#define UI_BOOT_RING_INACTIVE_LIGHT_HEX 0x52525B
#define UI_BOOT_RING_INACTIVE_HEX 0x52525b
#define UI_BOOT_SUBTITLE_COLOR_LIGHT_FROM_HEX 0x3F3F46
#define UI_BOOT_SUBTITLE_COLOR_LIGHT_TO_HEX 0x52525B
#define UI_BOOT_SUBTITLE_COLOR_FROM_HEX 0x71717A
#define UI_BOOT_SUBTITLE_COLOR_TO_HEX 0x52525B
#define UI_RING_INACTIVE_HEX 0x52525B
#define UI_BG_HEX 0x000000
#define UI_SURFACE_HEX 0x000000
#define UI_TEXT_MUTED_HEX 0x52525b
#define UI_FILTER_GOOD_HEX 0x10B981
#define UI_FILTER_WARNING_HEX 0xEAB308
#define UI_FILTER_CRITICAL_HEX 0xE10600
#define FAN_PWM_FREQ_HZ 25000
#define FAN_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define FAN_PWM_MAX_DUTY ((1 << 10) - 1)
#define OWNER_NAME_MAX_LEN 16
#define SETTINGS_MENU_ITEMS_COUNT 12
#define SENSITIVITY_MIN_CLICKS 10
#define SENSITIVITY_MAX_CLICKS 90
#define SENSITIVITY_DEFAULT_CLICKS 30
#define DISPLAY_BRIGHTNESS_MIN 10
#define DISPLAY_BRIGHTNESS_MAX 100
#define DISPLAY_BRIGHTNESS_DEFAULT 10
#define DISPLAY_ROTATION_STEP_DEG 180
#define CO5300_X_GAP_ROT_0 6
#define CO5300_X_GAP_ROT_180 2
#define DISPLAY_ACCENT_COUNT 20
#define UI_ENABLE_CUSTOM_ACCENT 0
#define DISPLAY_ACCENT_CUSTOM_INDEX DISPLAY_ACCENT_COUNT
#if UI_ENABLE_CUSTOM_ACCENT
#define DISPLAY_ACCENT_OPTIONS_COUNT (DISPLAY_ACCENT_COUNT + 1)
#define DISPLAY_ACCENT_MAX_INDEX DISPLAY_ACCENT_CUSTOM_INDEX
#else
#define DISPLAY_ACCENT_OPTIONS_COUNT DISPLAY_ACCENT_COUNT
#define DISPLAY_ACCENT_MAX_INDEX (DISPLAY_ACCENT_COUNT - 1)
#endif
#define ACCENT_CUSTOM_HUE_STEP_DEG 5
#define SETTINGS_CUSTOM_HUE_SEGMENT_COUNT 96
#define FILTER_USED_DEFAULT_HOURS 0
#define FILTER_LIFETIME_DEFAULT_HOURS 100
#define FILTER_LIFETIME_MIN_HOURS 50
#define FILTER_LIFETIME_MAX_HOURS 100
#define FILTER_USAGE_AUTOSAVE_INTERVAL_US (30ULL * 60ULL * 1000000ULL)
#define SLEEP_TIMEOUT_US (30ULL * 60ULL * 1000000ULL)
#define SETTINGS_ROOT_ITEMS_COUNT 11
#define SETTINGS_V2_DOTS_COUNT ((DISPLAY_ACCENT_COUNT > SETTINGS_ROOT_ITEMS_COUNT) ? DISPLAY_ACCENT_COUNT : SETTINGS_ROOT_ITEMS_COUNT)
#define SETTINGS_LANG_OPTIONS_COUNT 12
#define SETTINGS_WHEEL_ROOT_STEP_PX 104
#define SETTINGS_WHEEL_SCROLL_Q 256
#define SETTINGS_WHEEL_MAX_DIST_PX 96.0f
#define SETTINGS_WHEEL_CULL_EXTRA_STEPS 0.2f
#define SETTINGS_WHEEL_VISIBLE_EXTRA_STEPS 0.2f
#define SETTINGS_WHEEL_MIN_DRAW_STEP_PX 1
#define SETTINGS_KNOB_MIN_INTERVAL_US 1200
#define SETTINGS_KNOB_DIR_GLITCH_FILTER_US 3500
#define SETTINGS_KNOB_DIR_STREAK_MIN 2
#define SETTINGS_KNOB_DIR_STREAK_RESET_US 50000
#define SETTINGS_BLUR_RADIUS 0
#define SETTINGS_BLUR_BG_OPA 0
#define SETTINGS_BLUR_SLOT_PAD 0
#define UI_SHOW_FPS_COUNTER 0
#define UI_REFR_ACTIVE_PERIOD_MS 16
#define UI_REFR_IDLE_PERIOD_MS 16
#define UI_REFR_SETTINGS_ACTIVE_PERIOD_MS 20
#define UI_REFR_SETTINGS_IDLE_PERIOD_MS 28
#define UI_REFR_ACTIVE_HOLD_US (1200ULL * 1000ULL)
#define SETTINGS_V2_ENABLE_GLOW 0
#define SETTINGS_V2_ENABLE_CENTER_SHADOW 0
#define SETTINGS_V2_ANIMATE_CENTER_CIRCLE 0
#define SETTINGS_V2_SLIDE_SHIFT_PX 34
#define SETTINGS_V2_SLIDE_TIME_MS 140

typedef enum {
    UI_SCREEN_BOOT = 0,
    UI_SCREEN_MAIN,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_LANGUAGE,
    UI_SCREEN_OWNER,
    UI_SCREEN_SENSITIVITY,
    UI_SCREEN_OVERDRIVE,
    UI_SCREEN_PURIFICATION,
    UI_SCREEN_SLEEP,
} ui_screen_t;

typedef enum {
    LANG_EN = 0,
    LANG_RU,
    LANG_DE,
    LANG_FR,
    LANG_ET,
    LANG_FI,
    LANG_LV,
    LANG_UK,
    LANG_ES,
    LANG_PL,
    LANG_LT,
    LANG_IT,
    LANG_COUNT,
} ui_lang_t;

typedef enum {
    FILTER_STATUS_GOOD = 0,
    FILTER_STATUS_WARNING,
    FILTER_STATUS_CRITICAL,
} filter_status_t;

typedef enum {
    SETTINGS_VIEW_ROOT = 0,
    SETTINGS_VIEW_SENSITIVITY,
    SETTINGS_VIEW_ACCENT,
    SETTINGS_VIEW_LANGUAGE,
    SETTINGS_VIEW_BRIGHTNESS,
    SETTINGS_VIEW_THEME,
    SETTINGS_VIEW_ROTATION,
    SETTINGS_VIEW_LOCK_MODE,
    SETTINGS_VIEW_OVERDRIVE_WARNING,
    SETTINGS_VIEW_OVERDRIVE_MODE,
    SETTINGS_VIEW_FILTER,
    SETTINGS_VIEW_FILTER_RESET,
    SETTINGS_VIEW_PURIFICATION_CONFIRM,
    SETTINGS_VIEW_ABOUT,
} settings_view_t;

static ui_screen_t ui_screen = UI_SCREEN_BOOT;
static ui_lang_t current_lang = LANG_RU;
static bool ui_locked = false;
static filter_status_t filter_status = FILTER_STATUS_GOOD;
static int fan_speed_percent = FAN_SPEED_DEFAULT_PERCENT;
static int fan_speed_tenths = FAN_SPEED_DEFAULT_PERCENT * 10;
static int fan_speed_arc_displayed = FAN_SPEED_DEFAULT_PERCENT;
static int settings_index = 0;
static settings_view_t settings_view = SETTINGS_VIEW_ROOT;
static bool settings_edit_mode = false;
static int settings_items_count = 0;
static int speed_sensitivity_clicks = SENSITIVITY_DEFAULT_CLICKS;
static bool allow_overdrive = false;
static bool allow_child_lock = true;
static int display_brightness_percent = DISPLAY_BRIGHTNESS_DEFAULT;
static int display_rotation_deg = 0;
static int display_accent_index = 0;
static int custom_accent_hue = 0;
static bool display_theme_dark = true;
static int filter_used_hours = FILTER_USED_DEFAULT_HOURS;
static int filter_lifetime_hours = FILTER_LIFETIME_DEFAULT_HOURS;
static uint32_t filter_used_seconds_total = FILTER_USED_DEFAULT_HOURS * 3600U;
static int64_t filter_runtime_last_tick_us = 0;
static int64_t filter_runtime_partial_us = 0;
static int64_t filter_runtime_unsaved_us = 0;
static uint32_t ui_accent_color_hex = UI_ACCENT_COLOR_HEX;
static bool is_purifying = false;
static bool ui_sleeping = false;
static size_t owner_name_len = 0;
static char owner_name[OWNER_NAME_MAX_LEN + 1] = "";
static ui_screen_t ui_screen_before_sleep = UI_SCREEN_MAIN;

static lv_obj_t *boot_screen = NULL;
static lv_obj_t *boot_ring_segments[UI_SEGMENT_COUNT] = {0};
static lv_obj_t *boot_logo = NULL;
static lv_obj_t *boot_subtitle = NULL;
static lv_obj_t *boot_progress_track = NULL;
static lv_obj_t *boot_progress_fill = NULL;
static int32_t boot_progress_percent = 0;
static lv_opa_t boot_theme_dark_blend_opa = LV_OPA_TRANSP;
static int boot_last_active_count = -1;
static int boot_last_partial_index = -1;
static lv_coord_t boot_last_fill_width = -1;
static lv_obj_t *main_screen = NULL;
static lv_obj_t *lock_screen = NULL;
static lv_obj_t *lock_screen_icon = NULL;
static lv_obj_t *lock_screen_icon_shackle = NULL;
static lv_obj_t *lock_screen_icon_shackle_cut = NULL;
static lv_obj_t *lock_screen_icon_body = NULL;
static lv_obj_t *lock_screen_title = NULL;
static lv_obj_t *lock_screen_hint = NULL;
static lv_obj_t *lock_ring_container = NULL;
static lv_obj_t *lock_ring_segments[UI_SEGMENT_COUNT] = {0};
static lv_timer_t *lock_ring_timer = NULL;
static lv_obj_t *main_bg_glow_outer = NULL;
static lv_obj_t *main_bg_glow_inner = NULL;
static lv_obj_t *main_bg_glow_core = NULL;
static lv_obj_t *main_center_disc = NULL;
static lv_obj_t *settings_screen = NULL;
static lv_obj_t *language_screen = NULL;
static lv_obj_t *owner_screen = NULL;
static lv_obj_t *sensitivity_screen = NULL;
static lv_obj_t *overdrive_screen = NULL;
static lv_obj_t *label_speed = NULL;
static lv_obj_t *label_speed_caption = NULL;
static lv_obj_t *ring_segments[UI_SEGMENT_COUNT] = {0};
static int main_ring_active_count = -1;
static int main_ring_boundary_index = -1;
static lv_obj_t *lock_overlay = NULL;
static lv_obj_t *label_overdrive_warning = NULL;
static lv_obj_t *label_lock_status = NULL;
static lv_obj_t *label_lock_icon = NULL;
static lv_obj_t *label_filter_status = NULL;
static lv_obj_t *label_filter_icon = NULL;
static lv_obj_t *status_divider = NULL;
static lv_obj_t *settings_items[SETTINGS_MENU_ITEMS_COUNT] = {0};
static lv_obj_t *settings_item_icon_bgs[SETTINGS_MENU_ITEMS_COUNT] = {0};
static lv_obj_t *settings_item_icons[SETTINGS_MENU_ITEMS_COUNT] = {0};
static lv_obj_t *settings_item_titles[SETTINGS_MENU_ITEMS_COUNT] = {0};
static lv_obj_t *settings_item_values[SETTINGS_MENU_ITEMS_COUNT] = {0};
static lv_obj_t *settings_item_arrows[SETTINGS_MENU_ITEMS_COUNT] = {0};
static bool settings_item_value_is_icon[SETTINGS_MENU_ITEMS_COUNT] = {0};
static lv_obj_t *settings_value_slider_track = NULL;
static lv_obj_t *settings_value_slider_fill = NULL;
static lv_obj_t *settings_value_slider_knob = NULL;
static lv_obj_t *settings_v2_layer = NULL;
static lv_obj_t *settings_v2_glow = NULL;
static lv_obj_t *settings_v2_center_circle = NULL;
static lv_obj_t *settings_v2_center_icon = NULL;
static lv_obj_t *settings_v2_center_title = NULL;
static lv_obj_t *settings_v2_center_value = NULL;
static lv_obj_t *settings_v2_left_circle = NULL;
static lv_obj_t *settings_v2_left_icon = NULL;
static lv_obj_t *settings_v2_right_circle = NULL;
static lv_obj_t *settings_v2_right_icon = NULL;
static lv_obj_t *settings_v2_dots[SETTINGS_V2_DOTS_COUNT] = {0};
static lv_obj_t *settings_v2_arc = NULL;
static lv_obj_t *settings_v2_arc_icon = NULL;
static lv_obj_t *settings_v2_arc_title = NULL;
static lv_obj_t *settings_v2_arc_value = NULL;
static lv_obj_t *settings_v2_arc_suffix = NULL;
static lv_obj_t *settings_v2_arc_subtitle = NULL;
static lv_obj_t *settings_custom_hue_ring_segments[SETTINGS_CUSTOM_HUE_SEGMENT_COUNT] = {0};
static lv_obj_t *settings_custom_hue_knob = NULL;
static bool settings_custom_hue_ui_ready = false;
static int settings_custom_edit_prev_accent_index = 0;
static int settings_custom_edit_prev_hue = 0;
static bool settings_custom_edit_in_progress = false;
static int settings_v2_last_index = -1;
static int settings_v2_nav_dir = 0;
static int settings_v2_transition_mode = 0;
static bool settings_v2_theme_cache_valid = false;
static bool settings_v2_theme_cache_dark = false;
static uint32_t settings_v2_theme_cache_accent_hex = 0;
static bool settings_v2_root_cache_valid = false;
static uint32_t settings_v2_root_cache_sig = 0;
static int settings_v2_filter_btn_last_active = -1;
static int settings_v2_filter_confirm_last_active = -1;
static lv_obj_t *settings_overlay_icon_bg = NULL;
static lv_obj_t *settings_overlay_icon = NULL;
static lv_obj_t *settings_overlay_title = NULL;
static lv_obj_t *settings_overlay_line1 = NULL;
static lv_obj_t *settings_overlay_line2 = NULL;
static lv_obj_t *settings_overlay_line3 = NULL;
static lv_obj_t *settings_dim_overlay = NULL;
static lv_obj_t *settings_blur_top_overlay = NULL;
static lv_obj_t *settings_blur_bottom_overlay = NULL;
static int settings_root_saved_index = 0;
static lv_obj_t *settings_sensitivity_title = NULL;
static lv_obj_t *settings_sensitivity_value = NULL;
static lv_obj_t *settings_sensitivity_subtitle = NULL;
static lv_obj_t *settings_sensitivity_unit = NULL;
static lv_obj_t *settings_filter_used_arc = NULL;
static lv_obj_t *settings_filter_used_label = NULL;
static lv_obj_t *settings_filter_used_value = NULL;
static lv_obj_t *settings_filter_used_unit = NULL;
static lv_obj_t *settings_filter_used_subtitle = NULL;
static lv_obj_t *settings_about_title = NULL;
static lv_obj_t *settings_about_model_caption = NULL;
static lv_obj_t *settings_about_model_value = NULL;
static lv_obj_t *settings_about_fw_caption = NULL;
static lv_obj_t *settings_about_fw_value = NULL;
static lv_obj_t *settings_about_if_caption = NULL;
static lv_obj_t *settings_about_if_value = NULL;
static bool settings_filter_reset_animating = false;
static int settings_filter_reset_anim_value = -1;
static bool settings_filter_reset_confirm_open = false;
static lv_obj_t *settings_slider_caption = NULL;
static lv_obj_t *settings_focus_frame = NULL;
static lv_obj_t *label_settings_title = NULL;
static lv_obj_t *label_language_title = NULL;
static lv_obj_t *label_owner_title = NULL;
static lv_obj_t *label_sensitivity_title = NULL;
static lv_obj_t *label_sensitivity_value = NULL;
static lv_obj_t *label_overdrive_title = NULL;
static lv_obj_t *label_purification_title = NULL;
static lv_obj_t *label_purification_state = NULL;
static lv_obj_t *label_purification_hint = NULL;
static lv_obj_t *purification_spinner = NULL;
static lv_obj_t *purification_spinner_icon = NULL;
static lv_obj_t *roller_language = NULL;
static lv_obj_t *label_owner_value = NULL;
static lv_obj_t *roller_owner = NULL;
static lv_obj_t *roller_sensitivity = NULL;
static lv_obj_t *roller_overdrive = NULL;
static lv_obj_t *purification_screen = NULL;
static lv_obj_t *sleep_screen = NULL;
static lv_obj_t *label_fps_counter = NULL;
static char language_roller_options[256] = {0};
static int last_speed_label_percent = -1;
static int64_t main_knob_last_update_us = 0;
static int64_t settings_knob_last_update_us = 0;
static int32_t settings_root_scroll_q = 0;
static int32_t settings_root_scroll_last_draw_q = INT32_MIN;
static bool settings_root_scroll_animating = false;
static uint32_t fps_counter_frames = 0;
static int64_t fps_counter_last_update_us = 0;
static uint8_t fps_counter_cpu_percent = 0xFF;
static int64_t fps_counter_last_cpu_sample_us = 0;
static uint32_t fps_counter_prev_total_runtime = 0;
static uint32_t fps_counter_prev_idle_runtime = 0;
static bool fps_counter_prev_runtime_valid = false;
static uint32_t ui_refr_current_period_ms = 0;
static int64_t ui_refr_active_until_us = 0;

static uint32_t accent_palette_hex[DISPLAY_ACCENT_COUNT] = {0};
#define ACCENT_NAME_LEN 8

static const char *language_names[LANG_COUNT] = {
    [LANG_EN] = "English",
    [LANG_RU] = "Русский",
    [LANG_DE] = "Deutsch",
    [LANG_FR] = "Français",
    [LANG_ET] = "Eesti",
    [LANG_FI] = "Suomi",
    [LANG_LV] = "Latviešu",
    [LANG_UK] = "Українська",
    [LANG_ES] = "Español",
    [LANG_PL] = "Polski",
    [LANG_LT] = "Lietuvių",
    [LANG_IT] = "Italiano",
};

typedef struct {
    const char *fan_speed;
    const char *settings;
    const char *language;
    const char *owner_name;
    const char *locked;
    const char *unlocked;
} ui_strings_t;

static const ui_strings_t ui_strings[LANG_COUNT] = {
    [LANG_EN] = {
        .fan_speed = "Fan speed",
        .settings = "Settings",
        .language = "Language",
        .owner_name = "Owner name",
        .locked = "Locked",
        .unlocked = "Unlocked",
    },
    [LANG_RU] = {
        .fan_speed = "Скорость",
        .settings = "Настройки",
        .language = "Язык",
        .owner_name = "Имя владельца",
        .locked = "Закрыто",
        .unlocked = "Открыто",
    },
    [LANG_ET] = {
        .fan_speed = "Ventilaator",
        .settings = "Seaded",
        .language = "Keel",
        .owner_name = "Omaniku nimi",
        .locked = "Lukus",
        .unlocked = "Avatud",
    },
    [LANG_DE] = {
        .fan_speed = "Lüfter",
        .settings = "Einstellungen",
        .language = "Sprache",
        .owner_name = "Besitzername",
        .locked = "Gesperrt",
        .unlocked = "Entsperrt",
    },
    [LANG_FR] = {
        .fan_speed = "Ventilateur",
        .settings = "Paramètres",
        .language = "Langue",
        .owner_name = "Propriétaire",
        .locked = "Verrouillé",
        .unlocked = "Déverrouillé",
    },
    [LANG_FI] = {
        .fan_speed = "Tuuletin",
        .settings = "Asetukset",
        .language = "Kieli",
        .owner_name = "Omistaja",
        .locked = "Lukittu",
        .unlocked = "Avattu",
    },
    [LANG_LV] = {
        .fan_speed = "Ventilators",
        .settings = "Iestatījumi",
        .language = "Valoda",
        .owner_name = "Īpašnieks",
        .locked = "Bloķēts",
        .unlocked = "Atbloķēts",
    },
    [LANG_UK] = {
        .fan_speed = "Швидкість",
        .settings = "Налаштування",
        .language = "Мова",
        .owner_name = "Ім'я власника",
        .locked = "Заблоковано",
        .unlocked = "Розблоковано",
    },
    [LANG_ES] = {
        .fan_speed = "Ventilador",
        .settings = "Ajustes",
        .language = "Idioma",
        .owner_name = "Propietario",
        .locked = "Bloqueado",
        .unlocked = "Desbloqueado",
    },
    [LANG_PL] = {
        .fan_speed = "Prędkość",
        .settings = "Ustawienia",
        .language = "Język",
        .owner_name = "Nazwa właściciela",
        .locked = "Zablokowane",
        .unlocked = "Odblokowane",
    },
    [LANG_LT] = {
        .fan_speed = "Greitis",
        .settings = "Nustatymai",
        .language = "Kalba",
        .owner_name = "Savininko vardas",
        .locked = "Užrakinta",
        .unlocked = "Atrakinta",
    },
    [LANG_IT] = {
        .fan_speed = "Velocità",
        .settings = "Impostazioni",
        .language = "Lingua",
        .owner_name = "Nome proprietario",
        .locked = "Bloccato",
        .unlocked = "Sbloccato",
    },
};

static void handle_single_click(void);
static void update_main_ui(void);
static void apply_language(void);
static void set_lock_overlay(bool enabled);
static void set_obj_bg_opa_anim(void *obj, int32_t value);
static void set_obj_opa_anim(void *obj, int32_t value);
static void set_obj_text_opa_anim(void *obj, int32_t value);
static void build_segment_ring(lv_obj_t *parent);
static void update_segment_ring(int target_percent, bool increasing);
static void update_status_row(void);
static void animate_speed_arc_to(int target_percent);
static void apply_dark_screen_style(lv_obj_t *screen);
static void apply_theme_now(void);
static void restart_current_screen_after_theme_change(void);
static void apply_display_rotation_now(void);
static void fps_counter_create(void);
static void fps_display_refr_event_cb(lv_event_t *e);
static void settings_update_blur_layers(void);
static void update_settings_wheel_layout(void);
static void settings_sync_root_scroll_to_index(void);
static int settings_get_item_count(void);
static lv_color_t ui_theme_bg_color(void);
static int clamp_int(int value, int min_value, int max_value);
static const char *accent_palette_name(int index);
static void accent_set_custom_hue(int hue_deg);
static bool settings_is_custom_accent_edit_active(void);
static void ui_mark_activity(void);
static void ui_refresh_rate_tick(void);
#include "ui/i18n/ui_translations.inc"

static const char *ui_tr(const char *en, const char *ru)
{
    const char *picked = ui_translation_lookup(en);
    if (picked && picked[0]) {
        return picked;
    }

    /* Fallback keeps legacy EN/RU behavior for unmapped keys. */
    return (current_lang == LANG_RU && ru && ru[0]) ? ru : (en ? en : "");
}

#include "ui/screens/settings/wheel_layout.inc"

static void set_boot_theme_blend(lv_opa_t dark_blend_opa, int active_count, int partial_index);
static void set_boot_theme_mode(bool dark_mode, int active_count, int partial_index);
static void boot_theme_blend_anim_cb(void *var, int32_t value);
static void boot_fill_anim_ready_cb(lv_anim_t *a);
static void boot_transition_task(void *arg);
static void boot_progress_anim_cb(void *var, int32_t value);
static void start_boot_animation(void);
static void show_screen(lv_obj_t *screen);
static void reset_sleep_timer(void);
static void enter_sleep_mode(void);
static void wake_from_sleep(void);
static void start_purification_mode(void);
static void stop_purification_mode(bool go_to_main);
static void handle_double_click(void);
static esp_err_t prefill_panel_black(void);

#include "ui/core/system_core.inc"

#include "ui/screens/settings/settings_logic.inc"

#include "ui/runtime/ui_runtime.inc"

#include "ui/platform/lvgl_port.inc"

#include "ui/screens/boot/screen_boot.inc"

#include "ui/screens/speed/screen_speed.inc"

#include "ui/screens/lock/screen_lock.inc"

#include "ui/screens/settings/screen_settings_root.inc"

#include "ui/screens/settings/sections/language_screen.inc"

#include "ui/screens/settings/sections/owner_screen.inc"

#include "ui/screens/settings/sections/sensitivity_screen.inc"

#include "ui/screens/settings/sections/overdrive_screen.inc"

#include "ui/screens/purification/screen_purification.inc"

#include "ui/screens/sleep/screen_sleep.inc"

#include "ui/flow/ui_flow.inc"

static void accent_palette_init(void)
{
    const int hue_stride = 11; /* Coprime with 20: distributes neighboring items far apart in hue. */
    accent_palette_hex[0] = UI_ACCENT_COLOR_HEX;
    for (int i = 1; i < DISPLAY_ACCENT_COUNT; ++i) {
        int hue_slot = (i * hue_stride) % DISPLAY_ACCENT_COUNT;
        int hue = (hue_slot * 360) / DISPLAY_ACCENT_COUNT;
        accent_palette_hex[i] = accent_hex_from_hue(hue);
    }
}

static const char *accent_palette_name(int index)
{
    static char name[ACCENT_NAME_LEN];
    static const char hex_digits[] = "0123456789ABCDEF";
    if (index < 0 || index >= DISPLAY_ACCENT_COUNT) {
        return "";
    }

    uint32_t color = accent_palette_hex[index];
    name[0] = '#';
    name[1] = hex_digits[(color >> 20) & 0x0F];
    name[2] = hex_digits[(color >> 16) & 0x0F];
    name[3] = hex_digits[(color >> 12) & 0x0F];
    name[4] = hex_digits[(color >> 8) & 0x0F];
    name[5] = hex_digits[(color >> 4) & 0x0F];
    name[6] = hex_digits[color & 0x0F];
    name[7] = '\0';
    return name;
}

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x00}, 0, 0},
    {0xC4, (uint8_t[]){0x80}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},
    {0x35, (uint8_t[]){0x00}, 0, 10},
    {0x53, (uint8_t[]){0x28}, 1, 10},
    {0x51, (uint8_t[]){0xFF}, 1, 10},
    {0x63, (uint8_t[]){0xFF}, 1, 10},
    //{0x2A, (uint8_t []){0x00,0x06,0x01,0xD7}, 4, 0},
    //{0x2B, (uint8_t []){0x00,0x00,0x01,0xD1}, 4, 0},
    {0x2A, (uint8_t[]){0x00, 0x06, 0x01, 0xDD}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xD1}, 4, 0},
    {0x11, (uint8_t[]){0x00}, 0, 60},
};


//***************The following code is for standardizing the event triggering of rotary encoders and buttons.**************** */
//**************Encoder********* */
#include "ui/input/input_devices.inc"

/* ============================================================================
 * Main Function
 * ============================================================================ */
#include "ui/input/uart_console.inc"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    accent_palette_init();
    load_settings();
    apply_runtime_visual_settings();
    apply_dev_test_overrides();
    set_speed_percent(fan_speed_percent);
    init_pwm_fan();
    apply_fan_pwm(fan_speed_percent);

    if (EXAMPLE_PIN_NUM_BK_LIGHT >= 0)
    {
        ESP_LOGI(TAG, "Configure LCD power/enable pin");
        gpio_config_t bk_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
        ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
        gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
    }
    ESP_LOGI(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg =
        SH8601_PANEL_BUS_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_PCLK, EXAMPLE_PIN_NUM_LCD_DATA0,
                                     EXAMPLE_PIN_NUM_LCD_DATA1, EXAMPLE_PIN_NUM_LCD_DATA2,
                                     EXAMPLE_PIN_NUM_LCD_DATA3, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * LCD_BIT_PER_PIXEL / 8);

    ESP_ERROR_CHECK(spi_bus_initialize(EXAMPLE_LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = -1,                     // EXAMPLE_PIN_NUM_LCD_CS,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS, //-1,
        .pclk_hz = 80 * 1000 * 1000,
        .trans_queue_depth = 20,
        .lcd_cmd_bits = 32,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .flags = {
            .quad_mode = true,
        },
    };

    sh8601_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds, // Uncomment these line if use custom initialization commands
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(sh8601_lcd_init_cmd_t),
        .flags = {
            .use_qspi_interface = 1,
        },
    };

    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_LCD_HOST, &io_config, &lcd_io));

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    ESP_LOGI(TAG, "Install LCD driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(lcd_io, &panel_config, &lcd_panel));


    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel));

    // Keep panel OFF while GRAM is prefilled with black to avoid white sweep on first visible frame.
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel, false));
    esp_err_t prefill_err = prefill_panel_black();
    if (prefill_err != ESP_OK) {
        ESP_LOGW(TAG, "Panel black prefill failed: %s", esp_err_to_name(prefill_err));
    }

    /* Enable panel-side brightness control and apply saved brightness before first visible frame */
    display_init_brightness_control();
    display_set_brightness_percent(display_brightness_percent);

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel, true));
    /* Some panel revisions apply WRDISBV only after DISPON */
    display_set_brightness_percent(display_brightness_percent);

#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
    esp_err_t touch_ret = app_touch_init();
    if (touch_ret != ESP_OK) {
        ESP_LOGW(TAG, "Touch init failed: %s. Continue without touch.", esp_err_to_name(touch_ret));
        touch_handle = NULL;
    }
#else
    touch_handle = NULL;
    ESP_LOGI(TAG, "Touch is disabled by configuration");
#endif

    if (EXAMPLE_PIN_NUM_BK_LIGHT >= 0)
    {
        ESP_LOGI(TAG, "HF --Turn on LCD backlight");
        gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
    }

    app_lvgl_init();

    knob_init(BSP_ENCODER_A, BSP_ENCODER_B);
    button_init(BSP_BTN_PRESS);

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGE(TAG, "LVGL lock timeout in app_main ui_init");
        return;
    }
    apply_display_rotation_now();
    ui_init();
    // Release the mutex
    lvgl_port_unlock();

    xTaskCreate(uart_control_task, "uart_ctrl", 4096, NULL, 4, NULL);
}





















