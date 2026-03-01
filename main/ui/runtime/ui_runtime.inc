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
        lv_label_set_text(label_sensitivity_title, ui_tr("Sensitivity", "Чувствит."));
    }
    if (label_overdrive_title) {
        lv_label_set_text(label_overdrive_title, ui_tr("Overdrive", "ОВЕРДРАЙВ"));
    }
    if (label_purification_title) {
        lv_label_set_text(label_purification_title,
                          is_purifying ? ui_tr("PURIFYING", "ОЧИЩЕНИЕ")
                                       : ui_tr("PURIFICATION", "ОЧИСТКА"));
    }
    if (label_purification_state) {
        lv_label_set_text(label_purification_state, "");
    }
    if (label_purification_hint) {
        lv_label_set_text(label_purification_hint, ui_tr("DOUBLE PRESS TO EXIT", "ДВА НАЖАТИЯ: ВЫХОД"));
    }
    if (lock_screen_title) {
        lv_label_set_text(lock_screen_title, "");
    }
    if (lock_screen_hint) {
        lv_label_set_text(lock_screen_hint, ui_tr("PRESS TO UNLOCK", "НАЖМИТЕ: ОТКРЫТЬ"));
    }
    if (ui_screen == UI_SCREEN_SETTINGS) {
        settings_refresh_view();
    }
    update_status_row();
}

