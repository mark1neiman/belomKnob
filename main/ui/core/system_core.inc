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

static esp_err_t display_send_dcs_cmd(uint8_t cmd, const uint8_t *params, size_t len)
{
    if (!lcd_io) {
        return ESP_ERR_INVALID_STATE;
    }
    /* SH8601 on QSPI expects packed 32-bit command:
     * [31:24]=opcode(0x02 write cmd), [15:8]=DCS cmd, others 0. */
    const int packed_cmd = (int)(((uint32_t)0x02U << 24) | (((uint32_t)cmd & 0xFFU) << 8));
    return esp_lcd_panel_io_tx_param(lcd_io, packed_cmd, params, len);
}

static uint8_t display_brightness_percent_to_dbv(int percent)
{
    int clamped = clamp_int(percent, 0, 100);
    return (uint8_t)((clamped * 255 + 50) / 100);
}

static void display_init_brightness_control(void)
{
    /* WRCTRLD (0x53): BCTRL=1, DD=1 */
    const uint8_t ctrl = 0x28;
    esp_err_t err = display_send_dcs_cmd(0x53, &ctrl, 1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "WRCTRLD failed: %s", esp_err_to_name(err));
    }
}

static void display_set_brightness_percent(int percent)
{
    int clamped = clamp_int(percent, 0, 100);
    uint8_t dbv = display_brightness_percent_to_dbv(clamped);
    esp_err_t err = display_send_dcs_cmd(0x51, &dbv, 1); /* WRDISBV */
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "WRDISBV failed: %s", esp_err_to_name(err));
        return;
    }
    display_brightness_percent = clamped;
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

