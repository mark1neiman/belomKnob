
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
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

LV_FONT_DECLARE(lv_font_montserrat_200_num);
LV_FONT_DECLARE(lv_font_inter_medium_130_num);
LV_FONT_DECLARE(lv_font_inter_medium_145_num);
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
#define UI_MAIN_OVERDRIVE_Y -56
#define UI_BOOT_DIAL_SIZE EXAMPLE_LCD_V_RES
#define UI_BOOT_RING_RADIUS ((EXAMPLE_LCD_V_RES / 2) - (UI_SEGMENT_H / 2) - 1)
#define UI_BOOT_CENTER_OFFSET_X 8
#define UI_BOOT_CENTER_OFFSET_Y 10
#define UI_MAIN_CENTER_OFFSET_X UI_BOOT_CENTER_OFFSET_X
#define UI_MAIN_CENTER_OFFSET_Y UI_BOOT_CENTER_OFFSET_Y
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
#define UI_BOOT_BG_LIGHT_HEX 0x000000
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
#define UI_FILTER_CRITICAL_HEX 0xEF4444
#define FAN_PWM_FREQ_HZ 25000
#define FAN_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define FAN_PWM_MAX_DUTY ((1 << 10) - 1)
#define OWNER_NAME_MAX_LEN 16
#define SETTINGS_MENU_ITEMS_COUNT 12
#define SENSITIVITY_MIN_CLICKS 15
#define SENSITIVITY_MAX_CLICKS 100
#define SENSITIVITY_DEFAULT_CLICKS 15
#define DISPLAY_BRIGHTNESS_MIN 10
#define DISPLAY_BRIGHTNESS_MAX 100
#define DISPLAY_BRIGHTNESS_DEFAULT 100
#define DISPLAY_ROTATION_STEP_DEG 180
#define DISPLAY_ACCENT_COUNT 8
#define FILTER_USED_DEFAULT_HOURS 12
#define FILTER_LIFETIME_DEFAULT_HOURS 100
#define FILTER_LIFETIME_MIN_HOURS 50
#define FILTER_LIFETIME_MAX_HOURS 1000
#define SLEEP_TIMEOUT_US (30ULL * 60ULL * 1000000ULL)
#define SETTINGS_ROOT_ITEMS_COUNT 11
#define SETTINGS_LANG_OPTIONS_COUNT 4
#define SETTINGS_WHEEL_ROOT_STEP_PX 104
#define SETTINGS_WHEEL_SCROLL_Q 256
#define SETTINGS_WHEEL_MAX_DIST_PX 96.0f
#define SETTINGS_WHEEL_CULL_EXTRA_STEPS 0.1f
#define SETTINGS_WHEEL_VISIBLE_EXTRA_STEPS 0.2f
#define SETTINGS_WHEEL_MIN_DRAW_STEP_PX 4
#define SETTINGS_KNOB_MIN_INTERVAL_US 12000
#define SETTINGS_BLUR_BAND_H 150
#define SETTINGS_BLUR_RADIUS 10
#define UI_SHOW_FPS_COUNTER 0

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
    LANG_LT,
    LANG_ES,
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
    SETTINGS_VIEW_PURIFICATION_CONFIRM,
    SETTINGS_VIEW_ABOUT,
} settings_view_t;

static ui_screen_t ui_screen = UI_SCREEN_BOOT;
static ui_lang_t current_lang = LANG_EN;
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
static bool display_theme_dark = true;
static int filter_used_hours = FILTER_USED_DEFAULT_HOURS;
static int filter_lifetime_hours = FILTER_LIFETIME_DEFAULT_HOURS;
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
static lv_obj_t *settings_overlay_icon_bg = NULL;
static lv_obj_t *settings_overlay_icon = NULL;
static lv_obj_t *settings_overlay_title = NULL;
static lv_obj_t *settings_overlay_line1 = NULL;
static lv_obj_t *settings_overlay_line2 = NULL;
static lv_obj_t *settings_overlay_line3 = NULL;
static lv_obj_t *settings_blur_top_overlay = NULL;
static lv_obj_t *settings_blur_bottom_overlay = NULL;
static lv_color_t *settings_blur_top_buf = NULL;
static lv_color_t *settings_blur_bottom_buf = NULL;
static lv_draw_buf_t *settings_blur_snapshot_buf = NULL;
static lv_color_format_t settings_blur_color_format = LV_COLOR_FORMAT_RGB565;
static int settings_root_saved_index = 2;
static lv_obj_t *settings_sensitivity_title = NULL;
static lv_obj_t *settings_sensitivity_value = NULL;
static lv_obj_t *settings_sensitivity_subtitle = NULL;
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

static const uint32_t accent_palette_hex[DISPLAY_ACCENT_COUNT] = {
    0xE10600,
    0xF97316,
    0xEAB308,
    0x10B981,
    0x06B6D4,
    0x3B82F6,
    0x8B5CF6,
    0xEC4899,
};

static const char *accent_palette_names[DISPLAY_ACCENT_COUNT] = {
    "Red",
    "Orange",
    "Amber",
    "Green",
    "Cyan",
    "Blue",
    "Purple",
    "Pink",
};

static const char *language_names[LANG_COUNT] = {
    [LANG_EN] = "English",
    [LANG_RU] = "Русский",
    [LANG_DE] = "Deutsch",
    [LANG_FR] = "Fran�ais",
    [LANG_ET] = "Eesti",
    [LANG_FI] = "Suomi",
    [LANG_LV] = "Latviešu",
    [LANG_LT] = "Lietuviu",
    [LANG_ES] = "Español",
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
        .locked = "Заблокировано",
        .unlocked = "Разблокировано",
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
        .fan_speed = "Luefter",
        .settings = "Einstellungen",
        .language = "Sprache",
        .owner_name = "Besitzername",
        .locked = "Gesperrt",
        .unlocked = "Entsperrt",
    },
    [LANG_FR] = {
        .fan_speed = "Ventilateur",
        .settings = "Parametres",
        .language = "Langue",
        .owner_name = "Proprietaire",
        .locked = "Verrouille",
        .unlocked = "Deverrouille",
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
    [LANG_LT] = {
        .fan_speed = "Ventiliatorius",
        .settings = "Nustatymai",
        .language = "Kalba",
        .owner_name = "Savininko vardas",
        .locked = "Užrakinta",
        .unlocked = "Atrakinta",
    },
    [LANG_ES] = {
        .fan_speed = "Ventilador",
        .settings = "Ajustes",
        .language = "Idioma",
        .owner_name = "Propietario",
        .locked = "Bloqueado",
        .unlocked = "Desbloqueado",
    },
};

static void handle_single_click(void);
static void update_main_ui(void);
static void apply_language(void);
static void set_lock_overlay(bool enabled);
static void set_obj_bg_opa_anim(void *obj, int32_t value);
static void set_obj_text_opa_anim(void *obj, int32_t value);
static void build_segment_ring(lv_obj_t *parent);
static void update_segment_ring(int target_percent, bool increasing);
static void update_status_row(void);
static void animate_speed_arc_to(int target_percent);
static void apply_dark_screen_style(lv_obj_t *screen);
static void fps_counter_create(void);
static void fps_display_refr_event_cb(lv_event_t *e);
static void settings_update_blur_layers(void);
static void update_settings_wheel_layout(void);
static void settings_sync_root_scroll_to_index(void);
static void settings_start_root_scroll_anim(void);
static int clamp_int(int value, int min_value, int max_value);

void lv_canvas_stackblur_rgba(lv_obj_t *canvas, const lv_area_t *area, uint16_t radius);

static lv_coord_t settings_wheel_step_for_view(settings_view_t view)
{
    return (view == SETTINGS_VIEW_ROOT) ? SETTINGS_WHEEL_ROOT_STEP_PX : 98;
}

static bool settings_view_uses_wheel_list(settings_view_t view)
{
    return (view == SETTINGS_VIEW_ROOT);
}

static int32_t settings_index_to_scroll_q_for_view(int index, settings_view_t view)
{
    return index * settings_wheel_step_for_view(view) * SETTINGS_WHEEL_SCROLL_Q;
}

static void settings_root_scroll_anim_exec_cb(void *var, int32_t value)
{
    int32_t *scroll_q = (int32_t *)var;
    if (!scroll_q) {
        return;
    }

    int32_t delta_q = value - settings_root_scroll_last_draw_q;
    int32_t min_step_q = SETTINGS_WHEEL_MIN_DRAW_STEP_PX * SETTINGS_WHEEL_SCROLL_Q;
    if (settings_root_scroll_last_draw_q != INT32_MIN &&
        delta_q > -min_step_q && delta_q < min_step_q) {
        *scroll_q = value;
        return;
    }

    *scroll_q = value;
    settings_root_scroll_last_draw_q = value;
    update_settings_wheel_layout();
}

static void settings_root_scroll_anim_ready_cb(lv_anim_t *a)
{
    (void)a;
    settings_root_scroll_animating = false;
    settings_root_scroll_q = settings_index_to_scroll_q_for_view(settings_index, settings_view);
    settings_root_scroll_last_draw_q = INT32_MIN;
    update_settings_wheel_layout();
    settings_update_blur_layers();
}

static void settings_sync_root_scroll_to_index(void)
{
    lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
    settings_root_scroll_animating = false;
    settings_root_scroll_q = settings_index_to_scroll_q_for_view(settings_index, settings_view);
    settings_root_scroll_last_draw_q = INT32_MIN;
}

static void settings_start_root_scroll_anim(void)
{
    int32_t target = settings_index_to_scroll_q_for_view(settings_index, settings_view);
    if (target == settings_root_scroll_q) {
        update_settings_wheel_layout();
        return;
    }

    lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
    settings_root_scroll_animating = true;

    int32_t delta = target - settings_root_scroll_q;
    (void)delta;
    int anim_time = 450;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &settings_root_scroll_q);
    lv_anim_set_exec_cb(&a, settings_root_scroll_anim_exec_cb);
    lv_anim_set_values(&a, settings_root_scroll_q, target);
    lv_anim_set_time(&a, (uint32_t)anim_time);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, settings_root_scroll_anim_ready_cb);
    lv_anim_start(&a);
}