static void set_lock_overlay(bool enabled)
{
    if (!lock_screen) {
        return;
    }

    bool should_show = (ui_screen == UI_SCREEN_MAIN && !ui_sleeping && enabled);

    if (should_show) {
        lv_obj_clear_flag(lock_screen, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(lock_screen);
        if (label_fps_counter) {
            lv_obj_clear_flag(label_fps_counter, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(label_fps_counter);
        }
        lv_obj_set_style_opa(lock_screen, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_opa(lock_screen, LV_OPA_COVER, 0);
        for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
            if (lock_ring_segments[i]) {
                lv_obj_clear_flag(lock_ring_segments[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
        if (lock_screen_title) {
            lv_obj_clear_flag(lock_screen_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(lock_screen_title, LV_OPA_COVER, 0);
        }
    } else {
        lv_obj_add_flag(lock_screen, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(lock_screen, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_opa(lock_screen, LV_OPA_COVER, 0);
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
    lv_anim_set_values(&theme_anim, boot_theme_dark_blend_opa,
                       display_theme_dark ? LV_OPA_COVER : LV_OPA_TRANSP);
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

static void set_obj_opa_anim(void *obj, int32_t value)
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
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)value, 0);
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
    if (ui_locked) {
        return lv_color_hex(ui_accent_color_hex);
    }
    return display_theme_dark ? lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX) : lv_color_black();
}

static lv_color_t ui_theme_bg_color(void)
{
    return display_theme_dark ? lv_color_hex(UI_BG_HEX) : lv_color_white();
}

static lv_color_t ui_theme_text_color(void)
{
    return display_theme_dark ? lv_color_white() : lv_color_black();
}

static lv_color_t ui_theme_muted_color(void)
{
    return display_theme_dark ? lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX) : lv_color_black();
}

static const char *get_filter_status_text(void)
{
    if (filter_status == FILTER_STATUS_CRITICAL) {
        return ui_tr("REPLACE", "ЗАМЕНИТЬ");
    }
    return ui_tr("FILTER", "ФИЛЬТР");
}

static void update_status_row(void)
{
    lv_color_t filt_color = get_filter_status_color();
    const char *status = ui_tr("FILTER", "ФИЛЬТР");
    if (filter_status == FILTER_STATUS_CRITICAL) {
        status = ui_tr("REPLACE", "ЗАМЕНИТЬ");
    }

    if (status_divider) {
        lv_obj_set_style_bg_color(status_divider, filt_color, 0);
        lv_obj_set_style_bg_opa(status_divider, display_theme_dark ? (lv_opa_t)38 : (lv_opa_t)46, 0);
        lv_obj_set_style_border_color(status_divider, filt_color, 0);
        lv_obj_set_style_border_opa(status_divider, LV_OPA_COVER, 0);
    }
    if (label_filter_icon) {
        lv_obj_set_style_bg_color(label_filter_icon, filt_color, 0);
    }
    if (label_filter_status) {
        lv_label_set_text(label_filter_status, status);
        lv_obj_set_style_text_color(label_filter_status, filt_color, 0);
    }
    if (lock_screen_icon) {
        lv_obj_set_style_text_color(lock_screen_icon, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_icon_shackle) {
        lv_obj_set_style_border_color(lock_screen_icon_shackle, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_icon_body) {
        lv_obj_set_style_border_color(lock_screen_icon_body, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_icon_shackle_cut) {
        lv_obj_set_style_bg_color(lock_screen_icon_shackle_cut, lv_color_black(), 0);
    }
    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        if (lock_ring_segments[i]) {
            lv_obj_set_style_bg_color(lock_ring_segments[i], lv_color_hex(ui_accent_color_hex), 0);
        }
    }
    if (lock_ring_container) {
        lv_obj_set_style_arc_color(lock_ring_container, lv_color_hex(ui_accent_color_hex), LV_PART_MAIN);
        lv_obj_set_style_arc_color(lock_ring_container, lv_color_hex(ui_accent_color_hex), LV_PART_INDICATOR);
    }
    if (lock_screen_title) {
        lv_obj_set_style_text_color(lock_screen_title, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_hint) {
        lv_color_t hint_color = display_theme_dark ? lv_color_hex(0x6B7280) : lv_color_hex(0x6B7280);
        lv_obj_set_style_text_color(lock_screen_hint, hint_color, 0);
    }
}

static void build_segment_ring(lv_obj_t *parent)
{
    const float two_pi = 6.28318530718f;
    const lv_coord_t center_x = (EXAMPLE_LCD_H_RES / 2) + UI_SPEED_CENTER_OFFSET_X;
    const lv_coord_t center_y = (EXAMPLE_LCD_V_RES / 2) + UI_SPEED_CENTER_OFFSET_Y;
    const lv_coord_t seg_h = UI_SEGMENT_H + 4;              /* Slightly longer inward */
    const lv_coord_t ring_radius = UI_BOOT_RING_RADIUS + 3; /* Move segments 5px farther from center */

    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lv_obj_t *seg = lv_obj_create(parent);
        ring_segments[i] = seg;
        lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(seg, UI_SEGMENT_W, seg_h);
        lv_obj_set_style_radius(seg, 4, 0);
        lv_obj_set_style_border_width(seg, 0, 0);
        lv_obj_set_style_bg_color(seg, lv_color_hex(UI_BOOT_RING_INACTIVE_HEX), 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_40, 0);

        float angle = ((float)i / (float)UI_SEGMENT_COUNT) * two_pi;
        float angle_deg = ((float)i / (float)UI_SEGMENT_COUNT) * 360.0f;
        lv_coord_t x = center_x + (lv_coord_t)(sinf(angle) * ring_radius) - (UI_SEGMENT_W / 2);
        lv_coord_t y = center_y - (lv_coord_t)(cosf(angle) * ring_radius) - (seg_h / 2);
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
            lv_obj_set_style_bg_color(seg, lv_color_hex(ui_accent_color_hex), 0);
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
            lv_color_t partial_color = lv_color_mix(lv_color_hex(ui_accent_color_hex),
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
    boot_theme_dark_blend_opa = display_theme_dark ? LV_OPA_COVER : LV_OPA_TRANSP;
    set_boot_theme_mode(display_theme_dark, 0, -1);
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
    lv_obj_set_style_bg_color(screen, ui_theme_bg_color(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_text_color(screen, ui_theme_text_color(), 0);
}

static void apply_theme_now(void)
{
    const lv_color_t theme_text = ui_theme_text_color();
    const lv_color_t theme_muted = ui_theme_muted_color();
    const lv_color_t theme_secondary = display_theme_dark ? lv_color_hex(0x5D5D66) : lv_color_black();
    const lv_color_t theme_border = display_theme_dark ? lv_color_white() : lv_color_black();
    const lv_color_t theme_slider_track = display_theme_dark ? lv_color_hex(0x2D2F38) : lv_color_hex(0xD1D5DB);

    apply_dark_screen_style(boot_screen);
    set_boot_theme_mode(display_theme_dark, 0, -1);
    if (boot_subtitle) {
        set_obj_text_opa_anim(boot_subtitle, 255);
    }
    apply_dark_screen_style(main_screen);
    apply_dark_screen_style(lock_screen);
    if (lock_screen) {
        lv_obj_set_style_bg_color(lock_screen, lv_color_black(), 0);
    }
    apply_dark_screen_style(settings_screen);
    apply_dark_screen_style(language_screen);
    apply_dark_screen_style(owner_screen);
    apply_dark_screen_style(sensitivity_screen);
    apply_dark_screen_style(overdrive_screen);
    apply_dark_screen_style(sleep_screen);

    if (purification_screen) {
        lv_obj_set_style_bg_color(purification_screen, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(purification_screen, LV_OPA_COVER, 0);
    }

    if (label_speed) {
        lv_obj_set_style_text_color(label_speed, theme_text, 0);
    }
    if (label_speed_caption) {
        lv_obj_set_style_text_color(label_speed_caption, theme_secondary, 0);
    }
    if (lock_screen_icon) {
        lv_obj_set_style_text_color(lock_screen_icon, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_icon_shackle) {
        lv_obj_set_style_border_color(lock_screen_icon_shackle, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_icon_body) {
        lv_obj_set_style_border_color(lock_screen_icon_body, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_icon_shackle_cut) {
        lv_obj_set_style_bg_color(lock_screen_icon_shackle_cut, ui_theme_bg_color(), 0);
    }
    if (lock_screen_title) {
        lv_obj_set_style_text_color(lock_screen_title, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (lock_screen_hint) {
        lv_obj_set_style_text_color(lock_screen_hint, lv_color_hex(0x6B7280), 0);
    }
    if (main_bg_glow_outer && main_bg_glow_inner && main_bg_glow_core && main_center_disc) {
        if (display_theme_dark) {
            lv_obj_set_style_bg_color(main_bg_glow_outer, lv_color_hex(0x101216), 0);
            lv_obj_set_style_bg_opa(main_bg_glow_outer, (lv_opa_t)20, 0);
            lv_obj_set_style_bg_color(main_bg_glow_inner, lv_color_hex(0x1A1D22), 0);
            lv_obj_set_style_bg_opa(main_bg_glow_inner, (lv_opa_t)30, 0);
            lv_obj_set_style_bg_color(main_bg_glow_core, lv_color_hex(0x252A32), 0);
            lv_obj_set_style_bg_opa(main_bg_glow_core, (lv_opa_t)24, 0);
            lv_obj_set_style_bg_color(main_center_disc, lv_color_hex(UI_SURFACE_HEX), 0);
        } else {
            lv_obj_set_style_bg_color(main_bg_glow_outer, lv_color_white(), 0);
            lv_obj_set_style_bg_opa(main_bg_glow_outer, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(main_bg_glow_inner, lv_color_white(), 0);
            lv_obj_set_style_bg_opa(main_bg_glow_inner, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(main_bg_glow_core, lv_color_white(), 0);
            lv_obj_set_style_bg_opa(main_bg_glow_core, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(main_center_disc, lv_color_white(), 0);
        }
        lv_obj_set_style_bg_opa(main_center_disc, LV_OPA_COVER, 0);
    }
    if (label_settings_title) {
        lv_obj_set_style_text_color(label_settings_title, theme_muted, 0);
    }
    if (settings_sensitivity_title) {
        lv_obj_set_style_text_color(settings_sensitivity_title, theme_muted, 0);
    }
    if (settings_sensitivity_value) {
        lv_obj_set_style_text_color(settings_sensitivity_value, theme_text, 0);
    }
    if (settings_sensitivity_subtitle) {
        lv_obj_set_style_text_color(settings_sensitivity_subtitle, theme_secondary, 0);
    }
    if (settings_sensitivity_unit) {
        lv_obj_set_style_text_color(settings_sensitivity_unit, theme_text, 0);
    }
    if (settings_focus_frame) {
        lv_obj_set_style_border_color(settings_focus_frame, theme_border, 0);
    }
    if (settings_value_slider_track) {
        lv_obj_set_style_bg_color(settings_value_slider_track, theme_slider_track, 0);
    }
    if (settings_filter_used_label) {
        lv_obj_set_style_text_color(settings_filter_used_label, theme_secondary, 0);
    }
    if (settings_filter_used_unit) {
        lv_obj_set_style_text_color(settings_filter_used_unit, theme_secondary, 0);
    }
    if (settings_filter_used_subtitle) {
        lv_obj_set_style_text_color(settings_filter_used_subtitle, theme_secondary, 0);
    }
    if (settings_about_title) {
        lv_obj_set_style_text_color(settings_about_title, theme_muted, 0);
    }
    if (settings_about_model_caption) {
        lv_obj_set_style_text_color(settings_about_model_caption, theme_secondary, 0);
    }
    if (settings_about_model_value) {
        lv_obj_set_style_text_color(settings_about_model_value, theme_text, 0);
    }
    if (settings_about_fw_caption) {
        lv_obj_set_style_text_color(settings_about_fw_caption, theme_secondary, 0);
    }
    if (settings_about_fw_value) {
        lv_obj_set_style_text_color(settings_about_fw_value, theme_text, 0);
    }
    if (settings_about_if_caption) {
        lv_obj_set_style_text_color(settings_about_if_caption, theme_secondary, 0);
    }
    if (settings_about_if_value) {
        lv_obj_set_style_text_color(settings_about_if_value, theme_text, 0);
    }
    if (settings_v2_glow) {
        lv_obj_set_style_bg_color(settings_v2_glow, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_bg_opa(settings_v2_glow, display_theme_dark ? (lv_opa_t)26 : (lv_opa_t)18, 0);
    }
    if (settings_v2_center_circle) {
        lv_obj_set_style_bg_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_bg_grad_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_border_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_shadow_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (settings_v2_center_title) {
        lv_obj_set_style_text_color(settings_v2_center_title, theme_text, 0);
    }
    if (settings_v2_center_value) {
        lv_obj_set_style_text_color(settings_v2_center_value, theme_secondary, 0);
    }
    if (settings_v2_left_circle) {
        lv_obj_set_style_border_color(settings_v2_left_circle, display_theme_dark ? lv_color_hex(0x3F3F46) : lv_color_hex(0xA1A1AA), 0);
    }
    if (settings_v2_right_circle) {
        lv_obj_set_style_border_color(settings_v2_right_circle, display_theme_dark ? lv_color_hex(0x3F3F46) : lv_color_hex(0xA1A1AA), 0);
    }
    if (settings_v2_left_icon) {
        lv_obj_set_style_text_color(settings_v2_left_icon, display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A), 0);
    }
    if (settings_v2_right_icon) {
        lv_obj_set_style_text_color(settings_v2_right_icon, display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A), 0);
    }
    if (settings_v2_arc) {
        lv_obj_set_style_arc_color(settings_v2_arc, display_theme_dark ? lv_color_hex(0x1C1C1F) : lv_color_hex(0xD4D4D8), LV_PART_MAIN);
        lv_obj_set_style_arc_color(settings_v2_arc, lv_color_hex(ui_accent_color_hex), LV_PART_INDICATOR);
    }
    if (settings_v2_arc_icon) {
        lv_obj_set_style_text_color(settings_v2_arc_icon, lv_color_hex(ui_accent_color_hex), 0);
    }
    if (settings_v2_arc_title) {
        lv_obj_set_style_text_color(settings_v2_arc_title, theme_muted, 0);
    }
    if (settings_v2_arc_value) {
        lv_obj_set_style_text_color(settings_v2_arc_value, theme_text, 0);
    }
    if (settings_v2_arc_suffix) {
        lv_obj_set_style_text_color(settings_v2_arc_suffix, theme_secondary, 0);
    }
    if (settings_v2_arc_subtitle) {
        lv_obj_set_style_text_color(settings_v2_arc_subtitle, theme_secondary, 0);
    }

    update_status_row();
    update_main_ui();
    update_settings_wheel_layout();
}

static void apply_display_rotation_now(void)
{
    if (!lcd_panel) {
        return;
    }

    int rotation_deg = display_rotation_deg % 360;
    if (rotation_deg < 0) {
        rotation_deg += 360;
    }

    /* CO5300: keep UI geometry unchanged and rotate in panel MADCTL only.
     * Supported UI setting here is 0/180, so map everything else to 0. */
    rotation_deg = (rotation_deg == 180) ? 180 : 0;
    display_rotation_deg = rotation_deg;

    const bool invert_180 = (rotation_deg == 180);
    uint8_t madctl = invert_180 ? 0xC0 : 0x00; /* MY|MX for 180deg on CO5300 */

    if (lcd_io) {
        /* SH8601/CO5300 QSPI command format:
         * Instruction 0x02 + Address {0x00, CMD, 0x00}. */
        const int madctl_qspi_cmd = (int)((0x02U << 24) | (0x36U << 8));
        esp_err_t io_err = esp_lcd_panel_io_tx_param(lcd_io, madctl_qspi_cmd, &madctl, 1);
        if (io_err != ESP_OK) {
            ESP_LOGW(TAG, "panel MADCTL write failed: %s", esp_err_to_name(io_err));
        } else {
            ESP_LOGI(TAG, "panel MADCTL applied: rot=%d madctl=0x%02X", rotation_deg, madctl);
        }
    }

    /* CO5300 visible window has different X offset after 180deg flip. */
    esp_err_t gap_err = esp_lcd_panel_set_gap(lcd_panel,
                                              invert_180 ? CO5300_X_GAP_ROT_180 : CO5300_X_GAP_ROT_0,
                                              0);
    if (gap_err != ESP_OK) {
        ESP_LOGW(TAG, "panel gap set failed: %s", esp_err_to_name(gap_err));
    }

    if (lvgl_disp) {
        lv_obj_t *active = lv_screen_active();
        if (active) {
            lv_obj_invalidate(active);
        }
    }
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

static void ui_set_refresh_period_locked(uint32_t period_ms)
{
    if (!lvgl_disp) {
        return;
    }
    if (ui_refr_current_period_ms == period_ms) {
        return;
    }

    lv_timer_t *refr_timer = lv_display_get_refr_timer(lvgl_disp);
    if (!refr_timer) {
        return;
    }

    lv_timer_set_period(refr_timer, period_ms);
    lv_timer_ready(refr_timer);
    ui_refr_current_period_ms = period_ms;
}

static void ui_mark_activity(void)
{
    ui_refr_active_until_us = esp_timer_get_time() + (int64_t)UI_REFR_ACTIVE_HOLD_US;
    ui_set_refresh_period_locked(UI_REFR_ACTIVE_PERIOD_MS);
}

static void ui_refresh_rate_tick(void)
{
    const int64_t now_us = esp_timer_get_time();
    filter_usage_runtime_tick(now_us);

    if (ui_screen == UI_SCREEN_SETTINGS) {
        bool keep_settings_active = settings_root_scroll_animating || (now_us < ui_refr_active_until_us);
        uint32_t settings_period_ms = keep_settings_active ? UI_REFR_SETTINGS_ACTIVE_PERIOD_MS
                                                           : UI_REFR_SETTINGS_IDLE_PERIOD_MS;
        ui_set_refresh_period_locked(settings_period_ms);
        return;
    }

    const bool overdrive_badge_visible = label_overdrive_warning &&
                                         !lv_obj_has_flag(label_overdrive_warning, LV_OBJ_FLAG_HIDDEN);
    const bool keep_active = (ui_screen == UI_SCREEN_BOOT) ||
                             (ui_screen == UI_SCREEN_PURIFICATION) ||
                             settings_root_scroll_animating ||
                             overdrive_badge_visible ||
                             (now_us < ui_refr_active_until_us);
    uint32_t period_ms = keep_active ? UI_REFR_ACTIVE_PERIOD_MS : UI_REFR_IDLE_PERIOD_MS;
    if (ui_screen == UI_SCREEN_PURIFICATION && period_ms < 20U) {
        period_ms = 20U;
    }
    ui_set_refresh_period_locked(period_ms);
}

static void restart_current_screen_after_theme_change(void)
{
    lv_obj_t *screen = get_screen_by_id(ui_screen);
    show_screen(screen);

    if (ui_screen == UI_SCREEN_MAIN) {
        set_lock_overlay(ui_locked);
        update_main_ui();
    } else if (ui_screen == UI_SCREEN_SETTINGS) {
        settings_refresh_view();
    } else if (ui_screen == UI_SCREEN_PURIFICATION) {
        apply_language();
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
        ui_mark_activity();
        if (lock_screen && screen != main_screen) {
            lv_obj_add_flag(lock_screen, LV_OBJ_FLAG_HIDDEN);
        }
        if (label_fps_counter) {
            if (screen == main_screen || screen == settings_screen) {
                lv_obj_clear_flag(label_fps_counter, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(label_fps_counter);
            } else {
                lv_obj_add_flag(label_fps_counter, LV_OBJ_FLAG_HIDDEN);
            }
        }
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