static void save_filter_usage_progress(void)
{
    nvs_handle_t handle;
    if (nvs_open("settings", NVS_READWRITE, &handle) == ESP_OK) {
        uint32_t max_filter_seconds = (filter_lifetime_hours > 0)
                                          ? ((uint32_t)filter_lifetime_hours * 3600U)
                                          : 0U;
        if (filter_used_seconds_total > max_filter_seconds) {
            filter_used_seconds_total = max_filter_seconds;
        }
        filter_used_hours = (int)(filter_used_seconds_total / 3600U);
        nvs_set_u16(handle, "fused", (uint16_t)filter_used_hours);
        nvs_set_u32(handle, "fsec", filter_used_seconds_total);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

static void save_control_settings(void)
{
    nvs_handle_t handle;
    if (nvs_open("settings", NVS_READWRITE, &handle) == ESP_OK) {
        uint32_t max_filter_seconds = (filter_lifetime_hours > 0)
                                          ? ((uint32_t)filter_lifetime_hours * 3600U)
                                          : 0U;
        if (filter_used_seconds_total > max_filter_seconds) {
            filter_used_seconds_total = max_filter_seconds;
        }
        filter_used_hours = (int)(filter_used_seconds_total / 3600U);

        nvs_set_u8(handle, "sense", (uint8_t)speed_sensitivity_clicks);
        nvs_set_u8(handle, "ovrd", allow_overdrive ? 1 : 0);
        nvs_set_u8(handle, "chld", allow_child_lock ? 1 : 0);
        nvs_set_u8(handle, "bright", (uint8_t)display_brightness_percent);
        nvs_set_u8(handle, "theme", display_theme_dark ? 1 : 0);
        nvs_set_u16(handle, "rot", (uint16_t)display_rotation_deg);
        nvs_set_u8(handle, "accent", (uint8_t)display_accent_index);
        nvs_set_u16(handle, "achue", (uint16_t)custom_accent_hue);
        nvs_set_u16(handle, "fused", (uint16_t)filter_used_hours);
        nvs_set_u32(handle, "fsec", filter_used_seconds_total);
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
        bool loaded_filter_seconds = false;
        bool has_other_settings = false;
        uint8_t lang = 0;
        uint8_t sense = SENSITIVITY_DEFAULT_CLICKS;
        uint8_t ovrd = 0;
        uint8_t chld = 1;
        uint8_t bright = DISPLAY_BRIGHTNESS_DEFAULT;
        uint8_t theme = 1;
        uint16_t rot = 0;
        uint8_t accent = 0;
        uint16_t achue = 0;
        uint32_t fsec = (uint32_t)FILTER_USED_DEFAULT_HOURS * 3600U;
        uint16_t fused = FILTER_USED_DEFAULT_HOURS;
        uint16_t flife = FILTER_LIFETIME_DEFAULT_HOURS;
        if (nvs_get_u8(handle, "lang", &lang) == ESP_OK && lang < LANG_COUNT) {
            current_lang = (ui_lang_t)lang;
            has_other_settings = true;
        }
        if (nvs_get_u8(handle, "sense", &sense) == ESP_OK) {
            if (sense >= SENSITIVITY_MIN_CLICKS && sense <= SENSITIVITY_MAX_CLICKS) {
                speed_sensitivity_clicks = sense;
            }
            has_other_settings = true;
        }
        if (nvs_get_u8(handle, "ovrd", &ovrd) == ESP_OK) {
            allow_overdrive = (ovrd != 0);
            has_other_settings = true;
        }
        if (nvs_get_u8(handle, "chld", &chld) == ESP_OK) {
            allow_child_lock = (chld != 0);
            has_other_settings = true;
        }
        if (nvs_get_u8(handle, "bright", &bright) == ESP_OK) {
            if (bright <= DISPLAY_BRIGHTNESS_MAX) {
                display_brightness_percent = bright;
            }
            has_other_settings = true;
        }
        if (nvs_get_u8(handle, "theme", &theme) == ESP_OK) {
            display_theme_dark = (theme != 0);
            has_other_settings = true;
        }
        if (nvs_get_u16(handle, "rot", &rot) == ESP_OK) {
            if (rot == 0 || rot == 180) {
                display_rotation_deg = (int)rot;
            }
            has_other_settings = true;
        }
        if (nvs_get_u8(handle, "accent", &accent) == ESP_OK) {
            if (accent <= DISPLAY_ACCENT_MAX_INDEX) {
                display_accent_index = accent;
            }
            has_other_settings = true;
        }
        if (nvs_get_u16(handle, "achue", &achue) == ESP_OK) {
            accent_set_custom_hue((int)achue);
            has_other_settings = true;
        }
        if (nvs_get_u32(handle, "fsec", &fsec) == ESP_OK) {
            filter_used_seconds_total = fsec;
            loaded_filter_seconds = true;
        }
        if (nvs_get_u16(handle, "fused", &fused) == ESP_OK) {
            filter_used_hours = fused;
        }
        if (nvs_get_u16(handle, "flife", &flife) == ESP_OK) {
            if (flife >= FILTER_LIFETIME_MIN_HOURS && flife <= FILTER_LIFETIME_MAX_HOURS) {
                filter_lifetime_hours = flife;
            }
            has_other_settings = true;
        }
        size_t len = sizeof(owner_name);
        if (nvs_get_str(handle, "owner", owner_name, &len) == ESP_OK) {
            owner_name_len = strnlen(owner_name, sizeof(owner_name) - 1);
            has_other_settings = true;
        }
        nvs_close(handle);

        if (!loaded_filter_seconds) {
            filter_used_seconds_total = (uint32_t)filter_used_hours * 3600U;
        }

        /* Fresh boot profile: if no other settings exist yet, force clean filter usage. */
        if (!has_other_settings) {
            filter_used_seconds_total = 0;
            filter_used_hours = 0;
            filter_runtime_partial_us = 0;
            filter_runtime_unsaved_us = 0;
        }
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
    {
        uint32_t max_filter_seconds = (uint32_t)filter_lifetime_hours * 3600U;
        if (filter_used_seconds_total > max_filter_seconds) {
            filter_used_seconds_total = max_filter_seconds;
        }
        filter_used_hours = (int)(filter_used_seconds_total / 3600U);
    }
    if (display_brightness_percent < DISPLAY_BRIGHTNESS_MIN) {
        display_brightness_percent = DISPLAY_BRIGHTNESS_MIN;
    }
    if (display_brightness_percent > DISPLAY_BRIGHTNESS_MAX) {
        display_brightness_percent = DISPLAY_BRIGHTNESS_MAX;
    }
    if (display_accent_index < 0 || display_accent_index > DISPLAY_ACCENT_MAX_INDEX) {
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

static bool accent_is_custom_index(int index)
{
#if UI_ENABLE_CUSTOM_ACCENT
    return index == DISPLAY_ACCENT_CUSTOM_INDEX;
#else
    (void)index;
    return false;
#endif
}

static int accent_normalize_hue(int hue_deg)
{
    int hue = hue_deg % 360;
    if (hue < 0) {
        hue += 360;
    }
    return hue;
}

static uint32_t accent_hex_from_hue(int hue_deg)
{
    float h = (float)accent_normalize_hue(hue_deg) / 60.0f;
    int sector = (int)floorf(h);
    float x = 1.0f - fabsf(fmodf(h, 2.0f) - 1.0f);

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    switch (sector) {
    case 0: r = 1.0f; g = x;    b = 0.0f; break;
    case 1: r = x;    g = 1.0f; b = 0.0f; break;
    case 2: r = 0.0f; g = 1.0f; b = x;    break;
    case 3: r = 0.0f; g = x;    b = 1.0f; break;
    case 4: r = x;    g = 0.0f; b = 1.0f; break;
    default:r = 1.0f; g = 0.0f; b = x;    break;
    }

    uint8_t rr = (uint8_t)lroundf(r * 255.0f);
    uint8_t gg = (uint8_t)lroundf(g * 255.0f);
    uint8_t bb = (uint8_t)lroundf(b * 255.0f);
    return ((uint32_t)rr << 16) | ((uint32_t)gg << 8) | (uint32_t)bb;
}

static void accent_set_custom_hue(int hue_deg)
{
    custom_accent_hue = accent_normalize_hue(hue_deg);
}

static bool settings_is_custom_accent_edit_active(void)
{
#if UI_ENABLE_CUSTOM_ACCENT
    return (settings_view == SETTINGS_VIEW_ACCENT) &&
           settings_edit_mode &&
           (settings_index == DISPLAY_ACCENT_CUSTOM_INDEX);
#else
    return false;
#endif
}

static void refresh_filter_status_from_usage(void)
{
    if (filter_lifetime_hours <= 0) {
        filter_status = FILTER_STATUS_CRITICAL;
        return;
    }
    int wear_percent = (filter_used_hours * 100) / filter_lifetime_hours;
    if (wear_percent > 80) {
        filter_status = FILTER_STATUS_CRITICAL;
    } else if (wear_percent > 50) {
        filter_status = FILTER_STATUS_WARNING;
    } else {
        filter_status = FILTER_STATUS_GOOD;
    }
}

static void apply_runtime_visual_settings(void)
{
    display_accent_index = clamp_int(display_accent_index, 0, DISPLAY_ACCENT_MAX_INDEX);
    if (accent_is_custom_index(display_accent_index)) {
        ui_accent_color_hex = accent_hex_from_hue(custom_accent_hue);
    } else {
        ui_accent_color_hex = accent_palette_hex[display_accent_index];
    }
    if (boot_progress_fill) {
        lv_obj_set_style_bg_color(boot_progress_fill, lv_color_hex(ui_accent_color_hex), 0);
    }
    filter_lifetime_hours = clamp_int(filter_lifetime_hours, FILTER_LIFETIME_MIN_HOURS, FILTER_LIFETIME_MAX_HOURS);
    {
        uint32_t max_filter_seconds = (uint32_t)filter_lifetime_hours * 3600U;
        if (filter_used_seconds_total > max_filter_seconds) {
            filter_used_seconds_total = max_filter_seconds;
        }
        filter_used_hours = clamp_int((int)(filter_used_seconds_total / 3600U), 0, filter_lifetime_hours);
    }
    refresh_filter_status_from_usage();
}

static void apply_dev_test_overrides(void)
{
#if UI_DEV_TEST_ENABLE
    filter_lifetime_hours = clamp_int(UI_DEV_TEST_FILTER_LIFETIME_HOURS,
                                      FILTER_LIFETIME_MIN_HOURS,
                                      FILTER_LIFETIME_MAX_HOURS);
    filter_used_hours = clamp_int(UI_DEV_TEST_FILTER_USED_HOURS, 0, filter_lifetime_hours);
    filter_used_seconds_total = (uint32_t)filter_used_hours * 3600U;
    refresh_filter_status_from_usage();

#if UI_DEV_TEST_FORCE_FILTER_STATUS >= 0 && UI_DEV_TEST_FORCE_FILTER_STATUS <= 2
    filter_status = (filter_status_t)UI_DEV_TEST_FORCE_FILTER_STATUS;
#endif

#if UI_DEV_TEST_FORCE_ACCENT_INDEX >= 0
    display_accent_index = clamp_int(UI_DEV_TEST_FORCE_ACCENT_INDEX, 0, DISPLAY_ACCENT_MAX_INDEX);
    if (accent_is_custom_index(display_accent_index)) {
        ui_accent_color_hex = accent_hex_from_hue(custom_accent_hue);
    } else {
        ui_accent_color_hex = accent_palette_hex[display_accent_index];
    }
    if (boot_progress_fill) {
        lv_obj_set_style_bg_color(boot_progress_fill, lv_color_hex(ui_accent_color_hex), 0);
    }
#endif
#endif
}

static void filter_usage_reset_and_save(void)
{
    filter_runtime_partial_us = 0;
    filter_runtime_unsaved_us = 0;
    filter_used_seconds_total = 0;
    filter_used_hours = 0;
    refresh_filter_status_from_usage();
    save_filter_usage_progress();
}

static void filter_usage_runtime_tick(int64_t now_us)
{
    if (now_us <= 0) {
        return;
    }

    if (filter_runtime_last_tick_us <= 0) {
        filter_runtime_last_tick_us = now_us;
        return;
    }

    int64_t dt_us = now_us - filter_runtime_last_tick_us;
    filter_runtime_last_tick_us = now_us;
    if (dt_us <= 0) {
        return;
    }

    if (fan_speed_percent <= 0 || filter_lifetime_hours <= 0) {
        return;
    }

    filter_runtime_partial_us += dt_us;
    filter_runtime_unsaved_us += dt_us;

    uint32_t delta_sec = (uint32_t)(filter_runtime_partial_us / 1000000LL);
    if (delta_sec > 0) {
        filter_runtime_partial_us -= (int64_t)delta_sec * 1000000LL;

        uint32_t max_filter_seconds = (uint32_t)filter_lifetime_hours * 3600U;
        uint32_t next_seconds = filter_used_seconds_total + delta_sec;
        if (next_seconds < filter_used_seconds_total || next_seconds > max_filter_seconds) {
            next_seconds = max_filter_seconds;
        }

        int prev_hours = filter_used_hours;
        filter_status_t prev_status = filter_status;

        filter_used_seconds_total = next_seconds;
        filter_used_hours = (int)(filter_used_seconds_total / 3600U);
        refresh_filter_status_from_usage();

        if (filter_used_hours != prev_hours || filter_status != prev_status) {
            update_main_ui();
        }
    }

    if (filter_runtime_unsaved_us >= (int64_t)FILTER_USAGE_AUTOSAVE_INTERVAL_US) {
        save_filter_usage_progress();
        filter_runtime_unsaved_us = 0;
    }
}

