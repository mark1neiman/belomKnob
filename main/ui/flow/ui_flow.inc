static void ui_init(void)
{
    create_boot_screen();
    create_main_screen();
    create_lock_screen();
    create_settings_screen();
    create_language_screen();
    create_owner_screen();
    create_sensitivity_screen();
    create_overdrive_screen();
    create_purification_screen();
    create_sleep_screen();
    apply_theme_now();
    fps_counter_create();
    ui_mark_activity();

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
    purification_ring_reset_animation();
    ui_screen = UI_SCREEN_PURIFICATION;
    show_screen(purification_screen);
    if (purification_ring_timer) {
        lv_timer_resume(purification_ring_timer);
    }
}

static void stop_purification_mode(bool go_to_main)
{
    is_purifying = false;
    if (purification_ring_timer) {
        lv_timer_pause(purification_ring_timer);
    }
    if (go_to_main) {
        ui_screen = UI_SCREEN_MAIN;
        show_screen(main_screen);
        set_lock_overlay(ui_locked);
        update_main_ui();
    }
}

static void handle_double_click(void)
{
    if (ui_sleeping) {
        return;
    }
    reset_sleep_timer();
    ui_mark_activity();
    if (is_purifying) {
        stop_purification_mode(true);
    }
}

static bool knob_direction_glitch_filtered(int direction, int64_t now_us)
{
    static int last_dir = 0;
    static int64_t last_event_us = 0;
    static uint8_t same_dir_streak = 0;
    if (direction == 0) {
        return false;
    }
    int dir = (direction > 0) ? 1 : -1;

    int64_t dt_us = now_us - last_event_us;
    if (last_event_us == 0 || dt_us > SETTINGS_KNOB_DIR_STREAK_RESET_US) {
        same_dir_streak = 0;
    }

    bool opposite_bounce = (last_dir != 0) &&
                           (dir == -last_dir) &&
                           (same_dir_streak >= SETTINGS_KNOB_DIR_STREAK_MIN) &&
                           (dt_us > 0) &&
                           (dt_us <= SETTINGS_KNOB_DIR_GLITCH_FILTER_US);

    if (dir == last_dir) {
        if (same_dir_streak < 255) {
            same_dir_streak++;
        }
    } else {
        same_dir_streak = 1;
    }

    last_dir = dir;
    last_event_us = now_us;
    return opposite_bounce;
}

static void handle_knob_move(int direction)
{
    static int settings_knob_pending_delta = 0;

    if (ui_sleeping) {
        return;
    }

    /* Ignore encoder noise during boot to keep startup transition deterministic. */
    if (ui_screen == UI_SCREEN_BOOT) {
        return;
    }

    int64_t now_us = esp_timer_get_time();
    if (knob_direction_glitch_filtered(direction, now_us)) {
        return;
    }

    reset_sleep_timer();

    if (ui_screen != UI_SCREEN_SETTINGS) {
        settings_knob_pending_delta = 0;
    }

    if (ui_screen == UI_SCREEN_MAIN) {
        if (ui_locked) {
            return;
        }
        if ((now_us - main_knob_last_update_us) < 2500) {
            return;
        }

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
        ui_mark_activity();
        update_main_ui();
        lvgl_port_unlock();
        main_knob_last_update_us = now_us;
        apply_fan_pwm(fan_speed_percent);
        return;
    }

    if (ui_screen == UI_SCREEN_SETTINGS) {
        settings_knob_pending_delta += direction;
        if ((now_us - settings_knob_last_update_us) < SETTINGS_KNOB_MIN_INTERVAL_US) {
            return;
        }
        int delta = settings_knob_pending_delta;
        if (delta == 0) {
            settings_knob_last_update_us = now_us;
            return;
        }
        settings_knob_pending_delta = 0;
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            settings_knob_pending_delta += delta;
            ESP_LOGW(TAG, "LVGL lock timeout in handle_knob_move(settings)");
            return;
        }
        ui_mark_activity();
        if (settings_edit_mode) {
            settings_adjust_edited_value(delta);
        } else {
            int count = settings_items_count > 0 ? settings_items_count : settings_get_item_count();
            if (count <= 0) {
                lvgl_port_unlock();
                return;
            }
            int prev_index = settings_index;
            settings_v2_nav_dir = (delta > 0) ? 1 : -1;
            settings_index = clamp_int(settings_index + delta, 0, count - 1);

            if (settings_index == prev_index) {
                settings_v2_nav_dir = 0;
                lvgl_port_unlock();
                return;
            }

            settings_sync_root_scroll_to_index();
            update_settings_wheel_layout();
        }
        lvgl_port_unlock();
        settings_knob_last_update_us = now_us;
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
        ui_mark_activity();
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
        ui_mark_activity();
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
        ui_mark_activity();
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
        ui_mark_activity();
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
    } else if (strcmp(buf, "Save") == 0 || strcmp(buf, "Сохранить") == 0) {
        save_owner_name();
        ui_screen = UI_SCREEN_SETTINGS;
        show_screen(settings_screen);
        apply_language();
        update_settings_selection();
        return;
    } else {
        char ch = 0;
        if (strcmp(buf, "space") == 0 || strcmp(buf, "пробел") == 0) {
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
    ui_mark_activity();

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
    ui_mark_activity();

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