static void fps_counter_create(void)
{
    if (!UI_SHOW_FPS_COUNTER) {
        label_fps_counter = NULL;
        fps_counter_frames = 0;
        fps_counter_last_update_us = 0;
        return;
    }

    lv_obj_t *parent = settings_screen ? settings_screen : lv_layer_top();
    label_fps_counter = lv_label_create(parent);
    lv_obj_set_style_text_font(label_fps_counter, &lv_font_inter_medium_20_caps, 0);
    lv_obj_set_style_text_color(label_fps_counter, lv_color_hex(0xF4F4F5), 0);
    lv_obj_set_style_bg_color(label_fps_counter, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(label_fps_counter, (lv_opa_t)168, 0);
    lv_obj_set_style_pad_left(label_fps_counter, 8, 0);
    lv_obj_set_style_pad_right(label_fps_counter, 8, 0);
    lv_obj_set_style_pad_top(label_fps_counter, 4, 0);
    lv_obj_set_style_pad_bottom(label_fps_counter, 4, 0);
    lv_obj_set_style_radius(label_fps_counter, 10, 0);
    lv_obj_set_style_text_letter_space(label_fps_counter, 1, 0);
    lv_obj_set_style_border_width(label_fps_counter, 1, 0);
    lv_obj_set_style_border_color(label_fps_counter, lv_color_hex(0x52525B), 0);
    lv_obj_set_style_border_opa(label_fps_counter, (lv_opa_t)120, 0);
    lv_label_set_text(label_fps_counter, "FPS --");
    lv_obj_align(label_fps_counter, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_clear_flag(label_fps_counter, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(label_fps_counter);

    fps_counter_frames = 0;
    fps_counter_last_update_us = esp_timer_get_time();
}

static void fps_display_refr_event_cb(lv_event_t *e)
{
    if (!e || lv_event_get_code(e) != LV_EVENT_REFR_START || !label_fps_counter) {
        return;
    }

    fps_counter_frames++;
    int64_t now_us = esp_timer_get_time();
    int64_t dt_us = now_us - fps_counter_last_update_us;
    if (dt_us < 250000) {
        return;
    }

    uint32_t fps = (uint32_t)((fps_counter_frames * 1000000ULL + (uint64_t)(dt_us / 2)) / (uint64_t)dt_us);
    char buf[24];
    snprintf(buf, sizeof(buf), "FPS %02u", fps);
    lv_label_set_text(label_fps_counter, buf);

    fps_counter_frames = 0;
    fps_counter_last_update_us = now_us;
}

static void settings_update_blur_layers(void)
{
    if (settings_blur_top_overlay) {
        lv_obj_add_flag(settings_blur_top_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_blur_bottom_overlay) {
        lv_obj_add_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

static void update_settings_wheel_layout(void)
{
    const bool root_view = (settings_view == SETTINGS_VIEW_ROOT);
    const bool sensitivity_view = (settings_view == SETTINGS_VIEW_SENSITIVITY);
    const bool slider_view = (settings_view == SETTINGS_VIEW_SENSITIVITY) ||
                             (settings_view == SETTINGS_VIEW_BRIGHTNESS);
    const bool confirm_view = (settings_view == SETTINGS_VIEW_OVERDRIVE_WARNING) ||
                              (settings_view == SETTINGS_VIEW_PURIFICATION_CONFIRM);
    const lv_coord_t card_w = root_view ? 368 : 360;
    const lv_coord_t card_h = root_view ? 98 : 92;
    const lv_coord_t center_y = root_view ? ((EXAMPLE_LCD_V_RES - card_h) / 2) : (((EXAMPLE_LCD_V_RES - card_h) / 2) + 24);
    const lv_coord_t step_y = settings_wheel_step_for_view(settings_view);
    const bool wheel_list_view = settings_view_uses_wheel_list(settings_view);
    const bool fast_anim_pass = wheel_list_view && settings_root_scroll_animating;
    const float wheel_step = (float)step_y;
    float scroll_top = (float)settings_root_scroll_q / (float)SETTINGS_WHEEL_SCROLL_Q;

    if (wheel_list_view) {
        if (!settings_root_scroll_animating) {
            settings_root_scroll_q = settings_index_to_scroll_q_for_view(settings_index, settings_view);
        }
    } else if (settings_root_scroll_animating) {
        lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
        settings_root_scroll_animating = false;
    }
    scroll_top = (float)settings_root_scroll_q / (float)SETTINGS_WHEEL_SCROLL_Q;

    int visual_index = settings_index;
    if (wheel_list_view && settings_items_count > 0) {
        visual_index = clamp_int((int)lroundf(scroll_top / wheel_step), 0, settings_items_count - 1);
    }

    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        lv_obj_t *item = settings_items[i];
        if (!item) {
            continue;
        }
        if (sensitivity_view) {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        if (i >= settings_items_count) {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        int d = i - (wheel_list_view ? visual_index : settings_index);
        int ad = (d < 0) ? -d : d;
        bool selected = (ad == 0);
        float signed_dist_px = 0.0f;
        float distance_px = 0.0f;
        float normalized_dist = 1.0f;
        lv_opa_t wheel_item_opa = LV_OPA_COVER;
        lv_opa_t wheel_icon_opa = LV_OPA_COVER;
        lv_opa_t wheel_title_opa = LV_OPA_COVER;
        lv_opa_t wheel_value_opa = (lv_opa_t)230;
        lv_opa_t wheel_arrow_opa = (lv_opa_t)200;

        if (wheel_list_view) {
            signed_dist_px = ((float)i * wheel_step) - scroll_top;
            distance_px = fabsf(signed_dist_px);
            normalized_dist = fminf(distance_px, SETTINGS_WHEEL_MAX_DIST_PX) / SETTINGS_WHEEL_MAX_DIST_PX;

            wheel_item_opa = LV_OPA_COVER;
            wheel_icon_opa = LV_OPA_COVER;
            wheel_title_opa = LV_OPA_COVER;
            wheel_value_opa = (lv_opa_t)230;
            wheel_arrow_opa = (lv_opa_t)200;

            if (distance_px > (SETTINGS_WHEEL_MAX_DIST_PX + (SETTINGS_WHEEL_CULL_EXTRA_STEPS * wheel_step))) {
                lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
                continue;
            }
        }

        lv_coord_t item_w = card_w;
        lv_coord_t item_h = card_h;
        lv_coord_t y = center_y + d * step_y;
        if (wheel_list_view) {
            y = center_y +
                (lv_coord_t)lroundf(signed_dist_px + signed_dist_px * 0.1f) +
                (lv_coord_t)((card_h - item_h) / 2);
        }

        if (slider_view) {
            if (i == 0) {
                item_w = 352;
                item_h = 212;
                y = 66;
            } else if (i == 1) {
                item_w = 300;
                item_h = 74;
                y = 320;
            }
        } else if (confirm_view) {
            if (i == 0) {
                item_w = 72;
                item_h = 72;
            } else {
                item_w = 170;
                item_h = 72;
            }
            y = 324;
        }

        if (confirm_view) {
            lv_coord_t xoff = (i == 0) ? -70 : 66;
            lv_obj_set_size(item, item_w, item_h);
            lv_obj_align(item, LV_ALIGN_TOP_MID, xoff, y);
            lv_obj_set_style_radius(item, (i == 0) ? LV_RADIUS_CIRCLE : 36, 0);
            lv_obj_set_style_bg_color(item, (i == 0) ? lv_color_hex(0xF4F4F5) : lv_color_hex(0x27272A), 0);
            lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : (lv_opa_t)88, 0);
            lv_obj_set_style_border_width(item, selected ? 2 : 1, 0);
            lv_obj_set_style_border_color(item, selected ? lv_color_hex(0xA1A1AA) : lv_palette_darken(LV_PALETTE_GREY, 3), 0);
            lv_obj_set_style_border_opa(item, selected ? (lv_opa_t)86 : (lv_opa_t)35, 0);
            lv_obj_set_style_opa(item, LV_OPA_COVER, 0);

            if (settings_item_icon_bgs[i]) {
                lv_obj_add_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_icons[i]) {
                lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
                lv_obj_set_style_text_color(settings_item_icons[i], (i == 0) ? lv_color_hex(0x18181B) : lv_color_hex(0x7A7A83), 0);
                lv_label_set_text(settings_item_icons[i], (i == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : "");
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                if (i == 0) {
                    lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_add_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                }
            }
            if (settings_item_titles[i]) {
                lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_20_caps, 0);
                lv_obj_set_style_text_color(settings_item_titles[i], (i == 0) ? lv_color_hex(0x18181B) : lv_color_hex(0x7A7A83), 0);
                lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_width(settings_item_titles[i], item_w);
                lv_obj_align(settings_item_titles[i], LV_ALIGN_CENTER, 0, 0);
            }
            lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        lv_obj_set_size(item, item_w, item_h);
        if (root_view) {
            lv_coord_t item_x = (EXAMPLE_LCD_H_RES - item_w) / 2;
            lv_obj_set_x(item, item_x);
            lv_obj_set_y(item, y);
        } else {
            lv_obj_align(item, LV_ALIGN_TOP_MID, 0, y);
        }

        lv_obj_set_style_opa(item, wheel_item_opa, 0);

        bool visible = wheel_list_view
                           ? (distance_px <= (SETTINGS_WHEEL_MAX_DIST_PX + (SETTINGS_WHEEL_VISIBLE_EXTRA_STEPS * wheel_step)))
                           : (root_view ? (ad <= 3) : (ad <= 2));

        if (fast_anim_pass) {
            if (root_view) {
                lv_obj_set_style_bg_color(item, lv_color_hex(0x121214), 0);
                lv_obj_set_style_bg_opa(item, (lv_opa_t)88, 0);
                lv_obj_set_style_border_width(item, 1, 0);
                lv_opa_t fast_border_opa = (lv_opa_t)70;
                lv_obj_set_style_border_opa(item, fast_border_opa, 0);
                lv_obj_set_style_border_color(item, lv_color_hex(0x9F9FA9), 0);

                if (settings_item_icon_bgs[i]) {
                    lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_size(settings_item_icon_bgs[i], 64, 64);
                    lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, 0, 0);
                    lv_obj_set_style_bg_color(settings_item_icon_bgs[i], lv_color_hex(0x27272A), 0);
                    lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], (lv_opa_t)90, 0);
                    lv_obj_set_style_border_color(settings_item_icon_bgs[i], lv_color_hex(0x9F9FA9), 0);
                    lv_obj_set_style_border_opa(settings_item_icon_bgs[i], (lv_opa_t)36, 0);
                }
                if (settings_item_icons[i]) {
                    lv_obj_set_style_text_color(settings_item_icons[i], lv_color_hex(ui_accent_color_hex), 0);
                    lv_obj_set_style_text_opa(settings_item_icons[i], wheel_icon_opa, 0);
                }
                if (settings_item_titles[i]) {
                    lv_obj_set_style_text_color(settings_item_titles[i], lv_color_white(), 0);
                    lv_obj_set_style_text_opa(settings_item_titles[i], wheel_title_opa, 0);
                }
                if (settings_item_values[i]) {
                    if (settings_item_value_is_icon[i]) {
                        lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(ui_accent_color_hex), 0);
                    } else {
                        lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(0x5D5D66), 0);
                    }
                    lv_obj_set_style_text_opa(settings_item_values[i], wheel_value_opa, 0);
                }
                if (settings_item_arrows[i]) {
                    lv_obj_set_style_text_color(settings_item_arrows[i], lv_color_hex(0xA1A1AA), 0);
                    lv_obj_set_style_text_opa(settings_item_arrows[i], wheel_arrow_opa, 0);
                }
            }
            if (visible) {
                lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }

        lv_opa_t bg_opa = (lv_opa_t)(wheel_list_view ? 88 : (selected ? 84 : 64));
        lv_obj_set_style_bg_color(item, lv_color_hex(0x121214), 0);
        lv_obj_set_style_bg_opa(item, bg_opa, 0);
        lv_obj_set_style_radius(item, 30, 0);
        if (root_view) {
            lv_obj_set_style_border_width(item, 1, 0);
            lv_opa_t root_border_opa = (lv_opa_t)70;
            lv_obj_set_style_border_opa(item, root_border_opa, 0);
            lv_obj_set_style_border_color(item, lv_color_hex(0x9F9FA9), 0);
        } else {
            lv_obj_set_style_border_width(item, selected ? 3 : 1, 0);
            lv_obj_set_style_border_opa(item, selected ? (lv_opa_t)86 : (lv_opa_t)34, 0);
            lv_obj_set_style_border_color(item,
                                          selected ? (settings_edit_mode ? lv_color_hex(ui_accent_color_hex)
                                                                         : lv_color_hex(0xA1A1AA))
                                                   : lv_palette_darken(LV_PALETTE_GREY, 3),
                                          0);
        }

        if (settings_item_icon_bgs[i]) {
            lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
            int icon_bg_size = root_view ? 64 : (selected ? 74 : 64);
            lv_obj_set_size(settings_item_icon_bgs[i], icon_bg_size, icon_bg_size);
            lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, 0, 0);
            if (root_view && wheel_list_view) {
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], lv_color_hex(0x27272A), 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], (lv_opa_t)90, 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], lv_color_hex(0x9F9FA9), 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], (lv_opa_t)36, 0);
            } else {
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], selected ? lv_color_hex(0x2C2D31) : lv_color_hex(0x222327), 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], selected ? (lv_opa_t)102 : ((ad == 1) ? (lv_opa_t)88 : ((ad == 2) ? (lv_opa_t)58 : (lv_opa_t)34)), 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], selected ? (lv_opa_t)60 : (lv_opa_t)30, 0);
            }
        }

        if (settings_item_icons[i]) {
            lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
            lv_color_t icon_color = root_view ? lv_color_hex(ui_accent_color_hex)
                                              : (selected ? lv_color_hex(ui_accent_color_hex) : lv_color_hex(0x8A8A91));
            lv_obj_set_style_text_color(settings_item_icons[i],
                                        icon_color,
                                        0);
            lv_obj_set_style_text_opa(settings_item_icons[i],
                                      wheel_list_view ? wheel_icon_opa
                                                      : (selected ? LV_OPA_COVER
                                                                  : ((ad == 1) ? (lv_opa_t)92 : ((ad == 2) ? (lv_opa_t)66 : (lv_opa_t)42))),
                                      0);
            lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 1, 0);
        }

        if (settings_item_titles[i]) {
            const lv_font_t *title_font = root_view ? &lv_font_montserrat_20
                                                    : (selected ? &lv_font_montserrat_20 : &lv_font_inter_medium_16_ui);
            lv_obj_set_style_text_font(settings_item_titles[i], title_font, 0);
            lv_obj_set_style_text_color(settings_item_titles[i], root_view ? lv_color_white()
                                                                            : (selected ? lv_color_white() : lv_color_hex(0xD4D4D8)),
                                        0);
            lv_obj_set_style_text_opa(settings_item_titles[i],
                                      wheel_list_view ? wheel_title_opa
                                                      : (selected ? LV_OPA_COVER
                                                                  : ((ad == 1) ? (lv_opa_t)88 : ((ad == 2) ? (lv_opa_t)54 : (lv_opa_t)34))),
                                      0);
            lv_obj_align(settings_item_titles[i], LV_ALIGN_LEFT_MID, 100, (slider_view && i == 0) ? -54 : -18);
            lv_obj_set_width(settings_item_titles[i], item_w - 140);
            lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_LEFT, 0);
        }

        if (settings_item_values[i]) {
            lv_obj_clear_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            if (settings_item_value_is_icon[i]) {
                lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(ui_accent_color_hex), 0);
                lv_obj_set_style_text_opa(settings_item_values[i],
                                          wheel_list_view ? wheel_value_opa
                                                          : (selected ? LV_OPA_COVER : (lv_opa_t)80),
                                          0);
                lv_obj_align(settings_item_values[i], LV_ALIGN_RIGHT_MID, -20, 0);
            } else {
                const lv_font_t *value_font = root_view ? &lv_font_inter_medium_16_ui
                                                        : (selected ? &lv_font_inter_medium_16_ui : &lv_font_montserrat_14);
                lv_obj_set_style_text_font(settings_item_values[i], value_font, 0);
                lv_obj_set_style_text_color(settings_item_values[i], root_view ? lv_color_hex(0x5D5D66)
                                                                                : (selected ? lv_color_hex(0xD4D4D8) : lv_color_hex(0xA1A1AA)),
                                            0);
                lv_obj_set_style_text_opa(settings_item_values[i],
                                          wheel_list_view ? wheel_value_opa
                                                          : (selected ? LV_OPA_COVER
                                                                      : ((ad == 1) ? (lv_opa_t)86 : ((ad == 2) ? (lv_opa_t)52 : (lv_opa_t)32))),
                                          0);
                lv_obj_align(settings_item_values[i], LV_ALIGN_LEFT_MID, 100, (slider_view && i == 0) ? -8 : 22);
            }
        }

        if (settings_item_arrows[i]) {
            lv_obj_set_style_text_color(settings_item_arrows[i],
                                        root_view ? lv_color_hex(0xA1A1AA)
                                                  : (selected ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x7A7A83)),
                                        0);
            lv_obj_set_style_text_opa(settings_item_arrows[i],
                                      wheel_list_view ? wheel_arrow_opa
                                                      : (selected ? (lv_opa_t)98 : ((ad == 1) ? (lv_opa_t)74 : ((ad == 2) ? (lv_opa_t)46 : (lv_opa_t)28))),
                                      0);
            lv_obj_align(settings_item_arrows[i], LV_ALIGN_RIGHT_MID, 24, 0);
        }

        if (slider_view) {
            visible = (i == 0);
            if (i == 0) {
                lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(item, 0, 0);
                if (settings_item_icon_bgs[i]) {
                    lv_obj_add_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                }
                if (settings_item_icons[i]) {
                    lv_obj_add_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                }
                if (settings_item_arrows[i]) {
                    lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
                }
                if (settings_item_titles[i]) {
                    lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_20_caps, 0);
                    lv_obj_set_style_text_color(settings_item_titles[i], lv_color_hex(0x8A8A91), 0);
                    lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_set_width(settings_item_titles[i], 280);
                    lv_obj_align(settings_item_titles[i], LV_ALIGN_TOP_MID, 0, 36);
                }
                if (settings_item_values[i]) {
                    lv_obj_set_style_text_font(settings_item_values[i], &lv_font_inter_medium_145_num, 0);
                    lv_obj_set_style_text_color(settings_item_values[i], lv_color_white(), 0);
                    lv_obj_set_style_text_align(settings_item_values[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_set_width(settings_item_values[i], 280);
                    lv_obj_align(settings_item_values[i], LV_ALIGN_TOP_MID, 0, 98);
                }
            }
        }

        if (visible) {
            lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_focus_frame) {
        if (root_view) {
            lv_obj_clear_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_focus_frame, card_w, card_h);
            lv_obj_set_x(settings_focus_frame, (EXAMPLE_LCD_H_RES - card_w) / 2);
            lv_obj_set_y(settings_focus_frame, center_y);
            lv_obj_move_foreground(settings_focus_frame);
        } else {
            lv_obj_add_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_blur_top_overlay && settings_blur_bottom_overlay) {
        lv_obj_add_flag(settings_blur_top_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_HIDDEN);
    }

    if (settings_value_slider_track && settings_value_slider_fill && settings_value_slider_knob) {
        bool show_slider = slider_view;
        if (show_slider) {
            lv_obj_clear_flag(settings_value_slider_track, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_value_slider_fill, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
            if (settings_view == SETTINGS_VIEW_SENSITIVITY) {
                lv_obj_set_size(settings_value_slider_track, 288, 16);
                lv_obj_align(settings_value_slider_track, LV_ALIGN_TOP_MID, 0, 294);
            } else {
                lv_obj_set_size(settings_value_slider_track, 220, 18);
                lv_obj_align(settings_value_slider_track, LV_ALIGN_BOTTOM_LEFT, 96, -24);
            }

            int min_value = 0;
            int max_value = 100;
            int current = 0;
            if (settings_view == SETTINGS_VIEW_SENSITIVITY) {
                min_value = SENSITIVITY_MIN_CLICKS;
                max_value = SENSITIVITY_MAX_CLICKS;
                current = speed_sensitivity_clicks;
            } else {
                min_value = DISPLAY_BRIGHTNESS_MIN;
                max_value = DISPLAY_BRIGHTNESS_MAX;
                current = display_brightness_percent;
            }

            int span = max_value - min_value;
            int pos = (span > 0) ? ((current - min_value) * 1000) / span : 0;
            pos = clamp_int(pos, 0, 1000);

            lv_coord_t track_w = lv_obj_get_width(settings_value_slider_track);
            lv_coord_t track_h = lv_obj_get_height(settings_value_slider_track);
            lv_obj_set_height(settings_value_slider_fill, track_h);
            lv_coord_t fill_w = (lv_coord_t)((track_w * pos + 500) / 1000);
            if (fill_w < 0) {
                fill_w = 0;
            }
            if (fill_w > track_w) {
                fill_w = track_w;
            }
            lv_obj_set_width(settings_value_slider_fill, fill_w);

            lv_coord_t knob_w = lv_obj_get_width(settings_value_slider_knob);
            lv_coord_t knob_x = fill_w - (knob_w / 2);
            if (knob_x < -(knob_w / 2)) {
                knob_x = -(knob_w / 2);
            }
            if (knob_x > (track_w - (knob_w / 2))) {
                knob_x = track_w - (knob_w / 2);
            }
            lv_obj_set_pos(settings_value_slider_knob,
                           knob_x,
                           -(lv_obj_get_height(settings_value_slider_knob) - lv_obj_get_height(settings_value_slider_track)) / 2);
        } else {
            lv_obj_add_flag(settings_value_slider_track, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_value_slider_fill, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (label_settings_title) {
        if (sensitivity_view) {
            lv_obj_add_flag(label_settings_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(label_settings_title, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_sensitivity_title && settings_sensitivity_value && settings_sensitivity_subtitle) {
        if (sensitivity_view) {
            char sens_buf[12];
            snprintf(sens_buf, sizeof(sens_buf), "%d", speed_sensitivity_clicks);
            lv_label_set_text(settings_sensitivity_value, sens_buf);
            lv_obj_align(settings_sensitivity_title, LV_ALIGN_TOP_MID, 0, 66);
            lv_obj_align(settings_sensitivity_value, LV_ALIGN_TOP_MID, 0, 96);
            lv_obj_align(settings_sensitivity_subtitle, LV_ALIGN_TOP_MID, 0, 252);
            lv_obj_clear_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(settings_sensitivity_title);
            lv_obj_move_foreground(settings_sensitivity_subtitle);
            lv_obj_move_foreground(settings_sensitivity_value);
            if (settings_value_slider_track) {
                lv_obj_set_size(settings_value_slider_track, 288, 16);
                lv_obj_align(settings_value_slider_track, LV_ALIGN_TOP_MID, 0, 294);
                lv_obj_set_style_bg_color(settings_value_slider_track, lv_color_hex(0x3F3F46), 0);
                lv_obj_set_style_bg_opa(settings_value_slider_track, LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_value_slider_track);
            }
            if (settings_value_slider_fill) {
                lv_obj_set_style_bg_color(settings_value_slider_fill, lv_color_hex(ui_accent_color_hex), 0);
                lv_obj_set_style_bg_opa(settings_value_slider_fill, LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_value_slider_fill);
            }
            if (settings_value_slider_knob) {
                lv_obj_set_style_bg_color(settings_value_slider_knob, lv_color_hex(ui_accent_color_hex), 0);
                lv_obj_set_style_border_color(settings_value_slider_knob, lv_color_white(), 0);
                lv_obj_set_style_border_width(settings_value_slider_knob, 3, 0);
                lv_obj_move_foreground(settings_value_slider_knob);
            }
        } else {
            lv_obj_add_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
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

static void rebuild_language_options(void)
{
    size_t off = 0;
    language_roller_options[0] = '\0';
    for (int i = 0; i < LANG_COUNT; ++i) {
        int written = snprintf(language_roller_options + off,
                               sizeof(language_roller_options) - off,
                               "%s%s",
                               language_names[i],
                               (i < (LANG_COUNT - 1)) ? "\n" : "");
        if (written <= 0) {
            break;
        }
        off += (size_t)written;
        if (off >= sizeof(language_roller_options)) {
            language_roller_options[sizeof(language_roller_options) - 1] = '\0';
            break;
        }
    }
}

static esp_err_t prefill_panel_black(void)
{
    static uint16_t black_tile[EXAMPLE_LCD_H_RES * 8];
    const int tile_h = 8;
    memset(black_tile, 0, sizeof(black_tile));

    for (int y = 0; y < EXAMPLE_LCD_V_RES; y += tile_h) {
        int h = tile_h;
        if (y + h > EXAMPLE_LCD_V_RES) {
            h = EXAMPLE_LCD_V_RES - y;
        }
        esp_err_t err = esp_lcd_panel_draw_bitmap(lcd_panel, 0, y, EXAMPLE_LCD_H_RES, y + h, black_tile);
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to LVGL ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LVGL_BUFF_SIZE (EXAMPLE_LCD_H_RES * 40)
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 50
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE (8 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY 2

#define HF_ws2812 0

void my_print(lv_log_level_t level, const char *buf)
{
    esp_log_level_t lvl = ESP_LOG_NONE;

    switch (level)
    {
    case LV_LOG_LEVEL_TRACE:
        lvl = ESP_LOG_VERBOSE;
        break;
    case LV_LOG_LEVEL_INFO:
        lvl = ESP_LOG_INFO;
        break;
    case LV_LOG_LEVEL_WARN:
        lvl = ESP_LOG_WARN;
        break;
    case LV_LOG_LEVEL_ERROR:
        lvl = ESP_LOG_ERROR;
        break;
    case LV_LOG_LEVEL_USER:
        lvl = ESP_LOG_DEBUG;
        break;
    case LV_LOG_LEVEL_NONE:
        lvl = ESP_LOG_NONE;
        break;
    }
    ESP_LOG_LEVEL(lvl, TAG, "%s", buf);
}

static int get_max_speed_percent(void)
{
    if (allow_overdrive && filter_status == FILTER_STATUS_CRITICAL) {
        return FAN_SPEED_OVERDRIVE_MAX_PERCENT;
    }
    return FAN_SPEED_NORMAL_MAX_PERCENT;
}

static void clamp_speed_to_limits(void)
{
    int max_speed = get_max_speed_percent();
    if (fan_speed_tenths < FAN_SPEED_MIN_PERCENT * 10) {
        fan_speed_tenths = FAN_SPEED_MIN_PERCENT * 10;
    }
    if (fan_speed_tenths > max_speed * 10) {
        fan_speed_tenths = max_speed * 10;
    }
    fan_speed_percent = (fan_speed_tenths + 5) / 10;
}

static void set_speed_percent(int speed_percent)
{
    fan_speed_tenths = speed_percent * 10;
    clamp_speed_to_limits();
}

static void init_pwm_fan(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = FAN_PWM_RESOLUTION,
        .freq_hz = FAN_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    ledc_channel_config_t chan_conf = {
        .gpio_num = BSP_FAN_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&chan_conf));
}

static void apply_fan_pwm(int percent)
{
    if (percent < 0) {
        percent = 0;
    }
    if (percent > 100) {
        percent = 100;
    }
    uint32_t duty = (uint32_t)((percent * FAN_PWM_MAX_DUTY) / 100);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void save_owner_name(void)
{
    nvs_handle_t handle;
    if (nvs_open("settings", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_str(handle, "owner", owner_name);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static void save_language(void)
{
    nvs_handle_t handle;
    if (nvs_open("settings", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_u8(handle, "lang", (uint8_t)current_lang);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static void save_control_settings(void)
{
    nvs_handle_t handle;
    if (nvs_open("settings", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_u8(handle, "sense", (uint8_t)speed_sensitivity_clicks);
        nvs_set_u8(handle, "ovrd", allow_overdrive ? 1 : 0);
        nvs_set_u8(handle, "chld", allow_child_lock ? 1 : 0);
        nvs_set_u8(handle, "bright", (uint8_t)display_brightness_percent);
        nvs_set_u8(handle, "theme", display_theme_dark ? 1 : 0);
        nvs_set_u16(handle, "rot", (uint16_t)display_rotation_deg);
        nvs_set_u8(handle, "accent", (uint8_t)display_accent_index);
        nvs_set_u16(handle, "fused", (uint16_t)filter_used_hours);
        nvs_set_u16(handle, "flife", (uint16_t)filter_lifetime_hours);
        nvs_set_u8(handle, "lang", (uint8_t)current_lang);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static void load_settings(void)
{
    nvs_handle_t handle;
    if (nvs_open("settings", NVS_READONLY, &handle) == ESP_OK) {
        uint8_t lang = 0;
        uint8_t sense = SENSITIVITY_DEFAULT_CLICKS;
        uint8_t ovrd = 0;
        uint8_t chld = 1;
        uint8_t bright = DISPLAY_BRIGHTNESS_DEFAULT;
        uint8_t theme = 1;
        uint16_t rot = 0;
        uint8_t accent = 0;
        uint16_t fused = FILTER_USED_DEFAULT_HOURS;
        uint16_t flife = FILTER_LIFETIME_DEFAULT_HOURS;
        if (nvs_get_u8(handle, "lang", &lang) == ESP_OK && lang < LANG_COUNT) {
            current_lang = (ui_lang_t)lang;
        }
        if (nvs_get_u8(handle, "sense", &sense) == ESP_OK) {
            if (sense >= SENSITIVITY_MIN_CLICKS && sense <= SENSITIVITY_MAX_CLICKS) {
                speed_sensitivity_clicks = sense;
            }
        }
        if (nvs_get_u8(handle, "ovrd", &ovrd) == ESP_OK) {
            allow_overdrive = (ovrd != 0);
        }
        if (nvs_get_u8(handle, "chld", &chld) == ESP_OK) {
            allow_child_lock = (chld != 0);
        }
        if (nvs_get_u8(handle, "bright", &bright) == ESP_OK) {
            if (bright <= DISPLAY_BRIGHTNESS_MAX) {
                display_brightness_percent = bright;
            }
        }
        if (nvs_get_u8(handle, "theme", &theme) == ESP_OK) {
            display_theme_dark = (theme != 0);
        }
        if (nvs_get_u16(handle, "rot", &rot) == ESP_OK) {
            if ((rot % DISPLAY_ROTATION_STEP_DEG) == 0 && rot <= 270) {
                display_rotation_deg = rot;
            }
        }
        if (nvs_get_u8(handle, "accent", &accent) == ESP_OK) {
            if (accent < DISPLAY_ACCENT_COUNT) {
                display_accent_index = accent;
            }
        }
        if (nvs_get_u16(handle, "fused", &fused) == ESP_OK) {
            filter_used_hours = fused;
        }
        if (nvs_get_u16(handle, "flife", &flife) == ESP_OK) {
            if (flife >= FILTER_LIFETIME_MIN_HOURS && flife <= FILTER_LIFETIME_MAX_HOURS) {
                filter_lifetime_hours = flife;
            }
        }
        size_t len = sizeof(owner_name);
        if (nvs_get_str(handle, "owner", owner_name, &len) == ESP_OK) {
            owner_name_len = strnlen(owner_name, sizeof(owner_name) - 1);
        }
        nvs_close(handle);
    }
    if (filter_lifetime_hours < FILTER_LIFETIME_MIN_HOURS) {
        filter_lifetime_hours = FILTER_LIFETIME_MIN_HOURS;
    }
    if (filter_lifetime_hours > FILTER_LIFETIME_MAX_HOURS) {
        filter_lifetime_hours = FILTER_LIFETIME_MAX_HOURS;
    }
    if (filter_used_hours < 0) {
        filter_used_hours = 0;
    }
    if (filter_used_hours > filter_lifetime_hours) {
        filter_used_hours = filter_lifetime_hours;
    }
    if (display_brightness_percent < DISPLAY_BRIGHTNESS_MIN) {
        display_brightness_percent = DISPLAY_BRIGHTNESS_MIN;
    }
    if (display_brightness_percent > DISPLAY_BRIGHTNESS_MAX) {
        display_brightness_percent = DISPLAY_BRIGHTNESS_MAX;
    }
    if (display_accent_index < 0 || display_accent_index >= DISPLAY_ACCENT_COUNT) {
        display_accent_index = 0;
    }
}

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void refresh_filter_status_from_usage(void)
{
    if (filter_lifetime_hours <= 0) {
        filter_status = FILTER_STATUS_CRITICAL;
        return;
    }
    int wear_percent = (filter_used_hours * 100) / filter_lifetime_hours;
    if (wear_percent >= 100) {
        filter_status = FILTER_STATUS_CRITICAL;
    } else if (wear_percent >= 80) {
        filter_status = FILTER_STATUS_WARNING;
    } else {
        filter_status = FILTER_STATUS_GOOD;
    }
}

static void apply_runtime_visual_settings(void)
{
    display_accent_index = clamp_int(display_accent_index, 0, DISPLAY_ACCENT_COUNT - 1);
    ui_accent_color_hex = accent_palette_hex[display_accent_index];
    filter_lifetime_hours = clamp_int(filter_lifetime_hours, FILTER_LIFETIME_MIN_HOURS, FILTER_LIFETIME_MAX_HOURS);
    filter_used_hours = clamp_int(filter_used_hours, 0, filter_lifetime_hours);
    refresh_filter_status_from_usage();
}

static const char *settings_get_view_title(void)
{
    switch (settings_view) {
    case SETTINGS_VIEW_SENSITIVITY:
        return "SENSITIVITY";
    case SETTINGS_VIEW_ACCENT:
        return "ACCENT COLOR";
    case SETTINGS_VIEW_LANGUAGE:
        return "LANGUAGE";
    case SETTINGS_VIEW_BRIGHTNESS:
        return "BRIGHTNESS";
    case SETTINGS_VIEW_THEME:
        return "THEME";
    case SETTINGS_VIEW_ROTATION:
        return "ROTATION";
    case SETTINGS_VIEW_LOCK_MODE:
        return "LOCK MODE";
    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return "OVERDRIVE WARNING";
    case SETTINGS_VIEW_OVERDRIVE_MODE:
        return "OVERDRIVE MODE";
    case SETTINGS_VIEW_FILTER:
        return settings_edit_mode ? "FILTER  EDIT" : "FILTER";
    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return "AIR PURIFICATION";
    case SETTINGS_VIEW_ABOUT:
        return "ABOUT";
    case SETTINGS_VIEW_ROOT:
    default:
        return "SETTINGS";
    }
}

static int settings_get_item_count(void)
{
    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        return SETTINGS_ROOT_ITEMS_COUNT;
    case SETTINGS_VIEW_SENSITIVITY:
        return 1;
    case SETTINGS_VIEW_ACCENT:
        return DISPLAY_ACCENT_COUNT + 1;
    case SETTINGS_VIEW_LANGUAGE:
        return SETTINGS_LANG_OPTIONS_COUNT + 1;
    case SETTINGS_VIEW_BRIGHTNESS:
        return 2;
    case SETTINGS_VIEW_THEME:
        return 3;
    case SETTINGS_VIEW_ROTATION:
        return 3;
    case SETTINGS_VIEW_LOCK_MODE:
        return 3;
    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return 2;
    case SETTINGS_VIEW_OVERDRIVE_MODE:
        return 3;
    case SETTINGS_VIEW_FILTER:
        return 3;
    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return 2;
    case SETTINGS_VIEW_ABOUT:
        return 4;
    default:
        return 1;
    }
}

static bool settings_item_has_arrow(int index)
{
    (void)index;
    return (settings_view == SETTINGS_VIEW_ROOT);
}

static const char *settings_get_item_title(int index)
{
    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (index) {
        case 0: return "Sensitivity";
        case 1: return "Accent Color";
        case 2: return "Language";
        case 3: return "Brightness";
        case 4: return "Theme";
        case 5: return "Rotation";
        case 6: return "Lock Mode";
        case 7: return "Overdrive Mode";
        case 8: return "Filter";
        case 9: return "Air Purification";
        default: return "About";
        }

    case SETTINGS_VIEW_SENSITIVITY:
        return (index == 0) ? "Sensitivity" : "Back";

    case SETTINGS_VIEW_ACCENT:
        return (index < DISPLAY_ACCENT_COUNT) ? accent_palette_names[index] : "Back";

    case SETTINGS_VIEW_LANGUAGE:
        switch (index) {
        case 0: return "English";
        case 1: return "Russian";
        case 2: return "Deutsch";
        case 3: return "Francais";
        default: return "Back";
        }

    case SETTINGS_VIEW_BRIGHTNESS:
        return (index == 0) ? "Brightness" : "Back";

    case SETTINGS_VIEW_THEME:
        switch (index) {
        case 0: return "Dark";
        case 1: return "Light";
        default: return "Back";
        }

    case SETTINGS_VIEW_ROTATION:
        switch (index) {
        case 0: return "0 deg";
        case 1: return "180 deg";
        default: return "Back";
        }

    case SETTINGS_VIEW_LOCK_MODE:
        switch (index) {
        case 0: return "Auto lock";
        case 1: return "Always unlocked";
        default: return "Back";
        }

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return (index == 0) ? "Back" : "Continue";

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        switch (index) {
        case 0: return "Enabled";
        case 1: return "Disabled";
        default: return "Back";
        }

    case SETTINGS_VIEW_FILTER:
        switch (index) {
        case 0: return "Set lifetime";
        case 1: return "Reset usage";
        default: return "Back";
        }

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return (index == 0) ? "Back" : "Start";

    case SETTINGS_VIEW_ABOUT:
        switch (index) {
        case 0: return "Model";
        case 1: return "Firmware";
        case 2: return "Interface";
        default: return "Back";
        }

    default:
        return "";
    }
}

static const char *settings_get_item_icon(int index)
{
    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (index) {
        case 0: return UI_ICON_LUCIDE_GAUGE;
        case 1: return UI_ICON_LUCIDE_PALETTE;
        case 2: return UI_ICON_LUCIDE_GLOBE;
        case 3: return UI_ICON_LUCIDE_SUN;
        case 4: return display_theme_dark ? UI_ICON_LUCIDE_MOON : UI_ICON_LUCIDE_SUN;
        case 5: return UI_ICON_LUCIDE_ROTATE_CW;
        case 6: return UI_ICON_LUCIDE_LOCK;
        case 7: return UI_ICON_LUCIDE_ZAP;
        case 8: return UI_ICON_LUCIDE_FILTER;
        case 9: return UI_ICON_LUCIDE_WIND;
        default: return UI_ICON_LUCIDE_INFO;
        }

    case SETTINGS_VIEW_SENSITIVITY:
        return (index == 0) ? UI_ICON_LUCIDE_GAUGE : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_ACCENT:
        return (index < DISPLAY_ACCENT_COUNT) ? UI_ICON_LUCIDE_PALETTE : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_LANGUAGE:
        return (index < SETTINGS_LANG_OPTIONS_COUNT) ? UI_ICON_LUCIDE_GLOBE : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_BRIGHTNESS:
        return (index == 0) ? UI_ICON_LUCIDE_SUN : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_THEME:
        if (index == 0) {
            return UI_ICON_LUCIDE_MOON;
        }
        if (index == 1) {
            return UI_ICON_LUCIDE_SUN;
        }
        return UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_ROTATION:
        return (index < 2) ? UI_ICON_LUCIDE_ROTATE_CW : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_LOCK_MODE:
        return (index < 2) ? UI_ICON_LUCIDE_LOCK : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return (index == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_ALERT_TRIANGLE;

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        return (index < 2) ? UI_ICON_LUCIDE_ZAP : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_FILTER:
        if (index == 0) {
            return UI_ICON_LUCIDE_FILTER;
        }
        if (index == 1) {
            return UI_ICON_LUCIDE_REFRESH_CW;
        }
        return UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return (index == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_WIND;

    case SETTINGS_VIEW_ABOUT:
        return (index < 3) ? UI_ICON_LUCIDE_INFO : UI_ICON_LUCIDE_CHEVRON_LEFT;

    default:
        return UI_ICON_LUCIDE_INFO;
    }
}

static void settings_get_item_value(int index, char *buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return;
    }
    buf[0] = '\0';

    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (index) {
        case 0:
            snprintf(buf, buf_len, "%d clicks", speed_sensitivity_clicks);
            break;
        case 1:
            snprintf(buf, buf_len, "%s", accent_palette_names[display_accent_index]);
            break;
        case 2:
            snprintf(buf, buf_len, "%s", language_names[current_lang]);
            break;
        case 3:
            snprintf(buf, buf_len, "%d%%", display_brightness_percent);
            break;
        case 4:
            snprintf(buf, buf_len, "%s", display_theme_dark ? "Dark" : "Light");
            break;
        case 5:
            snprintf(buf, buf_len, "%ddeg", display_rotation_deg);
            break;
        case 6:
            snprintf(buf, buf_len, "%s", allow_child_lock ? "Auto lock" : "Always unlocked");
            break;
        case 7:
            snprintf(buf, buf_len, "%s", allow_overdrive ? "Enabled" : "Disabled");
            break;
        case 8:
            snprintf(buf, buf_len, "%dh / %dh", filter_used_hours, filter_lifetime_hours);
            break;
        case 9:
            snprintf(buf, buf_len, "%s", is_purifying ? "Running" : "Clean air mode");
            break;
        default:
            snprintf(buf, buf_len, "FW 2.4.1");
            break;
        }
        break;

    case SETTINGS_VIEW_SENSITIVITY:
        if (index == 0) {
            snprintf(buf, buf_len, "%d", speed_sensitivity_clicks);
        }
        break;

    case SETTINGS_VIEW_ACCENT:
        if (index < DISPLAY_ACCENT_COUNT) {
            snprintf(buf, buf_len, "%s", (index == display_accent_index) ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_LANGUAGE:
        if (index < SETTINGS_LANG_OPTIONS_COUNT) {
            int selected = (index == 0 && current_lang == LANG_EN) ||
                           (index == 1 && current_lang == LANG_RU) ||
                           (index == 2 && current_lang == LANG_DE) ||
                           (index == 3 && current_lang == LANG_FR);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_BRIGHTNESS:
        if (index == 0) {
            snprintf(buf, buf_len, "%d", display_brightness_percent);
        }
        break;

    case SETTINGS_VIEW_THEME:
        if (index < 2) {
            bool selected = (display_theme_dark && index == 0) || (!display_theme_dark && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_ROTATION:
        if (index < 2) {
            bool selected = (display_rotation_deg == 0 && index == 0) ||
                            (display_rotation_deg == 180 && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_LOCK_MODE:
        if (index < 2) {
            bool selected = (allow_child_lock && index == 0) || (!allow_child_lock && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        if (index == 1) {
            snprintf(buf, buf_len, "Motor wear warning");
        }
        break;

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        if (index < 2) {
            bool selected = (allow_overdrive && index == 0) || (!allow_overdrive && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_FILTER:
        if (index == 0) {
            snprintf(buf, buf_len, settings_edit_mode ? "EDIT  %dh" : "%dh", filter_lifetime_hours);
        } else if (index == 1) {
            snprintf(buf, buf_len, "used %dh", filter_used_hours);
        }
        break;

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        if (index == 1) {
            snprintf(buf, buf_len, "Clean air mode");
        }
        break;

    case SETTINGS_VIEW_ABOUT:
        if (index == 0) {
            snprintf(buf, buf_len, "BELOM A2");
        } else if (index == 1) {
            snprintf(buf, buf_len, "2.4.1");
        } else if (index == 2) {
            snprintf(buf, buf_len, "1.0.0");
        }
        break;

    default:
        break;
    }
}

static void settings_refresh_view(void)
{
    const bool sensitivity_view = (settings_view == SETTINGS_VIEW_SENSITIVITY);
    const bool slider_view = (settings_view == SETTINGS_VIEW_SENSITIVITY) ||
                             (settings_view == SETTINGS_VIEW_BRIGHTNESS);
    const bool overdrive_warning_view = (settings_view == SETTINGS_VIEW_OVERDRIVE_WARNING);
    const bool purification_confirm_view = (settings_view == SETTINGS_VIEW_PURIFICATION_CONFIRM);
    const bool confirm_view = overdrive_warning_view || purification_confirm_view;

    settings_items_count = settings_get_item_count();
    if (settings_items_count < 1) {
        settings_items_count = 1;
    }
    settings_index = clamp_int(settings_index, 0, settings_items_count - 1);

    if (label_settings_title) {
        lv_label_set_text(label_settings_title, settings_get_view_title());
    }

    char value_buf[48];
    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        settings_item_value_is_icon[i] = false;
        if (sensitivity_view) {
            if (settings_items[i]) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }
        if (i >= settings_items_count) {
            if (settings_items[i]) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }

        if (settings_item_icons[i]) {
            lv_label_set_text(settings_item_icons[i], settings_get_item_icon(i));
        }

        if (settings_item_titles[i]) {
            lv_label_set_text(settings_item_titles[i], settings_get_item_title(i));
        }

        if (settings_item_values[i]) {
            settings_get_item_value(i, value_buf, sizeof(value_buf));
            lv_label_set_text(settings_item_values[i], value_buf);
            bool is_icon_value = (value_buf[0] != '\0' && ((unsigned char)value_buf[0] >= 0xE0));
            settings_item_value_is_icon[i] = is_icon_value;
            if (is_icon_value) {
                lv_obj_set_style_text_font(settings_item_values[i], ui_font_lucide_20(), 0);
            } else {
                lv_obj_set_style_text_font(settings_item_values[i], &lv_font_montserrat_14, 0);
            }
        }

        if (settings_item_arrows[i]) {
            if (settings_item_has_arrow(i)) {
                lv_obj_clear_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    if (settings_slider_caption) {
        if (slider_view) {
            if (settings_view == SETTINGS_VIEW_SENSITIVITY) {
                lv_obj_add_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(settings_slider_caption, "PERCENT BRIGHTNESS");
                lv_obj_set_style_text_color(settings_slider_caption, lv_color_hex(0x5D5D66), 0);
                lv_obj_align(settings_slider_caption, LV_ALIGN_TOP_MID, 0, 168);
            }
        } else {
            lv_obj_add_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (confirm_view) {
        if (settings_item_titles[0]) {
            lv_label_set_text(settings_item_titles[0], "");
        }
        if (settings_item_icons[0]) {
            lv_label_set_text(settings_item_icons[0], UI_ICON_LUCIDE_CHEVRON_LEFT);
        }
        if (settings_item_values[0]) {
            lv_obj_add_flag(settings_item_values[0], LV_OBJ_FLAG_HIDDEN);
        }

        if (settings_item_titles[1]) {
            lv_label_set_text(settings_item_titles[1], overdrive_warning_view ? "CONTINUE" : "START");
        }
        if (settings_item_values[1]) {
            lv_obj_add_flag(settings_item_values[1], LV_OBJ_FLAG_HIDDEN);
        }

        if (settings_overlay_icon_bg) {
            lv_obj_clear_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(settings_overlay_icon_bg, LV_ALIGN_TOP_MID, 0, 98);
            lv_obj_set_style_bg_color(settings_overlay_icon_bg,
                                      overdrive_warning_view ? lv_color_hex(0x3F2A18) : lv_color_hex(0x053C35),
                                      0);
        }
        if (settings_overlay_icon) {
            lv_label_set_text(settings_overlay_icon,
                              overdrive_warning_view ? UI_ICON_LUCIDE_ALERT_TRIANGLE : UI_ICON_LUCIDE_WIND);
            lv_obj_set_style_text_color(settings_overlay_icon,
                                        overdrive_warning_view ? lv_color_hex(0xF59E0B) : lv_color_hex(0x10B981),
                                        0);
        }
        if (settings_overlay_title) {
            lv_obj_clear_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_title,
                              overdrive_warning_view ? "Overdrive Mode" : "Air Purification");
            lv_obj_align(settings_overlay_title, LV_ALIGN_TOP_MID, 0, 178);
        }
        if (settings_overlay_line1) {
            lv_obj_clear_flag(settings_overlay_line1, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_line1,
                              overdrive_warning_view ? "Allows fan speed up to 110% when filter is critical"
                                                     : "Please ensure an appropriate filter");
            lv_obj_align(settings_overlay_line1, LV_ALIGN_TOP_MID, 0, 222);
        }
        if (settings_overlay_line2) {
            lv_obj_clear_flag(settings_overlay_line2, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_line2,
                              overdrive_warning_view ? "Active only for 20 hours"
                                                     : "is installed for air cleaning.");
            lv_obj_align(settings_overlay_line2, LV_ALIGN_TOP_MID, 0, 248);
        }
        if (settings_overlay_line3) {
            lv_obj_clear_flag(settings_overlay_line3, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_line3,
                              overdrive_warning_view ? "then automatically returns to 100%"
                                                     : "Fan will run at 10% speed.");
            lv_obj_align(settings_overlay_line3, LV_ALIGN_TOP_MID, 0, 274);
        }
    } else {
        if (settings_overlay_icon_bg) {
            lv_obj_add_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_title) {
            lv_obj_add_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_line1) {
            lv_obj_add_flag(settings_overlay_line1, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_line2) {
            lv_obj_add_flag(settings_overlay_line2, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_line3) {
            lv_obj_add_flag(settings_overlay_line3, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_view_uses_wheel_list(settings_view) && !settings_root_scroll_animating) {
        settings_sync_root_scroll_to_index();
    }

    update_settings_wheel_layout();
    if (settings_view == SETTINGS_VIEW_ROOT && !settings_root_scroll_animating) {
        settings_update_blur_layers();
    }
}

static void settings_enter_view(settings_view_t view)
{
    settings_view_t prev_view = settings_view;
    if (prev_view == SETTINGS_VIEW_ROOT && view != SETTINGS_VIEW_ROOT) {
        settings_root_saved_index = settings_index;
    }

    settings_view = view;
    settings_edit_mode = (view == SETTINGS_VIEW_SENSITIVITY) || (view == SETTINGS_VIEW_BRIGHTNESS);

    if (view == SETTINGS_VIEW_ROOT) {
        int count = settings_get_item_count();
        settings_index = clamp_int(settings_root_saved_index, 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else {
        settings_index = 0;
        if (settings_view_uses_wheel_list(view)) {
            settings_sync_root_scroll_to_index();
        } else {
            lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
            settings_root_scroll_animating = false;
        }
    }

    settings_refresh_view();
}

static void settings_go_back(void)
{
    if (settings_edit_mode) {
        settings_edit_mode = false;
        settings_refresh_view();
        return;
    }

    if (settings_view == SETTINGS_VIEW_ROOT) {
        ui_screen = UI_SCREEN_MAIN;
        show_screen(main_screen);
        set_lock_overlay(ui_locked);
        update_main_ui();
        return;
    }

    if (settings_view == SETTINGS_VIEW_OVERDRIVE_MODE) {
        settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING);
        return;
    }

    settings_enter_view(SETTINGS_VIEW_ROOT);
}

static void settings_adjust_edited_value(int direction)
{
    if (!settings_edit_mode || direction == 0) {
        return;
    }

    if (settings_view == SETTINGS_VIEW_SENSITIVITY && settings_index == 0) {
        speed_sensitivity_clicks = clamp_int(speed_sensitivity_clicks + direction,
                                             SENSITIVITY_MIN_CLICKS,
                                             SENSITIVITY_MAX_CLICKS);
    } else if (settings_view == SETTINGS_VIEW_BRIGHTNESS && settings_index == 0) {
        display_brightness_percent = clamp_int(display_brightness_percent + direction * 10,
                                               DISPLAY_BRIGHTNESS_MIN,
                                               DISPLAY_BRIGHTNESS_MAX);
    } else if (settings_view == SETTINGS_VIEW_FILTER && settings_index == 0) {
        filter_lifetime_hours = clamp_int(filter_lifetime_hours + direction * 10,
                                          FILTER_LIFETIME_MIN_HOURS,
                                          FILTER_LIFETIME_MAX_HOURS);
        if (filter_used_hours > filter_lifetime_hours) {
            filter_used_hours = filter_lifetime_hours;
        }
        refresh_filter_status_from_usage();
        update_main_ui();
    }

    settings_refresh_view();
}

static void settings_activate_item(void)
{
    if (settings_edit_mode) {
        settings_edit_mode = false;
        save_control_settings();
        if (settings_view == SETTINGS_VIEW_SENSITIVITY || settings_view == SETTINGS_VIEW_BRIGHTNESS) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_refresh_view();
        }
        return;
    }

    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (settings_index) {
        case 0: settings_enter_view(SETTINGS_VIEW_SENSITIVITY); return;
        case 1: settings_enter_view(SETTINGS_VIEW_ACCENT); return;
        case 2: settings_enter_view(SETTINGS_VIEW_LANGUAGE); return;
        case 3: settings_enter_view(SETTINGS_VIEW_BRIGHTNESS); return;
        case 4: settings_enter_view(SETTINGS_VIEW_THEME); return;
        case 5: settings_enter_view(SETTINGS_VIEW_ROTATION); return;
        case 6: settings_enter_view(SETTINGS_VIEW_LOCK_MODE); return;
        case 7: settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING); return;
        case 8: settings_enter_view(SETTINGS_VIEW_FILTER); return;
        case 9: settings_enter_view(SETTINGS_VIEW_PURIFICATION_CONFIRM); return;
        default: settings_enter_view(SETTINGS_VIEW_ABOUT); return;
        }

    case SETTINGS_VIEW_SENSITIVITY:
        if (settings_index == 0) {
            settings_edit_mode = true;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_ACCENT:
        if (settings_index < DISPLAY_ACCENT_COUNT) {
            display_accent_index = settings_index;
            apply_runtime_visual_settings();
            update_main_ui();
            save_control_settings();
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_LANGUAGE:
        if (settings_index == 0) {
            current_lang = LANG_EN;
            save_language();
            apply_language();
            settings_refresh_view();
        } else if (settings_index == 1) {
            current_lang = LANG_RU;
            save_language();
            apply_language();
            settings_refresh_view();
        } else if (settings_index == 2) {
            current_lang = LANG_DE;
            save_language();
            apply_language();
            settings_refresh_view();
        } else if (settings_index == 3) {
            current_lang = LANG_FR;
            save_language();
            apply_language();
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_BRIGHTNESS:
        if (settings_index == 0) {
            settings_edit_mode = true;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_THEME:
        if (settings_index == 0) {
            display_theme_dark = true;
            settings_refresh_view();
        } else if (settings_index == 1) {
            display_theme_dark = false;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_ROTATION:
        if (settings_index == 0) {
            display_rotation_deg = 0;
            settings_refresh_view();
        } else if (settings_index == 1) {
            display_rotation_deg = 180;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_LOCK_MODE:
        if (settings_index == 0) {
            allow_child_lock = true;
            save_control_settings();
            settings_refresh_view();
        } else if (settings_index == 1) {
            allow_child_lock = false;
            ui_locked = false;
            set_lock_overlay(false);
            save_control_settings();
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        if (settings_index == 0) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_enter_view(SETTINGS_VIEW_OVERDRIVE_MODE);
        }
        return;

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        if (settings_index == 0) {
            allow_overdrive = true;
            clamp_speed_to_limits();
            apply_fan_pwm(fan_speed_percent);
            update_main_ui();
            save_control_settings();
            settings_refresh_view();
        } else if (settings_index == 1) {
            allow_overdrive = false;
            clamp_speed_to_limits();
            apply_fan_pwm(fan_speed_percent);
            update_main_ui();
            save_control_settings();
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING);
        }
        return;

    case SETTINGS_VIEW_FILTER:
        if (settings_index == 0) {
            settings_edit_mode = true;
            settings_refresh_view();
        } else if (settings_index == 1) {
            filter_used_hours = 0;
            refresh_filter_status_from_usage();
            update_main_ui();
            save_control_settings();
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        if (settings_index == 0) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            start_purification_mode();
        }
        return;

    case SETTINGS_VIEW_ABOUT:
        if (settings_index == 3) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    default:
        settings_go_back();
        return;
    }
}
static void update_main_ui(void)
{
    if (!label_speed) {
        return;
    }
    clamp_speed_to_limits();
    if (fan_speed_percent != last_speed_label_percent) {
        char speed_text[8];
        snprintf(speed_text, sizeof(speed_text), "%d", fan_speed_percent);
        lv_label_set_text(label_speed, speed_text);
        last_speed_label_percent = fan_speed_percent;
    }
    animate_speed_arc_to(fan_speed_percent);
    if (label_overdrive_warning) {
        if (fan_speed_percent > FAN_SPEED_NORMAL_MAX_PERCENT) {
            lv_obj_clear_flag(label_overdrive_warning, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(label_overdrive_warning, LV_OBJ_FLAG_HIDDEN);
        }
    }
    update_status_row();
}

static void apply_language(void)
{
    if (label_speed_caption) {
        lv_label_set_text(label_speed_caption, ui_strings[current_lang].fan_speed);
    }
    if (label_language_title) {
        lv_label_set_text(label_language_title, ui_strings[current_lang].language);
    }
    if (label_owner_title) {
        lv_label_set_text(label_owner_title, ui_strings[current_lang].owner_name);
    }
    if (label_sensitivity_title) {
        lv_label_set_text(label_sensitivity_title, "Sensitivity");
    }
    if (label_overdrive_title) {
        lv_label_set_text(label_overdrive_title, "Overdrive");
    }
    if (label_purification_title) {
        lv_label_set_text(label_purification_title, is_purifying ? "Purifying" : "Purification");
    }
    if (label_purification_state) {
        lv_label_set_text(label_purification_state, is_purifying ? "RUNNING" : "STOPPED");
    }
    if (lock_overlay) {
        lv_label_set_text(lock_overlay, ui_strings[current_lang].locked);
    }
    settings_refresh_view();
    update_status_row();
}

static void set_lock_overlay(bool enabled)
{
    (void)enabled;
    if (lock_overlay) {
        lv_obj_add_flag(lock_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    update_status_row();
}

static void update_settings_selection(void)
{
    settings_refresh_view();
}

static void set_boot_theme_blend(lv_opa_t dark_blend_opa, int active_count, int partial_index)
{
    (void)active_count;
    (void)partial_index;
    boot_theme_dark_blend_opa = dark_blend_opa;

    lv_color_t bg_color = lv_color_mix(lv_color_hex(UI_BG_HEX),
                                       lv_color_hex(UI_BOOT_BG_LIGHT_HEX),
                                       dark_blend_opa);
    if (boot_screen) {
        lv_obj_set_style_bg_color(boot_screen, bg_color, 0);
    }

    if (boot_logo) {
        lv_opa_t recolor_opa = (lv_opa_t)(LV_OPA_COVER - dark_blend_opa);
#if LVGL_VERSION_MAJOR >= 9
        lv_obj_set_style_image_recolor(boot_logo, lv_color_black(), 0);
        lv_obj_set_style_image_recolor_opa(boot_logo, recolor_opa, 0);
#else
        lv_obj_set_style_img_recolor(boot_logo, lv_color_black(), 0);
        lv_obj_set_style_img_recolor_opa(boot_logo, recolor_opa, 0);
#endif
    }
}

static void set_boot_theme_mode(bool dark_mode, int active_count, int partial_index)
{
    set_boot_theme_blend(dark_mode ? LV_OPA_COVER : LV_OPA_TRANSP, active_count, partial_index);
}

static void boot_theme_blend_anim_cb(void *var, int32_t value)
{
    (void)var;
    if (value < LV_OPA_TRANSP) {
        value = LV_OPA_TRANSP;
    }
    if (value > LV_OPA_COVER) {
        value = LV_OPA_COVER;
    }
    set_boot_theme_blend((lv_opa_t)value, 0, -1);
}

static void boot_fill_anim_ready_cb(lv_anim_t *a)
{
    (void)a;
    if (!UI_BOOT_ENABLE_DARK_THEME_TRANSITION) {
        return;
    }
    lv_anim_t theme_anim;
    lv_anim_init(&theme_anim);
    lv_anim_set_var(&theme_anim, NULL);
    lv_anim_set_exec_cb(&theme_anim, boot_theme_blend_anim_cb);
    lv_anim_set_values(&theme_anim, boot_theme_dark_blend_opa, LV_OPA_COVER);
    lv_anim_set_time(&theme_anim, UI_BOOT_THEME_TRANSITION_TIME_MS);
    lv_anim_set_path_cb(&theme_anim, lv_anim_path_ease_in_out);
    lv_anim_start(&theme_anim);
}

static void set_obj_bg_opa_anim(void *obj, int32_t value)
{
    if (!obj) {
        return;
    }
    if (value < LV_OPA_TRANSP) {
        value = LV_OPA_TRANSP;
    }
    if (value > LV_OPA_COVER) {
        value = LV_OPA_COVER;
    }
    lv_obj_set_style_bg_opa((lv_obj_t *)obj, (lv_opa_t)value, 0);
}

static void set_obj_text_opa_anim(void *obj, int32_t value)
{
    if (!obj) {
        return;
    }
    if (value < 0) {
        value = 0;
    }
    if (value > 255) {
        value = 255;
    }
    lv_color_t from = lv_color_mix(lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX),
                                   lv_color_hex(UI_BOOT_SUBTITLE_COLOR_LIGHT_FROM_HEX),
                                   boot_theme_dark_blend_opa);
    lv_color_t to = lv_color_mix(lv_color_hex(UI_BOOT_SUBTITLE_COLOR_TO_HEX),
                                 lv_color_hex(UI_BOOT_SUBTITLE_COLOR_LIGHT_TO_HEX),
                                 boot_theme_dark_blend_opa);
    lv_color_t final_color = lv_color_mix(from, to, (lv_opa_t)value);
    lv_obj_set_style_text_color((lv_obj_t *)obj, final_color, 0);
}

static lv_color_t get_filter_status_color(void)
{
    if (filter_status == FILTER_STATUS_CRITICAL) {
        return lv_color_hex(UI_FILTER_CRITICAL_HEX);
    }
    if (filter_status == FILTER_STATUS_WARNING) {
        return lv_color_hex(UI_FILTER_WARNING_HEX);
    }
    return lv_color_hex(UI_FILTER_GOOD_HEX);
}

static lv_color_t get_lock_icon_color(void)
{
    return ui_locked ? lv_color_hex(ui_accent_color_hex) : lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX);
}

static const char *get_filter_status_text(void)
{
    if (filter_status == FILTER_STATUS_CRITICAL) {
        return "FILTER REPLACE";
    }
    if (filter_status == FILTER_STATUS_WARNING) {
        return "FILTER LOW";
    }
    return "FILTER GOOD";
}

static void update_status_row(void)
{
    static int cached_locked = -1;
    static int cached_filter = -1;
    static uint32_t cached_accent = 0;
    int lock_state = ui_locked ? 1 : 0;
    int filter_state = (int)filter_status;

    lv_color_t lock_color = get_lock_icon_color();
    lv_color_t filt_color = get_filter_status_color();

    if (lock_state != cached_locked || cached_accent != ui_accent_color_hex) {
        if (label_lock_icon) {
            lv_label_set_text(label_lock_icon, ui_locked ? UI_ICON_LUCIDE_LOCK : UI_ICON_LUCIDE_LOCK_OPEN);
            lv_obj_set_style_text_color(label_lock_icon, lock_color, 0);
        }
        if (label_lock_status) {
            lv_label_set_text(label_lock_status, ui_locked ? "LOCKED" : "UNLOCK");
            lv_obj_set_style_text_color(label_lock_status, lock_color, 0);
        }
        if (main_ring_active_count >= 0) {
            update_segment_ring(fan_speed_percent, false);
        }
        cached_locked = lock_state;
        cached_accent = ui_accent_color_hex;
    }

    if (filter_state != cached_filter) {
        if (label_filter_icon) {
            lv_label_set_text(label_filter_icon, (filter_status == FILTER_STATUS_GOOD) ? UI_ICON_LUCIDE_SHIELD_CHECK : UI_ICON_LUCIDE_SHIELD_ALERT);
            lv_obj_set_style_text_color(label_filter_icon, filt_color, 0);
        }
        if (label_filter_status) {
            lv_label_set_text(label_filter_status, "FILTER");
            lv_obj_set_style_text_color(label_filter_status, filt_color, 0);
        }
        cached_filter = filter_state;
    }
}

static void build_segment_ring(lv_obj_t *parent)
{
    const float two_pi = 6.28318530718f;
    const lv_coord_t center_x = (EXAMPLE_LCD_H_RES / 2) + UI_MAIN_CENTER_OFFSET_X;
    const lv_coord_t center_y = (EXAMPLE_LCD_V_RES / 2) + UI_MAIN_CENTER_OFFSET_Y;

    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lv_obj_t *seg = lv_obj_create(parent);
        ring_segments[i] = seg;
        lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(seg, UI_SEGMENT_W, UI_SEGMENT_H);
        lv_obj_set_style_radius(seg, 4, 0);
        lv_obj_set_style_border_width(seg, 0, 0);
        lv_obj_set_style_bg_color(seg, lv_color_hex(UI_BOOT_RING_INACTIVE_HEX), 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_40, 0);

        float angle = ((float)i / (float)UI_SEGMENT_COUNT) * two_pi;
        float angle_deg = ((float)i / (float)UI_SEGMENT_COUNT) * 360.0f;
        lv_coord_t x = center_x + (lv_coord_t)(sinf(angle) * UI_BOOT_RING_RADIUS) - (UI_SEGMENT_W / 2);
        lv_coord_t y = center_y - (lv_coord_t)(cosf(angle) * UI_BOOT_RING_RADIUS) - (UI_SEGMENT_H / 2);
        lv_obj_set_pos(seg, x, y);
        lv_obj_set_style_transform_angle(seg, (int16_t)(angle_deg * 10.0f), 0);
    }
}

static void update_segment_ring(int target_percent, bool increasing)
{
    (void)increasing;
    lv_color_t active_color = lv_color_hex(ui_accent_color_hex);
    lv_color_t inactive_color = get_lock_icon_color();
    if (target_percent < FAN_SPEED_MIN_PERCENT) {
        target_percent = FAN_SPEED_MIN_PERCENT;
    }
    if (target_percent > FAN_SPEED_NORMAL_MAX_PERCENT) {
        target_percent = FAN_SPEED_NORMAL_MAX_PERCENT;
    }

    int32_t scaled = target_percent * UI_SEGMENT_COUNT;
    int active_count = (int)(scaled / FAN_SPEED_NORMAL_MAX_PERCENT);
    int remainder = (int)(scaled % FAN_SPEED_NORMAL_MAX_PERCENT);
    int boundary_index = (remainder > 0 && active_count < UI_SEGMENT_COUNT) ? active_count : -1;

    if (active_count < 0) {
        active_count = 0;
    }
    if (active_count > UI_SEGMENT_COUNT) {
        active_count = UI_SEGMENT_COUNT;
    }

    if (main_ring_active_count < 0) {
        for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
            lv_obj_t *seg = ring_segments[i];
            if (!seg) {
                continue;
            }
            if (i < active_count) {
                lv_obj_set_style_bg_color(seg, active_color, 0);
                lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
            } else {
                lv_obj_set_style_bg_color(seg, inactive_color, 0);
                lv_obj_set_style_bg_opa(seg, LV_OPA_40, 0);
            }
        }
        main_ring_active_count = active_count;
    } else {
        if (active_count != main_ring_active_count) {
            if (active_count > main_ring_active_count) {
                for (int i = main_ring_active_count; i < active_count; ++i) {
                    if (i < 0 || i >= UI_SEGMENT_COUNT) {
                        continue;
                    }
                    lv_obj_t *seg = ring_segments[i];
                    if (!seg) {
                        continue;
                    }
                    lv_obj_set_style_bg_color(seg, active_color, 0);
                    lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
                }
            } else {
                for (int i = active_count; i < main_ring_active_count; ++i) {
                    if (i < 0 || i >= UI_SEGMENT_COUNT) {
                        continue;
                    }
                    lv_obj_t *seg = ring_segments[i];
                    if (!seg) {
                        continue;
                    }
                    lv_obj_set_style_bg_color(seg, inactive_color, 0);
                    lv_obj_set_style_bg_opa(seg, LV_OPA_40, 0);
                }
            }
            main_ring_active_count = active_count;
        }
    }

    if (main_ring_boundary_index != boundary_index) {
        if (main_ring_boundary_index >= 0 && main_ring_boundary_index < UI_SEGMENT_COUNT) {
            lv_obj_t *prev = ring_segments[main_ring_boundary_index];
            if (prev) {
                lv_anim_delete(prev, set_obj_bg_opa_anim);
                if (main_ring_boundary_index < active_count) {
                    lv_obj_set_style_bg_color(prev, active_color, 0);
                    lv_obj_set_style_bg_opa(prev, LV_OPA_COVER, 0);
                } else {
                    lv_obj_set_style_bg_color(prev, inactive_color, 0);
                    lv_obj_set_style_bg_opa(prev, LV_OPA_40, 0);
                }
            }
        }
        main_ring_boundary_index = boundary_index;
        if (main_ring_boundary_index >= 0 && main_ring_boundary_index < UI_SEGMENT_COUNT) {
            lv_obj_t *seg = ring_segments[main_ring_boundary_index];
            if (seg) {
                lv_obj_set_style_bg_color(seg, active_color, 0);
                lv_obj_set_style_bg_opa(seg, LV_OPA_70, 0);

                lv_anim_t blink;
                lv_anim_init(&blink);
                lv_anim_set_var(&blink, seg);
                lv_anim_set_exec_cb(&blink, set_obj_bg_opa_anim);
                lv_anim_set_values(&blink, LV_OPA_50, LV_OPA_COVER);
                lv_anim_set_time(&blink, 850);
                lv_anim_set_playback_time(&blink, 850);
                lv_anim_set_repeat_count(&blink, LV_ANIM_REPEAT_INFINITE);
                lv_anim_set_path_cb(&blink, lv_anim_path_ease_in_out);
                lv_anim_start(&blink);
            }
        }
    }
}

static void update_boot_screen_visual(int32_t progress)
{
    if (progress < 0) {
        progress = 0;
    }
    if (progress > 1000) {
        progress = 1000;
    }
    boot_progress_percent = (progress + 5) / 10;
    lv_color_t inactive_color = lv_color_mix(lv_color_hex(UI_BOOT_RING_INACTIVE_HEX),
                                             lv_color_hex(UI_BOOT_RING_INACTIVE_LIGHT_HEX),
                                             boot_theme_dark_blend_opa);

    int32_t scaled_progress = progress * UI_SEGMENT_COUNT;
    int active_count = (int)(scaled_progress / 1000);
    if (active_count < 0) {
        active_count = 0;
    }
    if (active_count > UI_SEGMENT_COUNT) {
        active_count = UI_SEGMENT_COUNT;
    }
    int32_t partial_milli = scaled_progress % 1000;
    int partial_index = (progress >= 1000 || active_count >= UI_SEGMENT_COUNT) ? -1 : active_count;

    if (boot_last_active_count < 0) {
        boot_last_active_count = 0;
    }

    if (active_count > boot_last_active_count) {
        for (int i = boot_last_active_count; i < active_count; ++i) {
            lv_obj_t *seg = boot_ring_segments[i];
            if (!seg) {
                continue;
            }
            lv_obj_set_style_bg_color(seg, lv_color_hex(UI_ACCENT_COLOR_HEX), 0);
            lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
        }
    } else if (active_count < boot_last_active_count) {
        for (int i = active_count; i < boot_last_active_count; ++i) {
            lv_obj_t *seg = boot_ring_segments[i];
            if (!seg) {
                continue;
            }
            lv_obj_set_style_bg_color(seg, inactive_color, 0);
            lv_obj_set_style_bg_opa(seg, LV_OPA_40, 0);
        }
    }

    if (boot_last_partial_index >= 0 &&
        boot_last_partial_index < UI_SEGMENT_COUNT &&
        boot_last_partial_index != partial_index &&
        boot_last_partial_index >= active_count) {
        lv_obj_t *old_partial = boot_ring_segments[boot_last_partial_index];
        if (old_partial) {
            lv_obj_set_style_bg_color(old_partial, inactive_color, 0);
            lv_obj_set_style_bg_opa(old_partial, LV_OPA_40, 0);
        }
    }

    if (partial_index >= 0 && partial_index < UI_SEGMENT_COUNT) {
        lv_obj_t *seg = boot_ring_segments[partial_index];
        if (seg) {
            int32_t partial_mix = (partial_milli * LV_OPA_COVER) / 1000;
            if (partial_mix < 0) {
                partial_mix = 0;
            }
            if (partial_mix > LV_OPA_COVER) {
                partial_mix = LV_OPA_COVER;
            }
            lv_color_t partial_color = lv_color_mix(lv_color_hex(UI_ACCENT_COLOR_HEX),
                                                    inactive_color,
                                                    (lv_opa_t)partial_mix);
            lv_obj_set_style_bg_color(seg, partial_color, 0);
            int32_t partial_opa = LV_OPA_30 + (partial_milli * (LV_OPA_COVER - LV_OPA_30)) / 1000;
            if (partial_opa < LV_OPA_40) {
                partial_opa = LV_OPA_40;
            }
            if (partial_opa > LV_OPA_COVER) {
                partial_opa = LV_OPA_COVER;
            }
            lv_obj_set_style_bg_opa(seg, (lv_opa_t)partial_opa, 0);
        }
    }

    boot_last_active_count = active_count;
    boot_last_partial_index = partial_index;

    if (boot_progress_track && boot_progress_fill) {
        lv_coord_t track_w = lv_obj_get_width(boot_progress_track);
        lv_coord_t fill_w = (lv_coord_t)((progress * track_w) / 1000);
        if (fill_w < 0) {
            fill_w = 0;
        }
        if (fill_w != boot_last_fill_width) {
            lv_obj_set_width(boot_progress_fill, fill_w);
            boot_last_fill_width = fill_w;
        }
    }

}

static void boot_progress_anim_cb(void *var, int32_t value)
{
    (void)var;
    update_boot_screen_visual(value);
}

static void start_boot_animation(void)
{
    lv_anim_delete(NULL, boot_theme_blend_anim_cb);
    boot_theme_dark_blend_opa = LV_OPA_COVER;
    set_boot_theme_mode(true, 0, -1);
    update_boot_screen_visual(0);
    lv_anim_t boot_anim;
    lv_anim_init(&boot_anim);
    lv_anim_set_var(&boot_anim, NULL);
    lv_anim_set_exec_cb(&boot_anim, boot_progress_anim_cb);
    lv_anim_set_values(&boot_anim, 0, 1000);
    lv_anim_set_time(&boot_anim, (uint32_t)(UI_BOOT_ANIM_TIME_US / 1000ULL));
    lv_anim_set_path_cb(&boot_anim, lv_anim_path_linear);
    lv_anim_set_ready_cb(&boot_anim, boot_fill_anim_ready_cb);
    lv_anim_start(&boot_anim);
}

static void animate_speed_arc_to(int target_percent)
{
    if (target_percent < FAN_SPEED_MIN_PERCENT) {
        target_percent = FAN_SPEED_MIN_PERCENT;
    }
    if (target_percent > FAN_SPEED_NORMAL_MAX_PERCENT) {
        target_percent = FAN_SPEED_NORMAL_MAX_PERCENT;
    }

    if (target_percent == fan_speed_arc_displayed) {
        update_segment_ring(target_percent, false);
        return;
    }

    bool increasing = target_percent > fan_speed_arc_displayed;
    update_segment_ring(target_percent, increasing);
    fan_speed_arc_displayed = target_percent;
}

static void apply_dark_screen_style(lv_obj_t *screen)
{
    if (!screen) {
        return;
    }
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_BG_HEX), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_text_color(screen, lv_color_white(), 0);
}

static void set_backlight_enabled(bool enabled)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
#if EXAMPLE_LCD_BK_PIN_IS_POWER_ENABLE
    (void)enabled;
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
#else
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, enabled ? EXAMPLE_LCD_BK_LIGHT_ON_LEVEL : EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);
#endif
#else
    (void)enabled;
#endif
}

static lv_obj_t *get_screen_by_id(ui_screen_t screen)
{
    switch (screen) {
    case UI_SCREEN_BOOT:
        return boot_screen;
    case UI_SCREEN_MAIN:
        return main_screen;
    case UI_SCREEN_SETTINGS:
        return settings_screen;
    case UI_SCREEN_LANGUAGE:
        return language_screen;
    case UI_SCREEN_OWNER:
        return owner_screen;
    case UI_SCREEN_SENSITIVITY:
        return sensitivity_screen;
    case UI_SCREEN_OVERDRIVE:
        return overdrive_screen;
    case UI_SCREEN_PURIFICATION:
        return purification_screen;
    case UI_SCREEN_SLEEP:
        return sleep_screen;
    default:
        return main_screen;
    }
}

static void sleep_timer_cb(void *arg)
{
    (void)arg;
    enter_sleep_mode();
}

static void reset_sleep_timer(void)
{
    if (ui_sleeping) {
        return;
    }
    if (!sleep_timer) {
        esp_timer_create_args_t timer_args = {
            .callback = &sleep_timer_cb,
            .name = "sleep_timer",
        };
        ESP_ERROR_CHECK(esp_timer_create(&timer_args, &sleep_timer));
    }
    (void)esp_timer_stop(sleep_timer);
    ESP_ERROR_CHECK(esp_timer_start_once(sleep_timer, SLEEP_TIMEOUT_US));
}

static void enter_sleep_mode_locked(void)
{
    if (ui_sleeping) {
        return;
    }
    ui_sleeping = true;
    ui_screen_before_sleep = ui_screen;

    ui_screen = UI_SCREEN_SLEEP;
    show_screen(sleep_screen);
}

static void enter_sleep_mode(void)
{
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "LVGL lock timeout in enter_sleep_mode");
        return;
    }
    enter_sleep_mode_locked();
    lvgl_port_unlock();
    set_backlight_enabled(false);
    ESP_LOGI(TAG, "Sleep mode enabled");
}

static void wake_from_sleep_locked(void)
{
    if (!ui_sleeping) {
        return;
    }

    ui_sleeping = false;
    ui_screen = ui_screen_before_sleep;
    if (ui_screen == UI_SCREEN_SLEEP) {
        ui_screen = UI_SCREEN_MAIN;
    }
    show_screen(get_screen_by_id(ui_screen));
    if (ui_screen == UI_SCREEN_MAIN) {
        set_lock_overlay(ui_locked);
        update_main_ui();
    } else if (ui_screen == UI_SCREEN_SETTINGS) {
        apply_language();
        update_settings_selection();
    } else if (ui_screen == UI_SCREEN_PURIFICATION) {
        apply_language();
    }
}

static void wake_from_sleep(void)
{
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "LVGL lock timeout in wake_from_sleep");
        return;
    }
    wake_from_sleep_locked();
    lvgl_port_unlock();
    set_backlight_enabled(true);
    reset_sleep_timer();
    ESP_LOGI(TAG, "Sleep mode disabled");
}

static void show_screen(lv_obj_t *screen)
{
    if (screen) {
        lv_scr_load(screen);
    }
}

static void boot_transition_task(void *arg)
{
    (void)arg;
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGE(TAG, "Failed to lock LVGL in boot transition");
        vTaskDelete(NULL);
        return;
    }

#if UI_BOOT_LOOP_TEST
    ui_screen = UI_SCREEN_BOOT;
    show_screen(boot_screen);
    start_boot_animation();
    lvgl_port_unlock();
    ESP_ERROR_CHECK(esp_timer_start_once(boot_timer, UI_BOOT_TOTAL_TIME_US));
#else
    ui_screen = UI_SCREEN_MAIN;
    show_screen(main_screen);
    set_lock_overlay(ui_locked);
    update_main_ui();
    lvgl_port_unlock();
#endif

    vTaskDelete(NULL);
}

static void boot_timer_cb(void *arg)
{
    (void)arg;
    BaseType_t created = xTaskCreate(boot_transition_task, "boot_sw", 4096, NULL, 5, NULL);
    if (created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create boot transition task");
    }
}

esp_err_t app_touch_init(void)
{
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
    /* Initilize I2C */
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = EXAMPLE_PIN_NUM_TOUCH_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = EXAMPLE_PIN_NUM_TOUCH_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = EXAMPLE_TOUCH_I2C_CLK_HZ};
    ESP_RETURN_ON_ERROR(i2c_param_config(EXAMPLE_TOUCH_I2C_NUM, &i2c_conf), TAG, "I2C configuration failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(EXAMPLE_TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0), TAG, "I2C initialization failed");

    /* Initialize touch HW */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = EXAMPLE_PIN_NUM_TOUCH_RST, // Shared with LCD reset
        .int_gpio_num = EXAMPLE_PIN_NUM_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            // 屏幕方向一般在这里改
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST820_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)EXAMPLE_TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle), TAG, "");
    return esp_lcd_touch_new_i2c_cst820(tp_io_handle, &tp_cfg, &touch_handle);
#else
    touch_handle = NULL;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

void rounder_event_cb(lv_event_t *e)
{
    lv_area_t *area = lv_event_get_invalidated_area(e);
    if (!area) {
        return;
    }

    /* Keep invalidation area inside display bounds first.
     * Using unsigned intermediates here can overflow when LVGL sends
     * temporary negative coordinates (e.g. shadows/transforms). */
    if (area->x1 < 0) {
        area->x1 = 0;
    }
    if (area->y1 < 0) {
        area->y1 = 0;
    }
    if (area->x1 >= EXAMPLE_LCD_H_RES) {
        area->x1 = EXAMPLE_LCD_H_RES - 1;
    }
    if (area->y1 >= EXAMPLE_LCD_V_RES) {
        area->y1 = EXAMPLE_LCD_V_RES - 1;
    }
    if (area->x2 >= EXAMPLE_LCD_H_RES) {
        area->x2 = EXAMPLE_LCD_H_RES - 1;
    }
    if (area->y2 >= EXAMPLE_LCD_V_RES) {
        area->y2 = EXAMPLE_LCD_V_RES - 1;
    }
    if (area->x2 < 0) {
        area->x2 = 0;
    }
    if (area->y2 < 0) {
        area->y2 = 0;
    }

    // Round start down to even pixel, end up to odd pixel.
    area->x1 &= ~1;
    area->y1 &= ~1;
    area->x2 |= 1;
    area->y2 |= 1;

    if (area->x2 >= EXAMPLE_LCD_H_RES) {
        area->x2 = EXAMPLE_LCD_H_RES - 1;
    }
    if (area->y2 >= EXAMPLE_LCD_V_RES) {
        area->y2 = EXAMPLE_LCD_V_RES - 1;
    }
    if (area->x2 < area->x1) {
        area->x2 = area->x1;
    }
    if (area->y2 < area->y1) {
        area->y2 = area->y1;
    }
}

esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 12288,      /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT, // EXAMPLE_LCD_DRAW_BUFF_HEIGHT
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,  // false,  屏幕方向一般在这里改
            .mirror_x = false, // true,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
#if UI_ENABLE_INVALIDATE_ROUNDER
    lv_display_add_event_cb(lvgl_disp, rounder_event_cb, LV_EVENT_INVALIDATE_AREA, NULL);
#endif
    lv_display_add_event_cb(lvgl_disp, fps_display_refr_event_cb, LV_EVENT_REFR_START, NULL);

    /* Add touch input (for selected screen) */
    if (touch_handle) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lvgl_disp,
            .handle = touch_handle,
        };
        lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);
    } else {
        ESP_LOGW(TAG, "Touch is disabled because initialization failed");
    }

    return ESP_OK;
}

static void create_boot_screen(void)
{
    boot_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(boot_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(boot_screen);
    lv_obj_set_style_bg_color(boot_screen, lv_color_hex(UI_BG_HEX), 0);

    const float two_pi = 6.28318530718f;
    const lv_coord_t center_x = (EXAMPLE_LCD_H_RES / 2) + UI_BOOT_CENTER_OFFSET_X;
    const lv_coord_t center_y = (EXAMPLE_LCD_V_RES / 2) + UI_BOOT_CENTER_OFFSET_Y;
    const lv_coord_t ring_radius = UI_BOOT_RING_RADIUS;
    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lv_obj_t *seg = lv_obj_create(boot_screen);
        boot_ring_segments[i] = seg;
        lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(seg, UI_SEGMENT_W, UI_SEGMENT_H);
        lv_obj_set_style_radius(seg, 4, 0);
        lv_obj_set_style_border_width(seg, 0, 0);
        lv_obj_set_style_bg_color(seg, lv_color_hex(UI_BOOT_RING_INACTIVE_HEX), 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_40, 0);

        float angle = ((float)i / (float)UI_SEGMENT_COUNT) * two_pi;
        float angle_deg = ((float)i / (float)UI_SEGMENT_COUNT) * 360.0f;
        lv_coord_t x = center_x + (lv_coord_t)(sinf(angle) * ring_radius) - (UI_SEGMENT_W / 2);
        lv_coord_t y = center_y - (lv_coord_t)(cosf(angle) * ring_radius) - (UI_SEGMENT_H / 2);
        lv_obj_set_pos(seg, x, y);
        lv_obj_set_style_transform_angle(seg, (int16_t)(angle_deg * 10.0f), 0);
    }

    boot_last_active_count = 0;
    boot_last_partial_index = -1;
    boot_last_fill_width = -1;
    boot_theme_dark_blend_opa = LV_OPA_TRANSP;

#if LVGL_VERSION_MAJOR >= 9
    boot_logo = lv_image_create(boot_screen);
    lv_image_set_src(boot_logo, &belom_logo_white);
    int32_t logo_scale = (UI_BOOT_LOGO_TARGET_WIDTH * 256) / (int32_t)belom_logo_white.header.w;
    if (logo_scale < 1) {
        logo_scale = 1;
    } else if (logo_scale > 1024) {
        logo_scale = 1024;
    }
    lv_image_set_scale(boot_logo, (uint32_t)logo_scale);
    lv_obj_align(boot_logo, LV_ALIGN_CENTER, UI_BOOT_CENTER_OFFSET_X, -16 + UI_BOOT_CENTER_OFFSET_Y);
#else
    boot_logo = lv_img_create(boot_screen);
    lv_img_set_src(boot_logo, &belom_logo_white);
    int32_t logo_zoom = (UI_BOOT_LOGO_TARGET_WIDTH * 256) / (int32_t)belom_logo_white.header.w;
    if (logo_zoom < 1) {
        logo_zoom = 1;
    } else if (logo_zoom > 1024) {
        logo_zoom = 1024;
    }
    lv_img_set_zoom(boot_logo, (uint16_t)logo_zoom);
    lv_obj_align(boot_logo, LV_ALIGN_CENTER, UI_BOOT_CENTER_OFFSET_X, -16 + UI_BOOT_CENTER_OFFSET_Y);
#endif
    set_boot_theme_mode(true, 0, -1);

    boot_subtitle = lv_label_create(boot_screen);
    lv_label_set_text(boot_subtitle, "BELOM  A2");
    lv_obj_set_style_text_font(boot_subtitle, &lv_font_inter_medium_20_caps, 0);
    lv_obj_set_style_text_color(boot_subtitle, lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX), 0);
    lv_obj_align(boot_subtitle, LV_ALIGN_CENTER, UI_BOOT_CENTER_OFFSET_X, 96 + UI_BOOT_CENTER_OFFSET_Y);

    lv_anim_t subtitle_pulse;
    lv_anim_init(&subtitle_pulse);
    lv_anim_set_var(&subtitle_pulse, boot_subtitle);
    lv_anim_set_exec_cb(&subtitle_pulse, set_obj_text_opa_anim);
    lv_anim_set_values(&subtitle_pulse, 255, 0);
    lv_anim_set_time(&subtitle_pulse, 700);
    lv_anim_set_playback_time(&subtitle_pulse, 700);
    lv_anim_set_repeat_count(&subtitle_pulse, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&subtitle_pulse);

    boot_progress_track = lv_obj_create(boot_screen);
    lv_obj_remove_style_all(boot_progress_track);
    lv_obj_clear_flag(boot_progress_track, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(boot_progress_track, 72, 3);
    lv_obj_align(boot_progress_track, LV_ALIGN_CENTER, UI_BOOT_CENTER_OFFSET_X, 114 + UI_BOOT_CENTER_OFFSET_Y);
    lv_obj_set_style_radius(boot_progress_track, 1, 0);
    lv_obj_set_style_bg_color(boot_progress_track, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(boot_progress_track, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(boot_progress_track, 0, 0);
    lv_obj_set_style_pad_all(boot_progress_track, 0, 0);
    lv_obj_set_style_clip_corner(boot_progress_track, true, 0);

    boot_progress_fill = lv_obj_create(boot_progress_track);
    lv_obj_remove_style_all(boot_progress_fill);
    lv_obj_clear_flag(boot_progress_fill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(boot_progress_fill, 0, 3);
    lv_obj_set_pos(boot_progress_fill, 0, 0);
    lv_obj_set_style_radius(boot_progress_fill, 1, 0);
    lv_obj_set_style_bg_color(boot_progress_fill, lv_color_hex(UI_ACCENT_COLOR_HEX), 0);
    lv_obj_set_style_bg_opa(boot_progress_fill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(boot_progress_fill, 0, 0);
    lv_obj_set_style_pad_all(boot_progress_fill, 0, 0);

    update_boot_screen_visual(0);
}

static void create_main_screen(void)
{
    main_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(main_screen);

    /* Subtle center glow on top of pure black background */
    lv_obj_t *bg_glow_outer = lv_obj_create(main_screen);
    lv_obj_remove_style_all(bg_glow_outer);
    lv_obj_clear_flag(bg_glow_outer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(bg_glow_outer, 520, 520);
    lv_obj_align(bg_glow_outer, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y);
    lv_obj_set_style_radius(bg_glow_outer, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bg_glow_outer, lv_color_hex(0x101216), 0);
    lv_obj_set_style_bg_opa(bg_glow_outer, (lv_opa_t)20, 0);
    lv_obj_set_style_border_width(bg_glow_outer, 0, 0);

    lv_obj_t *bg_glow_inner = lv_obj_create(main_screen);
    lv_obj_remove_style_all(bg_glow_inner);
    lv_obj_clear_flag(bg_glow_inner, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(bg_glow_inner, 360, 360);
    lv_obj_align(bg_glow_inner, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y);
    lv_obj_set_style_radius(bg_glow_inner, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bg_glow_inner, lv_color_hex(0x1A1D22), 0);
    lv_obj_set_style_bg_opa(bg_glow_inner, (lv_opa_t)30, 0);
    lv_obj_set_style_border_width(bg_glow_inner, 0, 0);

    lv_obj_t *bg_glow_core = lv_obj_create(main_screen);
    lv_obj_remove_style_all(bg_glow_core);
    lv_obj_clear_flag(bg_glow_core, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(bg_glow_core, 250, 250);
    lv_obj_align(bg_glow_core, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y);
    lv_obj_set_style_radius(bg_glow_core, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bg_glow_core, lv_color_hex(0x252A32), 0);
    lv_obj_set_style_bg_opa(bg_glow_core, (lv_opa_t)24, 0);
    lv_obj_set_style_border_width(bg_glow_core, 0, 0);

    lv_obj_t *center_disc = lv_obj_create(main_screen);
    lv_obj_clear_flag(center_disc, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(center_disc, UI_MAIN_DIAL_SIZE, UI_MAIN_DIAL_SIZE);
    lv_obj_align(center_disc, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y);
    lv_obj_set_style_radius(center_disc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(center_disc, 0, 0);
    lv_obj_set_style_bg_color(center_disc, lv_color_hex(UI_SURFACE_HEX), 0);
    lv_obj_set_style_bg_opa(center_disc, LV_OPA_COVER, 0);

    build_segment_ring(main_screen);

    label_speed = lv_label_create(main_screen);
    // Bitmap Inter font is cheaper than runtime TTF rendering for frequent speed updates.
    lv_obj_set_style_text_font(label_speed, &lv_font_inter_medium_145_num, 0);
    lv_obj_set_style_text_letter_space(label_speed, -2, 0);
    lv_obj_set_style_text_color(label_speed, lv_color_white(), 0);
    lv_obj_set_style_text_align(label_speed, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_speed, EXAMPLE_LCD_H_RES);
    lv_obj_align(label_speed, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y - 10);
    lv_obj_move_foreground(label_speed);

    lv_obj_t *status_row = lv_obj_create(main_screen);
    lv_obj_remove_style_all(status_row);
    lv_obj_clear_flag(status_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(status_row, 300, 56);
    lv_obj_align(status_row, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y + UI_MAIN_STATUS_ROW_Y);
    lv_obj_set_flex_flow(status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(status_row, 0, 0);
    lv_obj_set_style_pad_column(status_row, 18, 0);

    label_lock_icon = lv_label_create(status_row);
    lv_obj_set_style_text_font(label_lock_icon, ui_font_lucide_20(), 0);
    lv_obj_set_style_transform_zoom(label_lock_icon, 332, 0); /* +30% */
    lv_obj_set_style_text_align(label_lock_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_lock_icon, 56);
    lv_label_set_text(label_lock_icon, UI_ICON_LUCIDE_LOCK_OPEN);

    label_lock_status = NULL;

    label_filter_icon = lv_label_create(status_row);
    lv_obj_set_style_text_font(label_filter_icon, ui_font_lucide_20(), 0);
    lv_obj_set_style_transform_zoom(label_filter_icon, 332, 0); /* +30% */
    lv_obj_set_style_text_align(label_filter_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_filter_icon, 56);
    lv_label_set_text(label_filter_icon, UI_ICON_LUCIDE_SHIELD_CHECK);

    label_filter_status = lv_label_create(status_row);
    lv_obj_set_style_text_font(label_filter_status, &lv_font_inter_medium_20_caps, 0);
    lv_obj_set_style_transform_zoom(label_filter_status, 282, 0); /* +10% */
    lv_obj_set_style_text_letter_space(label_filter_status, 1, 0);
    lv_obj_set_style_text_align(label_filter_status, LV_TEXT_ALIGN_CENTER, 0);

    status_divider = NULL;

    lock_overlay = NULL;

    label_overdrive_warning = lv_label_create(main_screen);
    lv_obj_set_style_text_font(label_overdrive_warning, &lv_font_inter_medium_12_caps, 0);
    lv_obj_set_style_text_color(label_overdrive_warning, lv_color_hex(UI_FILTER_CRITICAL_HEX), 0);
    lv_label_set_text(label_overdrive_warning, "OVERDRIVE MODE");
    lv_obj_align(label_overdrive_warning, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y + UI_MAIN_OVERDRIVE_Y);
    lv_obj_add_flag(label_overdrive_warning, LV_OBJ_FLAG_HIDDEN);

    main_ring_active_count = -1;
    main_ring_boundary_index = -1;
    fan_speed_arc_displayed = fan_speed_percent;
    update_segment_ring(fan_speed_percent, false);
    update_status_row();

    label_speed_caption = NULL;
}

static void create_settings_screen(void)
{
    settings_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(settings_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(settings_screen);

    label_settings_title = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(label_settings_title, &lv_font_inter_medium_20_caps, 0);
    lv_obj_set_style_text_color(label_settings_title, lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX), 0);
    lv_obj_set_style_text_letter_space(label_settings_title, 2, 0);
    lv_obj_align(label_settings_title, LV_ALIGN_TOP_MID, 0, 56);

    settings_overlay_icon_bg = lv_obj_create(settings_screen);
    lv_obj_clear_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_overlay_icon_bg, 64, 64);
    lv_obj_set_style_radius(settings_overlay_icon_bg, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_overlay_icon_bg, lv_color_hex(0x27272A), 0);
    lv_obj_set_style_bg_opa(settings_overlay_icon_bg, (lv_opa_t)85, 0);
    lv_obj_set_style_border_width(settings_overlay_icon_bg, 1, 0);
    lv_obj_set_style_border_color(settings_overlay_icon_bg, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_border_opa(settings_overlay_icon_bg, (lv_opa_t)45, 0);
    lv_obj_add_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_icon = lv_label_create(settings_overlay_icon_bg);
    lv_obj_set_style_text_font(settings_overlay_icon, ui_font_lucide_20(), 0);
    lv_obj_set_style_text_color(settings_overlay_icon, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_center(settings_overlay_icon);

    settings_overlay_title = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(settings_overlay_title, lv_color_white(), 0);
    lv_obj_set_style_text_align(settings_overlay_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_title, 320);
    lv_obj_add_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_line1 = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_line1, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_overlay_line1, lv_color_hex(0xC3CAD6), 0);
    lv_obj_set_style_text_align(settings_overlay_line1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_line1, 320);
    lv_obj_add_flag(settings_overlay_line1, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_line2 = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_line2, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_overlay_line2, lv_color_hex(0x9CA3AF), 0);
    lv_obj_set_style_text_align(settings_overlay_line2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_line2, 320);
    lv_obj_add_flag(settings_overlay_line2, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_line3 = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_line3, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_overlay_line3, lv_color_hex(0x7A7A83), 0);
    lv_obj_set_style_text_align(settings_overlay_line3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_line3, 320);
    lv_obj_add_flag(settings_overlay_line3, LV_OBJ_FLAG_HIDDEN);

    {
        lv_display_t *disp = lv_obj_get_display(settings_screen);
        settings_blur_color_format = LV_COLOR_FORMAT_RGB565;
        if (disp) {
            lv_color_format_t disp_cf = lv_display_get_color_format(disp);
            if (disp_cf == LV_COLOR_FORMAT_RGB565_SWAPPED || disp_cf == LV_COLOR_FORMAT_RGB565) {
                settings_blur_color_format = disp_cf;
            }
        }

        uint32_t blur_stride = lv_draw_buf_width_to_stride(EXAMPLE_LCD_H_RES, settings_blur_color_format);
        uint32_t blur_buf_size = blur_stride * SETTINGS_BLUR_BAND_H;
        if (!settings_blur_top_buf) {
            settings_blur_top_buf = lv_malloc(blur_buf_size);
        }
        if (!settings_blur_bottom_buf) {
            settings_blur_bottom_buf = lv_malloc(blur_buf_size);
        }

        if (settings_blur_top_buf && settings_blur_bottom_buf) {
            memset(settings_blur_top_buf, 0, blur_buf_size);
            memset(settings_blur_bottom_buf, 0, blur_buf_size);

            settings_blur_top_overlay = lv_canvas_create(settings_screen);
            lv_canvas_set_buffer(settings_blur_top_overlay, settings_blur_top_buf,
                                 EXAMPLE_LCD_H_RES, SETTINGS_BLUR_BAND_H, settings_blur_color_format);
            lv_obj_remove_flag(settings_blur_top_overlay, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_border_width(settings_blur_top_overlay, 0, 0);
            lv_obj_set_style_bg_opa(settings_blur_top_overlay, LV_OPA_TRANSP, 0);
            lv_obj_set_style_image_recolor(settings_blur_top_overlay, lv_color_black(), 0);
            lv_obj_set_style_image_recolor_opa(settings_blur_top_overlay, (lv_opa_t)28, 0);
            lv_obj_set_pos(settings_blur_top_overlay, 0, 0);
            lv_obj_add_flag(settings_blur_top_overlay, LV_OBJ_FLAG_HIDDEN);

            settings_blur_bottom_overlay = lv_canvas_create(settings_screen);
            lv_canvas_set_buffer(settings_blur_bottom_overlay, settings_blur_bottom_buf,
                                 EXAMPLE_LCD_H_RES, SETTINGS_BLUR_BAND_H, settings_blur_color_format);
            lv_obj_remove_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_border_width(settings_blur_bottom_overlay, 0, 0);
            lv_obj_set_style_bg_opa(settings_blur_bottom_overlay, LV_OPA_TRANSP, 0);
            lv_obj_set_style_image_recolor(settings_blur_bottom_overlay, lv_color_black(), 0);
            lv_obj_set_style_image_recolor_opa(settings_blur_bottom_overlay, (lv_opa_t)28, 0);
            lv_obj_set_pos(settings_blur_bottom_overlay, 0, EXAMPLE_LCD_V_RES - SETTINGS_BLUR_BAND_H);
            lv_obj_add_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_HIDDEN);

            if (!settings_blur_snapshot_buf) {
                settings_blur_snapshot_buf = lv_snapshot_create_draw_buf(settings_screen, settings_blur_color_format);
            }
        }
    }

    settings_slider_caption = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_slider_caption, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_slider_caption, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_align(settings_slider_caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_slider_caption, 300);
    lv_obj_add_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_title = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_title, &lv_font_inter_medium_20_caps, 0);
    lv_obj_set_style_text_color(settings_sensitivity_title, lv_color_hex(0x71717A), 0);
    lv_obj_set_style_text_letter_space(settings_sensitivity_title, 2, 0);
    lv_label_set_text(settings_sensitivity_title, "SENSITIVITY");
    lv_obj_align(settings_sensitivity_title, LV_ALIGN_TOP_MID, 0, 66);
    lv_obj_add_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_value = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_value, &lv_font_inter_medium_145_num, 0);
    lv_obj_set_style_text_color(settings_sensitivity_value, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(settings_sensitivity_value, -2, 0);
    lv_obj_set_style_text_align(settings_sensitivity_value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_sensitivity_value, 320);
    lv_label_set_text(settings_sensitivity_value, "15");
    lv_obj_align(settings_sensitivity_value, LV_ALIGN_TOP_MID, 0, 96);
    lv_obj_add_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_subtitle = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_subtitle, &lv_font_inter_medium_12_caps, 0);
    lv_obj_set_style_text_color(settings_sensitivity_subtitle, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_letter_space(settings_sensitivity_subtitle, 3, 0);
    lv_label_set_text(settings_sensitivity_subtitle, "CLICKS PER 100%");
    lv_obj_align(settings_sensitivity_subtitle, LV_ALIGN_TOP_MID, 0, 252);
    lv_obj_add_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);

    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        settings_items[i] = lv_obj_create(settings_screen);
        lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(settings_items[i], 368, 98);
        lv_obj_set_style_bg_color(settings_items[i], lv_color_hex(0x121214), 0);
        lv_obj_set_style_bg_opa(settings_items[i], (lv_opa_t)64, 0);
        lv_obj_set_style_border_width(settings_items[i], 1, 0);
        lv_obj_set_style_border_color(settings_items[i], lv_palette_darken(LV_PALETTE_GREY, 3), 0);
        lv_obj_set_style_radius(settings_items[i], 30, 0);
        lv_obj_set_style_pad_left(settings_items[i], 18, 0);
        lv_obj_set_style_pad_right(settings_items[i], 16, 0);
        lv_obj_set_style_pad_top(settings_items[i], 10, 0);
        lv_obj_set_style_pad_bottom(settings_items[i], 10, 0);

        settings_item_icon_bgs[i] = lv_obj_create(settings_items[i]);
        lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(settings_item_icon_bgs[i], 64, 64);
        lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_item_icon_bgs[i], lv_color_hex(0x27272A), 0);
        lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], (lv_opa_t)90, 0);
        lv_obj_set_style_border_width(settings_item_icon_bgs[i], 1, 0);
        lv_obj_set_style_border_color(settings_item_icon_bgs[i], lv_color_hex(0x9F9FA9), 0);
        lv_obj_set_style_border_opa(settings_item_icon_bgs[i], (lv_opa_t)36, 0);

        lv_obj_set_style_pad_all(settings_item_icon_bgs[i], 0, 0);
        settings_item_icons[i] = lv_label_create(settings_item_icon_bgs[i]);
        lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
        lv_obj_set_style_text_color(settings_item_icons[i], lv_color_hex(UI_TEXT_MUTED_HEX), 0);
        lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 1, 0);

        settings_item_titles[i] = lv_label_create(settings_items[i]);
        lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_16_ui, 0);
        lv_obj_set_style_text_color(settings_item_titles[i], lv_color_hex(0xDCE2ED), 0);
        lv_obj_align(settings_item_titles[i], LV_ALIGN_LEFT_MID, 100, -16);
        settings_item_values[i] = lv_label_create(settings_items[i]);
        lv_obj_set_style_text_font(settings_item_values[i], &lv_font_inter_medium_16_ui, 0);
        lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(0x5D5D66), 0);
        lv_obj_set_style_text_opa(settings_item_values[i], (lv_opa_t)130, 0);
        lv_obj_align(settings_item_values[i], LV_ALIGN_LEFT_MID, 100, 20);
        settings_item_arrows[i] = lv_label_create(settings_items[i]);
        lv_obj_set_style_text_font(settings_item_arrows[i], ui_font_lucide_20(), 0);
        lv_obj_set_style_transform_pivot_x(settings_item_arrows[i], 8, 0);
        lv_obj_set_style_transform_pivot_y(settings_item_arrows[i], 10, 0);
        lv_obj_set_style_transform_angle(settings_item_arrows[i], 1800, 0);    /* chevron-left -> chevron-right */
        lv_obj_set_style_transform_zoom(settings_item_arrows[i], 332, 0);      /* about +30% */
        lv_obj_set_style_text_color(settings_item_arrows[i], lv_color_hex(UI_TEXT_MUTED_HEX), 0);
        lv_obj_set_style_text_letter_space(settings_item_arrows[i], 0, 0);
        lv_label_set_text(settings_item_arrows[i], UI_ICON_LUCIDE_CHEVRON_LEFT);
        lv_obj_set_width(settings_item_arrows[i], 44);
        lv_obj_set_style_text_align(settings_item_arrows[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(settings_item_arrows[i], LV_ALIGN_RIGHT_MID, 24, 0);
    }

    settings_focus_frame = lv_obj_create(settings_screen);
    lv_obj_clear_flag(settings_focus_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_focus_frame, 368, 98);
    lv_obj_set_style_radius(settings_focus_frame, 30, 0);
    lv_obj_set_style_bg_opa(settings_focus_frame, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(settings_focus_frame, 3, 0);
    lv_obj_set_style_border_color(settings_focus_frame, lv_color_hex(0x9F9FA9), 0);
    lv_obj_set_style_border_opa(settings_focus_frame, (lv_opa_t)90, 0);
    lv_obj_add_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
    settings_value_slider_track = lv_obj_create(settings_screen);
    lv_obj_remove_style_all(settings_value_slider_track);
    lv_obj_clear_flag(settings_value_slider_track, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_value_slider_track, 288, 16);
    lv_obj_align(settings_value_slider_track, LV_ALIGN_TOP_MID, 0, 294);
    lv_obj_set_style_radius(settings_value_slider_track, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_value_slider_track, lv_color_hex(0x3F3F46), 0);
    lv_obj_set_style_bg_opa(settings_value_slider_track, LV_OPA_COVER, 0);
    lv_obj_set_style_clip_corner(settings_value_slider_track, false, 0);

    settings_value_slider_fill = lv_obj_create(settings_value_slider_track);
    lv_obj_remove_style_all(settings_value_slider_fill);
    lv_obj_clear_flag(settings_value_slider_fill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_value_slider_fill, 0, 16);
    lv_obj_set_pos(settings_value_slider_fill, 0, 0);
    lv_obj_set_style_radius(settings_value_slider_fill, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_value_slider_fill, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_bg_opa(settings_value_slider_fill, LV_OPA_COVER, 0);

    settings_value_slider_knob = lv_obj_create(settings_value_slider_track);
    lv_obj_remove_style_all(settings_value_slider_knob);
    lv_obj_clear_flag(settings_value_slider_knob, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_value_slider_knob, 28, 28);
    lv_obj_set_style_radius(settings_value_slider_knob, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_value_slider_knob, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_bg_opa(settings_value_slider_knob, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(settings_value_slider_knob, 3, 0);
    lv_obj_set_style_border_color(settings_value_slider_knob, lv_color_white(), 0);
    lv_obj_set_style_border_opa(settings_value_slider_knob, LV_OPA_COVER, 0);

    lv_obj_add_flag(settings_value_slider_track, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(settings_value_slider_fill, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);

    settings_enter_view(SETTINGS_VIEW_ROOT);
}
static void create_language_screen(void)
{
    language_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(language_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(language_screen);

    label_language_title = lv_label_create(language_screen);
    lv_obj_set_style_text_font(label_language_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_language_title, lv_color_white(), 0);
    lv_label_set_text(label_language_title, ui_strings[current_lang].language);
    lv_obj_align(label_language_title, LV_ALIGN_TOP_MID, 0, 12);

    roller_language = lv_roller_create(language_screen);
    lv_obj_set_width(roller_language, 220);
    lv_obj_align(roller_language, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(roller_language, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_language, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_language, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_language, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_language, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_language, lv_color_white(), LV_PART_SELECTED);

    rebuild_language_options();
    lv_roller_set_options(roller_language, language_roller_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_language, 4);
    lv_roller_set_selected(roller_language, current_lang, LV_ANIM_OFF);
}

static void create_owner_screen(void)
{
    owner_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(owner_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(owner_screen);

    label_owner_title = lv_label_create(owner_screen);
    lv_obj_set_style_text_font(label_owner_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_owner_title, lv_color_white(), 0);
    lv_label_set_text(label_owner_title, ui_strings[current_lang].owner_name);
    lv_obj_align(label_owner_title, LV_ALIGN_TOP_MID, 0, 12);

    label_owner_value = lv_label_create(owner_screen);
    lv_label_set_text(label_owner_value, owner_name_len ? owner_name : "-");
    lv_obj_set_style_text_font(label_owner_value, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_owner_value, lv_color_white(), 0);
    lv_obj_align(label_owner_value, LV_ALIGN_TOP_MID, 0, 46);

    roller_owner = lv_roller_create(owner_screen);
    lv_obj_set_width(roller_owner, 200);
    lv_obj_align(roller_owner, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(roller_owner, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_owner, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_owner, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_owner, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_owner, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_owner, lv_color_white(), LV_PART_SELECTED);
    lv_roller_set_visible_row_count(roller_owner, 4);
    lv_roller_set_options(roller_owner,
                          "A\nB\nC\nD\nE\nF\nG\nH\nI\nJ\nK\nL\nM\nN\nO\nP\nQ\nR\nS\nT\nU\nV\nW\nX\nY\nZ\n"
                          "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl\nm\nn\no\np\nq\nr\ns\nt\nu\nv\nw\nx\ny\nz\n"
                          "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n"
                          "space\n<-\nSave",
                          LV_ROLLER_MODE_NORMAL);
}

static int get_sensitivity_from_roller(void)
{
    if (!roller_sensitivity) {
        return speed_sensitivity_clicks;
    }
    int selected = lv_roller_get_selected(roller_sensitivity);
    int value = SENSITIVITY_MIN_CLICKS + selected;
    if (value < SENSITIVITY_MIN_CLICKS) {
        value = SENSITIVITY_MIN_CLICKS;
    }
    if (value > SENSITIVITY_MAX_CLICKS) {
        value = SENSITIVITY_MAX_CLICKS;
    }
    return value;
}

static void refresh_sensitivity_value_label(void)
{
    if (!label_sensitivity_value) {
        return;
    }
    char buf[24];
    snprintf(buf, sizeof(buf), "%d clicks", get_sensitivity_from_roller());
    lv_label_set_text(label_sensitivity_value, buf);
}

static void create_sensitivity_screen(void)
{
    sensitivity_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(sensitivity_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(sensitivity_screen);

    label_sensitivity_title = lv_label_create(sensitivity_screen);
    lv_obj_set_style_text_font(label_sensitivity_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_sensitivity_title, lv_color_white(), 0);
    lv_label_set_text(label_sensitivity_title, "Sensitivity");
    lv_obj_align(label_sensitivity_title, LV_ALIGN_TOP_MID, 0, 12);

    label_sensitivity_value = lv_label_create(sensitivity_screen);
    lv_obj_set_style_text_font(label_sensitivity_value, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_sensitivity_value, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
    lv_obj_align(label_sensitivity_value, LV_ALIGN_TOP_MID, 0, 48);

    roller_sensitivity = lv_roller_create(sensitivity_screen);
    lv_obj_set_width(roller_sensitivity, 220);
    lv_obj_align(roller_sensitivity, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(roller_sensitivity, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_sensitivity, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_sensitivity, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_sensitivity, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_sensitivity, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_sensitivity, lv_color_white(), LV_PART_SELECTED);
    lv_roller_set_visible_row_count(roller_sensitivity, 5);

    static char options[(SENSITIVITY_MAX_CLICKS - SENSITIVITY_MIN_CLICKS + 1) * 5];
    options[0] = '\0';
    size_t offset = 0;
    for (int v = SENSITIVITY_MIN_CLICKS; v <= SENSITIVITY_MAX_CLICKS; ++v) {
        int written = snprintf(options + offset, sizeof(options) - offset, "%d%s", v, (v < SENSITIVITY_MAX_CLICKS) ? "\n" : "");
        if (written <= 0) {
            break;
        }
        offset += (size_t)written;
        if (offset >= sizeof(options)) {
            break;
        }
    }
    lv_roller_set_options(roller_sensitivity, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(roller_sensitivity, speed_sensitivity_clicks - SENSITIVITY_MIN_CLICKS, LV_ANIM_OFF);
    refresh_sensitivity_value_label();
}

static void create_overdrive_screen(void)
{
    overdrive_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(overdrive_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(overdrive_screen);

    label_overdrive_title = lv_label_create(overdrive_screen);
    lv_obj_set_style_text_font(label_overdrive_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_overdrive_title, lv_color_white(), 0);
    lv_label_set_text(label_overdrive_title, "Overdrive");
    lv_obj_align(label_overdrive_title, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *desc = lv_label_create(overdrive_screen);
    lv_obj_set_style_text_font(desc, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(desc, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_label_set_text(desc, "Allows 110% when filter is critical");
    lv_obj_align(desc, LV_ALIGN_TOP_MID, 0, 48);

    roller_overdrive = lv_roller_create(overdrive_screen);
    lv_obj_set_width(roller_overdrive, 220);
    lv_obj_align(roller_overdrive, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(roller_overdrive, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_overdrive, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_overdrive, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_overdrive, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_overdrive, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_overdrive, lv_color_white(), LV_PART_SELECTED);
    lv_roller_set_options(roller_overdrive, "Disabled\nEnabled", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller_overdrive, 3);
    lv_roller_set_selected(roller_overdrive, allow_overdrive ? 1 : 0, LV_ANIM_OFF);
}

static void create_purification_screen(void)
{
    purification_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(purification_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(purification_screen);

    label_purification_title = lv_label_create(purification_screen);
    lv_obj_set_style_text_font(label_purification_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_purification_title, lv_color_hex(UI_FILTER_GOOD_HEX), 0);
    lv_obj_align(label_purification_title, LV_ALIGN_TOP_MID, 0, 150);

    label_purification_state = lv_label_create(purification_screen);
    lv_obj_set_style_text_font(label_purification_state, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_purification_state, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
    lv_obj_align(label_purification_state, LV_ALIGN_TOP_MID, 0, 184);

    lv_obj_t *spinner = lv_spinner_create(purification_screen);
    lv_obj_set_size(spinner, 132, 132);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(UI_FILTER_GOOD_HEX), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(spinner, LV_OPA_90, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_MAIN);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -16);

    lv_obj_t *hint = lv_label_create(purification_screen);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_TEXT_MUTED_HEX), 0);
    lv_label_set_text(hint, "Double click to stop");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -40);
}

static void create_sleep_screen(void)
{
    sleep_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(sleep_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(sleep_screen);
}

static void ui_init(void)
{
    create_boot_screen();
    create_main_screen();
    create_settings_screen();
    create_language_screen();
    create_owner_screen();
    create_sensitivity_screen();
    create_overdrive_screen();
    create_purification_screen();
    create_sleep_screen();
    fps_counter_create();

    show_screen(boot_screen);
    update_main_ui();
    apply_language();
    start_boot_animation();

    reset_sleep_timer();

    if (!boot_timer) {
        esp_timer_create_args_t timer_args = {
            .callback = &boot_timer_cb,
            .name = "boot_timer",
        };
        ESP_ERROR_CHECK(esp_timer_create(&timer_args, &boot_timer));
    }
    esp_timer_start_once(boot_timer, UI_BOOT_TOTAL_TIME_US);
}

static void start_purification_mode(void)
{
    is_purifying = true;
    set_speed_percent(10);
    apply_fan_pwm(fan_speed_percent);
    ui_screen = UI_SCREEN_PURIFICATION;
    show_screen(purification_screen);
    apply_language();
}

static void stop_purification_mode(bool go_to_main)
{
    is_purifying = false;
    set_speed_percent(FAN_SPEED_MIN_PERCENT);
    apply_fan_pwm(fan_speed_percent);
    update_main_ui();
    if (go_to_main) {
        ui_screen = UI_SCREEN_MAIN;
        show_screen(main_screen);
        set_lock_overlay(ui_locked);
    }
    apply_language();
}

static void handle_double_click(void)
{
    if (ui_sleeping) {
        return;
    }
    reset_sleep_timer();
    if (is_purifying) {
        stop_purification_mode(true);
    }
}

static void handle_knob_move(int direction)
{
    if (ui_sleeping) {
        return;
    }
    reset_sleep_timer();

    if (ui_screen == UI_SCREEN_MAIN) {
        if (ui_locked) {
            return;
        }
        int64_t now_us = esp_timer_get_time();
        if ((now_us - main_knob_last_update_us) < 6000) {
            return;
        }
        main_knob_last_update_us = now_us;

        int prev_speed_percent = fan_speed_percent;
        int step_tenths = (get_max_speed_percent() * 10 + (speed_sensitivity_clicks / 2)) / speed_sensitivity_clicks;
        if (step_tenths < 1) {
            step_tenths = 1;
        }
        fan_speed_tenths += direction * step_tenths;
        clamp_speed_to_limits();
        if (fan_speed_percent == prev_speed_percent) {
            return;
        }
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(main)");
            return;
        }
        update_main_ui();
        lvgl_port_unlock();
        apply_fan_pwm(fan_speed_percent);
        return;
    }

    if (ui_screen == UI_SCREEN_SETTINGS) {
        int64_t now_us = esp_timer_get_time();
        if ((now_us - settings_knob_last_update_us) < SETTINGS_KNOB_MIN_INTERVAL_US) {
            return;
        }
        settings_knob_last_update_us = now_us;
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(settings)");
            return;
        }
        if (settings_edit_mode) {
            settings_adjust_edited_value(direction);
        } else {
            int count = settings_items_count > 0 ? settings_items_count : settings_get_item_count();
            int prev_index = settings_index;
            settings_index += direction;
            if (settings_index < 0) {
                settings_index = count - 1;
            }
            if (settings_index >= count) {
                settings_index = 0;
            }

            bool wrapped = (direction > 0 && prev_index == count - 1 && settings_index == 0) ||
                           (direction < 0 && prev_index == 0 && settings_index == count - 1);
            if (settings_view_uses_wheel_list(settings_view) && !wrapped) {
                settings_start_root_scroll_anim();
            } else {
                settings_sync_root_scroll_to_index();
                update_settings_wheel_layout();
            }
        }
        lvgl_port_unlock();
        return;
    }

    if (ui_screen == UI_SCREEN_LANGUAGE && roller_language) {
        int selected = lv_roller_get_selected(roller_language);
        selected += direction;
        if (selected < 0) {
            selected = LANG_COUNT - 1;
        }
        if (selected >= LANG_COUNT) {
            selected = 0;
        }
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(language)");
            return;
        }
        lv_roller_set_selected(roller_language, selected, LV_ANIM_ON);
        lvgl_port_unlock();
        return;
    }

    if (ui_screen == UI_SCREEN_OWNER && roller_owner) {
        int selected = lv_roller_get_selected(roller_owner);
        selected += direction;
        if (selected < 0) {
            selected = lv_roller_get_option_cnt(roller_owner) - 1;
        }
        if (selected >= lv_roller_get_option_cnt(roller_owner)) {
            selected = 0;
        }
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(owner)");
            return;
        }
        lv_roller_set_selected(roller_owner, selected, LV_ANIM_ON);
        lvgl_port_unlock();
        return;
    }

    if (ui_screen == UI_SCREEN_SENSITIVITY && roller_sensitivity) {
        int selected = lv_roller_get_selected(roller_sensitivity);
        selected += direction;
        if (selected < 0) {
            selected = 0;
        }
        int max_index = SENSITIVITY_MAX_CLICKS - SENSITIVITY_MIN_CLICKS;
        if (selected > max_index) {
            selected = max_index;
        }
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(sensitivity)");
            return;
        }
        lv_roller_set_selected(roller_sensitivity, selected, LV_ANIM_ON);
        refresh_sensitivity_value_label();
        lvgl_port_unlock();
        return;
    }

    if (ui_screen == UI_SCREEN_OVERDRIVE && roller_overdrive) {
        int selected = lv_roller_get_selected(roller_overdrive);
        selected += direction;
        if (selected < 0) {
            selected = 0;
        }
        if (selected > 1) {
            selected = 1;
        }
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(overdrive)");
            return;
        }
        lv_roller_set_selected(roller_overdrive, selected, LV_ANIM_ON);
        lvgl_port_unlock();
    }
}

static void handle_owner_selection(void)
{
    if (!roller_owner || !label_owner_value) {
        return;
    }
    char buf[16] = {0};
    lv_roller_get_selected_str(roller_owner, buf, sizeof(buf));
    if (strcmp(buf, "<-") == 0) {
        if (owner_name_len > 0) {
            owner_name[owner_name_len - 1] = '\0';
            owner_name_len--;
        }
    } else if (strcmp(buf, "Save") == 0) {
        save_owner_name();
        ui_screen = UI_SCREEN_SETTINGS;
        show_screen(settings_screen);
        apply_language();
        update_settings_selection();
        return;
    } else {
        char ch = 0;
        if (strcmp(buf, "space") == 0) {
            ch = ' ';
        } else {
            ch = buf[0];
        }
        if (owner_name_len < OWNER_NAME_MAX_LEN) {
            owner_name[owner_name_len] = ch;
            owner_name_len++;
            owner_name[owner_name_len] = '\0';
        }
    }
    lv_label_set_text(label_owner_value, owner_name_len ? owner_name : "-");
}

static void handle_single_click(void)
{
    if (ui_sleeping) {
        wake_from_sleep_locked();
        set_backlight_enabled(true);
        reset_sleep_timer();
        return;
    }
    reset_sleep_timer();

    if (ui_screen == UI_SCREEN_MAIN) {
        if (allow_child_lock) {
            ui_locked = !ui_locked;
            set_lock_overlay(ui_locked);
        }
        return;
    }

    if (ui_screen == UI_SCREEN_SETTINGS) {
        settings_activate_item();
        return;
    }

    if (ui_screen == UI_SCREEN_LANGUAGE && roller_language) {
        int selected = lv_roller_get_selected(roller_language);
        if (selected >= 0 && selected < LANG_COUNT) {
            current_lang = (ui_lang_t)selected;
            apply_language();
            save_language();
        }
        ui_screen = UI_SCREEN_SETTINGS;
        show_screen(settings_screen);
        return;
    }

    if (ui_screen == UI_SCREEN_OWNER) {
        handle_owner_selection();
        return;
    }

    if (ui_screen == UI_SCREEN_SENSITIVITY) {
        speed_sensitivity_clicks = get_sensitivity_from_roller();
        save_control_settings();
        ui_screen = UI_SCREEN_SETTINGS;
        show_screen(settings_screen);
        apply_language();
        update_settings_selection();
        return;
    }

    if (ui_screen == UI_SCREEN_OVERDRIVE && roller_overdrive) {
        allow_overdrive = (lv_roller_get_selected(roller_overdrive) == 1);
        save_control_settings();
        clamp_speed_to_limits();
        apply_fan_pwm(fan_speed_percent);
        update_main_ui();
        ui_screen = UI_SCREEN_SETTINGS;
        show_screen(settings_screen);
        apply_language();
        update_settings_selection();
        return;
    }

    if (ui_screen == UI_SCREEN_PURIFICATION) {
        return;
    }
}

static void handle_long_press(void)
{
    if (ui_sleeping) {
        return;
    }
    reset_sleep_timer();

    if (ui_screen == UI_SCREEN_MAIN || ui_screen == UI_SCREEN_PURIFICATION) {
        ui_screen = UI_SCREEN_SETTINGS;
        settings_knob_last_update_us = 0;
        show_screen(settings_screen);
        settings_enter_view(SETTINGS_VIEW_ROOT);
    } else if (ui_screen == UI_SCREEN_SETTINGS) {
        settings_go_back();
    } else {
        ui_screen = UI_SCREEN_MAIN;
        show_screen(main_screen);
        set_lock_overlay(ui_locked);
        update_main_ui();
    }
}

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t[]){0x00}, 0, 0},
    {0xC4, (uint8_t[]){0x80}, 1, 0},
    {0x3A, (uint8_t[]){0x55}, 1, 0},
    {0x35, (uint8_t[]){0x00}, 0, 10},
    {0x53, (uint8_t[]){0x20}, 1, 10},
    {0x51, (uint8_t[]){0xFF}, 1, 10},
    {0x63, (uint8_t[]){0xFF}, 1, 10},
    //{0x2A, (uint8_t []){0x00,0x06,0x01,0xD7}, 4, 0},
    //{0x2B, (uint8_t []){0x00,0x00,0x01,0xD1}, 4, 0},
    {0x2A, (uint8_t[]){0x00, 0x06, 0x01, 0xDD}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xD1}, 4, 0},
    {0x11, (uint8_t[]){0x00}, 0, 60},
};


//***************The following code is for standardizing the event triggering of rotary encoders and buttons.**************** */
//**************旋钮********* */
void LVGL_knob_event(void *event)
{
}

void LVGL_button_event(void *event)
{
}

static knob_handle_t knob = NULL;

const char *knob_event_table[] = {
    "KNOB_LEFT",
    "KNOB_RIGHT",
    "KNOB_H_LIM",
    "KNOB_L_LIM",
    "KNOB_ZERO",
};

static void knob_event_cb(void *arg, void *data)
{
    ESP_LOGD(TAG, "knob event %s, %d", knob_event_table[(knob_event_t)data], iot_knob_get_count_value(knob));
    if ((knob_event_t)data == KNOB_LEFT) {
        handle_knob_move(-1);
    } else if ((knob_event_t)data == KNOB_RIGHT) {
        handle_knob_move(1);
    }
    LVGL_knob_event(data);
}

void knob_init(uint32_t encoder_a, uint32_t encoder_b)
{
    knob_config_t cfg = {
        .default_direction = 0,
        .gpio_encoder_a = encoder_a,
        .gpio_encoder_b = encoder_b,
#if CONFIG_PM_ENABLE
        .enable_power_save = true,
#endif
    };

    knob = iot_knob_create(&cfg);
    assert(knob);
    esp_err_t err = iot_knob_register_cb(knob, KNOB_LEFT, knob_event_cb, (void *)KNOB_LEFT);
    err |= iot_knob_register_cb(knob, KNOB_RIGHT, knob_event_cb, (void *)KNOB_RIGHT);
    err |= iot_knob_register_cb(knob, KNOB_H_LIM, knob_event_cb, (void *)KNOB_H_LIM);
    err |= iot_knob_register_cb(knob, KNOB_L_LIM, knob_event_cb, (void *)KNOB_L_LIM);
    err |= iot_knob_register_cb(knob, KNOB_ZERO, knob_event_cb, (void *)KNOB_ZERO);
    ESP_ERROR_CHECK(err);
}

/* 按键相关函数 */
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C6
#define BOOT_BUTTON_NUM 9
#else
#define BOOT_BUTTON_NUM 0
#endif
#define BUTTON_ACTIVE_LEVEL 0
const char *button_event_table[] = {
    "BUTTON_PRESS_DOWN",
    "BUTTON_PRESS_UP",
    "BUTTON_PRESS_REPEAT",
    "BUTTON_PRESS_REPEAT_DONE",
    "BUTTON_SINGLE_CLICK",
    "BUTTON_DOUBLE_CLICK",
    "BUTTON_MULTIPLE_CLICK",
    "BUTTON_LONG_PRESS_START",
    "BUTTON_LONG_PRESS_HOLD",
    "BUTTON_LONG_PRESS_UP",
    "BUTTON_PRESS_END",
};

static void button_event_cb(void *arg, void *data)
{
    button_event_t event = (button_event_t)data;
    ESP_LOGI(TAG, "Button event %s", button_event_table[event]);
    if (event == BUTTON_SINGLE_CLICK) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in button single click");
            return;
        }
        handle_single_click();
        lvgl_port_unlock();
    } else if (event == BUTTON_DOUBLE_CLICK) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in button double click");
            return;
        }
        handle_double_click();
        lvgl_port_unlock();
    } else if (event == BUTTON_LONG_PRESS_START) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in button long press");
            return;
        }
        handle_long_press();
        lvgl_port_unlock();
    }
    LVGL_button_event(data);
}
void button_init(uint32_t button_num)
{
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = button_num,
            .active_level = BUTTON_ACTIVE_LEVEL,
#if CONFIG_GPIO_BUTTON_SUPPORT_POWER_SAVE
            .enable_power_save = true,
#endif
        },
    };
    button_handle_t btn = iot_button_create(&btn_cfg);
    assert(btn);
    esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, button_event_cb, (void *)BUTTON_SINGLE_CLICK);
    err |= iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, button_event_cb, (void *)BUTTON_DOUBLE_CLICK);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_START, button_event_cb, (void *)BUTTON_LONG_PRESS_START);

#if CONFIG_ENTER_LIGHT_SLEEP_MODE_MANUALLY
    /*!< For enter Power Save */
    button_power_save_config_t config = {
        .enter_power_save_cb = button_enter_power_save,
    };
    err |= iot_button_register_power_save_cb(&config);
#endif

    ESP_ERROR_CHECK(err);
}

/* ============================================================================
 * 主函数
 * ============================================================================ */
static void uart_control_print_help(void)
{
    printf("\r\nUART control commands:\r\n");
    printf("  help          - show this help\r\n");
    printf("  + / up        - increase fan speed / move selection up\r\n");
    printf("  - / down      - decrease fan speed / move selection down\r\n");
    printf("  speed <0-110> - set fan speed in percent (110 only in overdrive)\r\n");
    printf("  filter <good|warning|critical> - set filter status color\r\n");
    printf("  click         - single click action\r\n");
    printf("  double        - double click action\r\n");
    printf("  hold          - long press action\r\n");
    printf("  lock          - lock main screen\r\n");
    printf("  unlock        - unlock main screen\r\n");
    printf("  purify <on|off> - start/stop purification mode\r\n");
    printf("  sleep / wake  - enter/exit eco sleep mode\r\n");
}

static void uart_control_set_speed(int speed)
{
    set_speed_percent(speed);
    apply_fan_pwm(fan_speed_percent);
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "LVGL lock timeout in uart set_speed");
        return;
    }
    update_main_ui();
    lvgl_port_unlock();
    ESP_LOGI(TAG, "Fan speed set to %d%%", fan_speed_percent);
}

static void uart_control_set_filter(filter_status_t new_status)
{
    filter_status = new_status;
    clamp_speed_to_limits();
    apply_fan_pwm(fan_speed_percent);
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "LVGL lock timeout in uart set_filter");
        return;
    }
    update_main_ui();
    lvgl_port_unlock();
    ESP_LOGI(TAG, "Filter status set to: %s", get_filter_status_text());
}

static void uart_control_exec(char *cmd)
{
    while (*cmd == ' ' || *cmd == '\t') {
        cmd++;
    }
    if (*cmd == '\0') {
        return;
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        uart_control_print_help();
        return;
    }
    if (strcmp(cmd, "+") == 0 || strcmp(cmd, "up") == 0) {
        handle_knob_move(1);
        return;
    }
    if (strcmp(cmd, "-") == 0 || strcmp(cmd, "down") == 0) {
        handle_knob_move(-1);
        return;
    }
    if (strncmp(cmd, "speed ", 6) == 0) {
        uart_control_set_speed(atoi(cmd + 6));
        return;
    }
    if (strncmp(cmd, "filter ", 7) == 0) {
        const char *value = cmd + 7;
        if (strcmp(value, "good") == 0) {
            uart_control_set_filter(FILTER_STATUS_GOOD);
        } else if (strcmp(value, "warning") == 0) {
            uart_control_set_filter(FILTER_STATUS_WARNING);
        } else if (strcmp(value, "critical") == 0) {
            uart_control_set_filter(FILTER_STATUS_CRITICAL);
        } else {
            ESP_LOGW(TAG, "Unknown filter status: %s", value);
        }
        return;
    }
    if (strcmp(cmd, "click") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart click");
            return;
        }
        handle_single_click();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "double") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart double");
            return;
        }
        handle_double_click();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "hold") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart hold");
            return;
        }
        handle_long_press();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "lock") == 0) {
        ui_locked = true;
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart lock");
            return;
        }
        set_lock_overlay(true);
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "unlock") == 0) {
        ui_locked = false;
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart unlock");
            return;
        }
        set_lock_overlay(false);
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "sleep") == 0) {
        enter_sleep_mode();
        return;
    }
    if (strcmp(cmd, "wake") == 0) {
        wake_from_sleep();
        return;
    }
    if (strcmp(cmd, "purify on") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart purify on");
            return;
        }
        start_purification_mode();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "purify off") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart purify off");
            return;
        }
        stop_purification_mode(true);
        lvgl_port_unlock();
        return;
    }

    ESP_LOGW(TAG, "Unknown UART command: %s", cmd);
}

static void uart_control_task(void *arg)
{
    (void)arg;
    char line[64];

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    uart_control_print_help();

    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }
        line[strcspn(line, "\r\n")] = '\0';
        uart_control_exec(line);
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    load_settings();
    apply_runtime_visual_settings();
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
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel, true));

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
    ui_init();
    // Release the mutex
    lvgl_port_unlock();

    xTaskCreate(uart_control_task, "uart_ctrl", 4096, NULL, 4, NULL);
}


















