static bool settings_v2_is_swatch_view(settings_view_t view)
{
    return (view == SETTINGS_VIEW_ACCENT ||
            view == SETTINGS_VIEW_LANGUAGE ||
            view == SETTINGS_VIEW_THEME ||
            view == SETTINGS_VIEW_ROTATION ||
            view == SETTINGS_VIEW_LOCK_MODE);
}

static bool settings_v2_is_arc_view(void)
{
    bool filter_lifetime_panel_view = (settings_view == SETTINGS_VIEW_FILTER) &&
                                      settings_edit_mode &&
                                      (settings_index == 0);
    return (settings_view == SETTINGS_VIEW_SENSITIVITY) ||
           (settings_view == SETTINGS_VIEW_BRIGHTNESS) ||
           filter_lifetime_panel_view;
}

static bool settings_v2_is_enabled_view(settings_view_t view)
{
    bool filter_lifetime_panel_view = (view == SETTINGS_VIEW_FILTER) &&
                                      settings_edit_mode &&
                                      (settings_index == 0);
    return (view == SETTINGS_VIEW_ROOT) ||
           settings_v2_is_swatch_view(view) ||
           (view == SETTINGS_VIEW_SENSITIVITY) ||
           (view == SETTINGS_VIEW_BRIGHTNESS) ||
           (view == SETTINGS_VIEW_OVERDRIVE_WARNING) ||
           (view == SETTINGS_VIEW_OVERDRIVE_MODE) ||
           (view == SETTINGS_VIEW_PURIFICATION_CONFIRM) ||
           (view == SETTINGS_VIEW_FILTER) ||
           (view == SETTINGS_VIEW_FILTER_RESET) ||
           filter_lifetime_panel_view;
}

static void settings_v2_set_translate_x_anim(void *obj, int32_t value)
{
    if (!obj) {
        return;
    }
    lv_obj_set_style_translate_x((lv_obj_t *)obj, value, 0);
}

static void settings_v2_play_slide_anim(lv_obj_t *obj, int direction)
{
    if (!obj || direction == 0) {
        return;
    }
    int32_t from = (direction > 0) ? SETTINGS_V2_SLIDE_SHIFT_PX : -SETTINGS_V2_SLIDE_SHIFT_PX;
    lv_obj_set_style_translate_x(obj, from, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, settings_v2_set_translate_x_anim);
    lv_anim_set_values(&a, from, 0);
    lv_anim_set_time(&a, SETTINGS_V2_SLIDE_TIME_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

static void settings_v2_play_transition(void)
{
    if (!settings_v2_layer || settings_v2_transition_mode == 0) {
        return;
    }
    /* Keep transitions disabled for now to avoid black-frame artifacts on target HW. */
    lv_obj_set_style_transform_zoom(settings_v2_layer, 256, 0);
    lv_obj_set_style_opa(settings_v2_layer, LV_OPA_COVER, 0);
    settings_v2_transition_mode = 0;
}

static const ui_lang_t settings_v2_language_order[SETTINGS_LANG_OPTIONS_COUNT] = {
    LANG_EN,
    LANG_ET,
    LANG_RU,
    LANG_LV,
    LANG_FI,
    LANG_DE,
    LANG_ES,
    LANG_FR,
    LANG_UK,
    LANG_PL,
    LANG_LT,
    LANG_IT,
};

static int settings_v2_language_index_from_lang(ui_lang_t lang)
{
    for (int i = 0; i < SETTINGS_LANG_OPTIONS_COUNT; ++i) {
        if (settings_v2_language_order[i] == lang) {
            return i;
        }
    }
    return 0;
}

static int settings_v2_selected_index_for_view(settings_view_t view)
{
    switch (view) {
    case SETTINGS_VIEW_ACCENT:
        return display_accent_index;
    case SETTINGS_VIEW_LANGUAGE:
        return settings_v2_language_index_from_lang(current_lang);
    case SETTINGS_VIEW_THEME:
        return display_theme_dark ? 0 : 1;
    case SETTINGS_VIEW_ROTATION:
        return ((display_rotation_deg % 360) == 180) ? 1 : 0;
    case SETTINGS_VIEW_LOCK_MODE:
        return allow_child_lock ? 0 : 1;
    default:
        return -1;
    }
}

static const char *settings_v2_lang_code(int index)
{
    switch (index) {
    case 0: return "EN";
    case 1: return "ET";
    case 2: return "RU";
    case 3: return "LV";
    case 4: return "FI";
    case 5: return "DE";
    case 6: return "ES";
    case 7: return "FR";
    case 8: return "UK";
    case 9: return "PL";
    case 10: return "LT";
    case 11: return "IT";
    default: return "";
    }
}

static void settings_v2_update_dots(int count, int active)
{
    if (count < 1) {
        count = 1;
    }
    if (count > SETTINGS_V2_DOTS_COUNT) {
        count = SETTINGS_V2_DOTS_COUNT;
    }
    active = clamp_int(active, 0, count - 1);

    bool accent_wide = (settings_view == SETTINGS_VIEW_ACCENT);
    int window = accent_wide ? 11 : 7;
    if (window > count) {
        window = count;
    }
    int half = window / 2;
    int start = active - half;
    if (start < 0) {
        start = 0;
    }
    if (start > (count - window)) {
        start = count - window;
    }
    if (start < 0) {
        start = 0;
    }
    int end = start + window - 1;
    if (end > (count - 1)) {
        end = count - 1;
    }

    int visible = end - start + 1;
    int spacing = 20;
    int center_x0 = (EXAMPLE_LCD_H_RES / 2) - ((visible - 1) * spacing) / 2;
    int center_y = EXAMPLE_LCD_V_RES - 53;
    for (int i = 0; i < SETTINGS_V2_DOTS_COUNT; ++i) {
        lv_obj_t *dot = settings_v2_dots[i];
        if (!dot) {
            continue;
        }
        if (i < start || i > end || i >= count) {
            lv_obj_add_flag(dot, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        int pos = i - start;
        bool is_active = (i == active);
        int dot_size = accent_wide ? (is_active ? 11 : 9) : (is_active ? 12 : 10);
        lv_obj_set_size(dot, dot_size, dot_size);
        lv_obj_set_style_bg_color(dot, is_active ? lv_color_hex(ui_accent_color_hex) : lv_color_hex(0x71717A), 0);
        if (accent_wide) {
            lv_obj_set_style_bg_opa(dot, is_active ? LV_OPA_COVER : ((abs(i - active) > 3) ? (lv_opa_t)100 : (lv_opa_t)170), 0);
        } else {
            lv_obj_set_style_bg_opa(dot, is_active ? LV_OPA_COVER : ((abs(i - active) > 2) ? (lv_opa_t)100 : (lv_opa_t)170), 0);
        }
        int center_x = center_x0 + pos * spacing;
        lv_obj_set_pos(dot, center_x - (dot_size / 2), center_y - (dot_size / 2));
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_HIDDEN);
    }
}

static void settings_v2_hide_legacy_widgets(void)
{
    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        if (settings_items[i]) {
            lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (settings_focus_frame) {
        lv_obj_add_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_value_slider_track) {
        lv_obj_add_flag(settings_value_slider_track, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_value_slider_fill) {
        lv_obj_add_flag(settings_value_slider_fill, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_value_slider_knob) {
        lv_obj_add_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_sensitivity_title) {
        lv_obj_add_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_sensitivity_value) {
        lv_obj_add_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_sensitivity_subtitle) {
        lv_obj_add_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_sensitivity_unit) {
        lv_obj_add_flag(settings_sensitivity_unit, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_filter_used_arc) {
        lv_obj_add_flag(settings_filter_used_arc, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_filter_used_label) {
        lv_obj_add_flag(settings_filter_used_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_filter_used_value) {
        lv_obj_add_flag(settings_filter_used_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_filter_used_unit) {
        lv_obj_add_flag(settings_filter_used_unit, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_filter_used_subtitle) {
        lv_obj_add_flag(settings_filter_used_subtitle, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_title) {
        lv_obj_add_flag(settings_about_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_model_caption) {
        lv_obj_add_flag(settings_about_model_caption, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_model_value) {
        lv_obj_add_flag(settings_about_model_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_fw_caption) {
        lv_obj_add_flag(settings_about_fw_caption, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_fw_value) {
        lv_obj_add_flag(settings_about_fw_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_if_caption) {
        lv_obj_add_flag(settings_about_if_caption, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_about_if_value) {
        lv_obj_add_flag(settings_about_if_value, LV_OBJ_FLAG_HIDDEN);
    }
}

static void settings_custom_hue_widgets_set_hidden(bool hidden)
{
#if !UI_ENABLE_CUSTOM_ACCENT
    (void)hidden;
    settings_custom_hue_ui_ready = false;
    return;
#endif

    bool need_update = false;
    for (int i = 0; i < SETTINGS_CUSTOM_HUE_SEGMENT_COUNT; ++i) {
        if (!settings_custom_hue_ring_segments[i]) {
            continue;
        }
        bool is_hidden = lv_obj_has_flag(settings_custom_hue_ring_segments[i], LV_OBJ_FLAG_HIDDEN);
        if (is_hidden != hidden) {
            need_update = true;
        }
        break;
    }
    if (!need_update && settings_custom_hue_knob) {
        bool knob_hidden = lv_obj_has_flag(settings_custom_hue_knob, LV_OBJ_FLAG_HIDDEN);
        if (knob_hidden != hidden) {
            need_update = true;
        }
    }
    if (!need_update) {
        return;
    }

    for (int i = 0; i < SETTINGS_CUSTOM_HUE_SEGMENT_COUNT; ++i) {
        if (!settings_custom_hue_ring_segments[i]) {
            continue;
        }
        if (hidden) {
            lv_obj_add_flag(settings_custom_hue_ring_segments[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(settings_custom_hue_ring_segments[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (settings_custom_hue_knob) {
        if (hidden) {
            lv_obj_add_flag(settings_custom_hue_knob, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(settings_custom_hue_knob, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (hidden) {
        settings_custom_hue_ui_ready = false;
    }
}

static void settings_v2_hide_layer(void)
{
    if (settings_v2_layer) {
        lv_obj_add_flag(settings_v2_layer, LV_OBJ_FLAG_HIDDEN);
    }
    settings_custom_hue_widgets_set_hidden(true);
    settings_v2_theme_cache_valid = false;
    settings_v2_root_cache_valid = false;
    settings_v2_filter_btn_last_active = -1;
    settings_v2_filter_confirm_last_active = -1;
}

static void settings_v2_apply_theme(void)
{
    if (!settings_v2_layer) {
        return;
    }
    if (settings_v2_theme_cache_valid &&
        settings_v2_theme_cache_dark == display_theme_dark &&
        settings_v2_theme_cache_accent_hex == ui_accent_color_hex) {
        return;
    }

    lv_color_t primary = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
    lv_color_t secondary = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);
    lv_color_t ghost_bg = display_theme_dark ? lv_color_white() : lv_color_black();
    lv_color_t ghost_border = display_theme_dark ? lv_color_hex(0x3F3F46) : lv_color_hex(0xA1A1AA);
    if (settings_v2_center_title) {
        lv_obj_set_style_text_color(settings_v2_center_title, primary, 0);
    }
    if (settings_v2_center_value) {
        lv_obj_set_style_text_color(settings_v2_center_value, secondary, 0);
    }
    if (settings_v2_left_circle) {
        lv_obj_set_style_bg_color(settings_v2_left_circle, ghost_bg, 0);
        lv_obj_set_style_bg_opa(settings_v2_left_circle, display_theme_dark ? (lv_opa_t)22 : (lv_opa_t)16, 0);
        lv_obj_set_style_border_color(settings_v2_left_circle, ghost_border, 0);
    }
    if (settings_v2_right_circle) {
        lv_obj_set_style_bg_color(settings_v2_right_circle, ghost_bg, 0);
        lv_obj_set_style_bg_opa(settings_v2_right_circle, display_theme_dark ? (lv_opa_t)22 : (lv_opa_t)16, 0);
        lv_obj_set_style_border_color(settings_v2_right_circle, ghost_border, 0);
    }
    if (settings_v2_glow) {
#if SETTINGS_V2_ENABLE_GLOW
        lv_obj_set_style_bg_color(settings_v2_glow, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_bg_grad_color(settings_v2_glow, ui_theme_bg_color(), 0);
#if LV_USE_DRAW_SW_COMPLEX_GRADIENTS
        lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_RADIAL, 0);
        lv_obj_set_style_bg_opa(settings_v2_glow, display_theme_dark ? (lv_opa_t)28 : (lv_opa_t)20, 0);
#else
        lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_NONE, 0);
        lv_obj_set_style_bg_opa(settings_v2_glow, LV_OPA_TRANSP, 0);
#endif
#else
        lv_obj_add_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
#endif
    }

    settings_v2_theme_cache_valid = true;
    settings_v2_theme_cache_dark = display_theme_dark;
    settings_v2_theme_cache_accent_hex = ui_accent_color_hex;
}

static bool settings_v2_render_layout(void)
{
    if (!settings_v2_layer) {
        return false;
    }
    bool v2_view = settings_v2_is_enabled_view(settings_view);
    if (!v2_view) {
        settings_v2_hide_layer();
        return false;
    }

    settings_v2_hide_legacy_widgets();
    settings_v2_apply_theme();
    lv_obj_clear_flag(settings_v2_layer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(settings_v2_layer);
    settings_v2_play_transition();
    if (label_settings_title) {
        lv_obj_add_flag(label_settings_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_dim_overlay) {
        lv_obj_add_flag(settings_dim_overlay, LV_OBJ_FLAG_HIDDEN);
    }
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

    if (settings_v2_arc) {
        lv_obj_add_flag(settings_v2_arc, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_arc_icon) {
        lv_obj_add_flag(settings_v2_arc_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_arc_title) {
        lv_obj_add_flag(settings_v2_arc_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_arc_value) {
        lv_obj_add_flag(settings_v2_arc_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_arc_suffix) {
        lv_obj_add_flag(settings_v2_arc_suffix, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_arc_subtitle) {
        lv_obj_add_flag(settings_v2_arc_subtitle, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_center_circle) {
        lv_obj_add_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_center_title) {
        lv_obj_add_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_center_value) {
        lv_obj_add_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_center_icon) {
        lv_obj_clear_flag(settings_v2_center_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_left_circle) {
        lv_obj_add_flag(settings_v2_left_circle, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_v2_right_circle) {
        lv_obj_add_flag(settings_v2_right_circle, LV_OBJ_FLAG_HIDDEN);
    }
    for (int i = 0; i < SETTINGS_V2_DOTS_COUNT; ++i) {
        if (settings_v2_dots[i]) {
            lv_obj_add_flag(settings_v2_dots[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    settings_custom_hue_widgets_set_hidden(true);

    if (settings_view == SETTINGS_VIEW_FILTER && !settings_edit_mode) {
        int total = (filter_lifetime_hours > 0) ? filter_lifetime_hours : 1;
        int used = clamp_int(filter_used_hours, 0, total);
        int remaining = total - used;
        int remaining_percent = (remaining * 100 + (total / 2)) / total;
        remaining_percent = clamp_int(remaining_percent, 0, 100);
        int arc_pos = remaining_percent * 10;
        bool reset_confirm = settings_filter_reset_confirm_open;
        int active = clamp_int(settings_index, 0, reset_confirm ? 1 : 2);

        const lv_color_t primary = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
        const lv_color_t secondary = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);
        const lv_color_t btn_border_idle = display_theme_dark ? lv_color_hex(0x52525B) : lv_color_hex(0xA1A1AA);
        const lv_color_t btn_icon_idle = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);
        const lv_color_t back_sel_bg = display_theme_dark ? lv_color_hex(0xE4E4E7) : lv_color_hex(0x18181B);
        const lv_color_t back_sel_icon = display_theme_dark ? lv_color_hex(0x18181B) : lv_color_white();
        lv_color_t indicator = lv_color_hex(ui_accent_color_hex);

        if (settings_dim_overlay) {
            if (reset_confirm) {
                lv_obj_clear_flag(settings_dim_overlay, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_bg_color(settings_dim_overlay, lv_color_black(), 0);
                lv_obj_set_style_bg_opa(settings_dim_overlay, (lv_opa_t)120, 0);
                lv_obj_move_foreground(settings_dim_overlay);
            } else {
                lv_obj_add_flag(settings_dim_overlay, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (reset_confirm) {
            if (settings_v2_glow) {
                lv_obj_add_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_add_flag(settings_v2_arc, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_v2_arc_icon, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_v2_arc_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_v2_arc_suffix, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_v2_arc_subtitle, LV_OBJ_FLAG_HIDDEN);

            if (settings_overlay_icon_bg) {
                lv_obj_clear_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_overlay_icon_bg, 156, 156);
                lv_obj_align(settings_overlay_icon_bg, LV_ALIGN_TOP_MID, 0, 44);
                lv_obj_set_style_radius(settings_overlay_icon_bg, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_overlay_icon_bg, display_theme_dark ? lv_color_hex(0x18181B) : lv_color_hex(0xF4F4F5), 0);
                lv_obj_set_style_bg_opa(settings_overlay_icon_bg, LV_OPA_COVER, 0);
                lv_obj_set_style_border_width(settings_overlay_icon_bg, 3, 0);
                lv_obj_set_style_border_color(settings_overlay_icon_bg,
                                              display_theme_dark ? lv_color_hex(0x27272A) : lv_color_hex(0xD4D4D8),
                                              0);
                lv_obj_set_style_border_opa(settings_overlay_icon_bg, LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_overlay_icon_bg);
            }
            if (settings_overlay_icon) {
                lv_label_set_text(settings_overlay_icon, UI_ICON_LUCIDE_REFRESH_CW);
                lv_obj_set_style_text_font(settings_overlay_icon, &lv_font_lucide_64_filter_reset_icons, 0);
                lv_obj_set_style_text_color(settings_overlay_icon, lv_color_hex(ui_accent_color_hex), 0);
                lv_obj_center(settings_overlay_icon);
            }
            if (settings_overlay_title) {
                lv_obj_clear_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(settings_overlay_title, ui_tr("RESET FILTER USAGE", "СБРОС РЕСУРСА ФИЛЬТРА"));
                lv_obj_set_style_text_font(settings_overlay_title, &lv_font_inter_semibold_28_ui, 0);
                lv_obj_set_style_text_letter_space(settings_overlay_title, 1, 0);
                lv_obj_set_style_text_color(settings_overlay_title, primary, 0);
                lv_obj_set_style_text_align(settings_overlay_title, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_width(settings_overlay_title, 360);
                lv_obj_align(settings_overlay_title, LV_ALIGN_TOP_MID, 0, 234);
                lv_obj_move_foreground(settings_overlay_title);
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

            static const char *filter_confirm_icons[2] = {
                UI_ICON_LUCIDE_CHEVRON_LEFT,
                UI_ICON_LUCIDE_REFRESH_CW,
            };
            for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
                if (!settings_items[i]) {
                    continue;
                }
                if (i >= 2) {
                    lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                    continue;
                }

                bool selected = (i == active);
                lv_color_t btn_bg = lv_color_hex(0x000000);
                lv_opa_t btn_bg_opa = LV_OPA_TRANSP;
                lv_color_t btn_border = btn_border_idle;
                lv_color_t btn_icon = btn_icon_idle;
                if (selected) {
                    if (i == 0) {
                        btn_bg = back_sel_bg;
                        btn_bg_opa = LV_OPA_COVER;
                        btn_border = back_sel_bg;
                        btn_icon = back_sel_icon;
                    } else {
                        btn_bg = lv_color_hex(0xDC2626);
                        btn_bg_opa = LV_OPA_COVER;
                        btn_border = lv_color_hex(0xDC2626);
                        btn_icon = lv_color_white();
                    }
                }

                lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_items[i], 92, 92);
                lv_obj_align(settings_items[i], LV_ALIGN_CENTER, (i == 0) ? -52 : 52, 112);
                lv_obj_set_style_radius(settings_items[i], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_opa(settings_items[i], LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(settings_items[i], 0, 0);
                lv_obj_set_style_clip_corner(settings_items[i], false, 0);
                lv_obj_set_style_pad_all(settings_items[i], 0, 0);
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);

                if (settings_item_icon_bgs[i]) {
                    lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_size(settings_item_icon_bgs[i], 84, 84);
                    lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                    lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
                    lv_obj_set_style_bg_color(settings_item_icon_bgs[i], btn_bg, 0);
                    lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], btn_bg_opa, 0);
                    lv_obj_set_style_border_width(settings_item_icon_bgs[i], 2, 0);
                    lv_obj_set_style_border_color(settings_item_icon_bgs[i], btn_border, 0);
                    lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
                    lv_obj_set_style_clip_corner(settings_item_icon_bgs[i], false, 0);
                    lv_obj_set_style_transform_zoom(settings_item_icon_bgs[i], 256, 0);
                    lv_obj_move_foreground(settings_item_icon_bgs[i]);
                }
                if (settings_item_icons[i]) {
                    lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_lucide_64_filter_reset_icons, 0);
                    lv_obj_set_style_transform_zoom(settings_item_icons[i], 256, 0);
                    lv_obj_set_style_text_color(settings_item_icons[i], btn_icon, 0);
                    lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                    lv_label_set_text(settings_item_icons[i], filter_confirm_icons[i]);
                    lv_obj_set_width(settings_item_icons[i], 88);
                    lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                    lv_obj_move_foreground(settings_item_icons[i]);
                }
                if (settings_item_titles[i]) {
                    lv_obj_add_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
                }
                if (settings_item_values[i]) {
                    lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
                }
                if (settings_item_arrows[i]) {
                    lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
                }
                lv_obj_move_foreground(settings_items[i]);
            }

            settings_v2_filter_confirm_last_active = active;
            settings_v2_filter_btn_last_active = -1;
            settings_v2_last_index = active;
            settings_v2_nav_dir = 0;
            return true;
        }

        if (settings_v2_glow) {
            lv_obj_add_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_clear_flag(settings_v2_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(settings_v2_arc, 438, 438);
        lv_obj_align(settings_v2_arc, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_arc_width(settings_v2_arc, 12, LV_PART_MAIN);
        lv_obj_set_style_arc_width(settings_v2_arc, 12, LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(settings_v2_arc, true, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(settings_v2_arc, true, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(settings_v2_arc,
                                   display_theme_dark ? lv_color_hex(0x1C1C1F) : lv_color_hex(0xD4D4D8),
                                   LV_PART_MAIN);
        lv_obj_set_style_arc_color(settings_v2_arc, indicator, LV_PART_INDICATOR);
        lv_arc_set_value(settings_v2_arc, arc_pos);

        lv_obj_add_flag(settings_v2_arc_title, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(settings_v2_arc_icon, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(settings_v2_arc_value, LV_OBJ_FLAG_HIDDEN);
        char percent_buf[16];
        snprintf(percent_buf, sizeof(percent_buf), "%d", remaining_percent);
        lv_label_set_text(settings_v2_arc_value, percent_buf);
        lv_obj_set_style_text_font(settings_v2_arc_value, &lv_font_inter_medium_145_num, 0);
        lv_obj_set_style_transform_zoom(settings_v2_arc_value, 256, 0);
        lv_obj_set_style_text_letter_space(settings_v2_arc_value, -2, 0);
        lv_obj_set_style_text_color(settings_v2_arc_value, primary, 0);
        lv_obj_align(settings_v2_arc_value, LV_ALIGN_CENTER, 0, -72);

        lv_obj_clear_flag(settings_v2_arc_suffix, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(settings_v2_arc_suffix, "%");
        lv_obj_set_style_text_font(settings_v2_arc_suffix, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(settings_v2_arc_suffix, primary, 0);
        lv_obj_set_style_text_letter_space(settings_v2_arc_suffix, -1, 0);
        lv_obj_update_layout(settings_v2_arc_value);
        lv_coord_t px = lv_obj_get_x(settings_v2_arc_value);
        lv_coord_t pw = lv_obj_get_width(settings_v2_arc_value);
        lv_coord_t py = lv_obj_get_y(settings_v2_arc_value);
        lv_obj_set_pos(settings_v2_arc_suffix, px + pw + 4, py + 20);

        lv_obj_clear_flag(settings_v2_arc_subtitle, LV_OBJ_FLAG_HIDDEN);
        char usage_buf[32];
        snprintf(usage_buf, sizeof(usage_buf), ui_tr("%dh / %dh", "%dч / %dч"), used, total);
        lv_label_set_text(settings_v2_arc_subtitle, usage_buf);
        lv_obj_set_style_text_font(settings_v2_arc_subtitle, &lv_font_inter_semibold_28_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_arc_subtitle, 0, 0);
        lv_obj_set_style_text_color(settings_v2_arc_subtitle, secondary, 0);
        lv_obj_align(settings_v2_arc_subtitle, LV_ALIGN_CENTER, 0, 22);

        static const char *filter_btn_icons[3] = {
            UI_ICON_LUCIDE_CLOCK_3,
            UI_ICON_LUCIDE_REFRESH_CW,
            UI_ICON_LUCIDE_CHEVRON_LEFT,
        };
        for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
            if (!settings_items[i]) {
                continue;
            }
            if (i >= 3) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            bool selected = (i == active);
            lv_color_t btn_bg = lv_color_hex(0x000000);
            lv_opa_t btn_bg_opa = LV_OPA_TRANSP;
            lv_color_t btn_border = btn_border_idle;
            lv_color_t btn_icon = btn_icon_idle;
            if (selected) {
                if (i == 2) {
                    btn_bg = back_sel_bg;
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = lv_color_hex(ui_accent_color_hex);
                    btn_icon = back_sel_icon;
                } else {
                    btn_bg = lv_color_hex(ui_accent_color_hex);
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = lv_color_hex(ui_accent_color_hex);
                    btn_icon = lv_color_white();
                }
            }

            lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_items[i], 92, 92);
            lv_obj_align(settings_items[i], LV_ALIGN_CENTER, (i - 1) * 96, 110);
            lv_obj_set_style_radius(settings_items[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(settings_items[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(settings_items[i], 0, 0);
            lv_obj_set_style_clip_corner(settings_items[i], false, 0);
            lv_obj_set_style_pad_all(settings_items[i], 0, 0);
            lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], 84, 84);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], btn_bg, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], btn_bg_opa, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 2, 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], btn_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
                lv_obj_set_style_clip_corner(settings_item_icon_bgs[i], false, 0);
                lv_obj_set_style_transform_zoom(settings_item_icon_bgs[i], 256, 0);
            }
            if (settings_item_icons[i]) {
                lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_lucide_40_filter_btn_icons, 0);
                lv_obj_set_style_transform_zoom(settings_item_icons[i], 256, 0);
                lv_obj_set_style_text_color(settings_item_icons[i], btn_icon, 0);
                lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                lv_label_set_text(settings_item_icons[i], filter_btn_icons[i]);
                lv_obj_set_width(settings_item_icons[i], 80);
                lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_move_foreground(settings_item_icons[i]);
            }
            if (settings_item_titles[i]) {
                lv_obj_add_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_icon_bgs[i]) {
                lv_obj_move_foreground(settings_item_icon_bgs[i]);
            }
            lv_obj_move_foreground(settings_items[i]);
        }

        settings_v2_filter_btn_last_active = active;
        settings_v2_filter_confirm_last_active = -1;
        settings_v2_last_index = active;
        settings_v2_nav_dir = 0;
        return true;
    }

    if (settings_view == SETTINGS_VIEW_FILTER_RESET) {
        int active = clamp_int(settings_index, 0, 1);
        int total = (filter_lifetime_hours > 0) ? filter_lifetime_hours : 1;
        int used = clamp_int(filter_used_hours, 0, total);
        const lv_color_t secondary = display_theme_dark ? lv_color_hex(0x71717A) : lv_color_hex(0x52525B);
        const lv_color_t panel_bg = display_theme_dark ? lv_color_hex(0x18181B) : lv_color_hex(0xF4F4F5);
        const lv_color_t panel_border = display_theme_dark ? lv_color_hex(0x27272A) : lv_color_hex(0xD4D4D8);
        const lv_color_t btn_border_idle = display_theme_dark ? lv_color_hex(0x52525B) : lv_color_hex(0xA1A1AA);
        const lv_color_t btn_icon_idle = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);

        if (settings_v2_glow) {
            lv_obj_add_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(settings_v2_center_circle, 248, 248);
        lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, -52);
        lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_v2_center_circle, panel_bg, 0);
        lv_obj_set_style_bg_grad_color(settings_v2_center_circle, panel_bg, 0);
        lv_obj_set_style_bg_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_v2_center_circle, 4, 0);
        lv_obj_set_style_border_color(settings_v2_center_circle, panel_border, 0);
        lv_obj_set_style_shadow_opa(settings_v2_center_circle, LV_OPA_TRANSP, 0);

        char used_buf[16];
        snprintf(used_buf, sizeof(used_buf), "%d", used);
        lv_label_set_text(settings_v2_center_icon, used_buf);
        lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_inter_bold_72_lang, 0);
        lv_obj_set_style_text_color(settings_v2_center_icon, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_icon, -2, 0);
        lv_obj_align(settings_v2_center_icon, LV_ALIGN_CENTER, 0, -6);

        char total_buf[20];
        snprintf(total_buf, sizeof(total_buf), ui_tr("of %dh", "из %dч"), total);
        lv_label_set_text(settings_v2_center_title, total_buf);
        lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_medium_22_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_title, 0, 0);
        lv_obj_set_style_text_color(settings_v2_center_title, secondary, 0);
        lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, 32);

        static const char *filter_reset_icons[2] = {
            UI_ICON_LUCIDE_CHEVRON_LEFT,
            UI_ICON_LUCIDE_REFRESH_CW,
        };
        for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
            if (!settings_items[i]) {
                continue;
            }
            if (i >= 2) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            bool selected = (i == active);
            lv_color_t btn_bg = lv_color_hex(0x000000);
            lv_opa_t btn_bg_opa = LV_OPA_TRANSP;
            lv_color_t btn_border = btn_border_idle;
            lv_color_t btn_icon = btn_icon_idle;
            if (selected) {
                if (i == 0) {
                    btn_bg = display_theme_dark ? lv_color_hex(0xE4E4E7) : lv_color_hex(0x18181B);
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = btn_bg;
                    btn_icon = display_theme_dark ? lv_color_hex(0x18181B) : lv_color_white();
                } else {
                    btn_bg = lv_color_hex(0xDC2626);
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = lv_color_hex(0xDC2626);
                    btn_icon = lv_color_white();
                }
            }

            lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_items[i], 92, 92);
            lv_obj_align(settings_items[i], LV_ALIGN_CENTER, (i == 0) ? -48 : 48, 130);
            lv_obj_set_style_radius(settings_items[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(settings_items[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(settings_items[i], 0, 0);
            lv_obj_set_style_pad_all(settings_items[i], 0, 0);
            lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], 84, 84);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], btn_bg, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], btn_bg_opa, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 2, 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], btn_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
            }
            if (settings_item_icons[i]) {
                lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
                lv_obj_set_style_text_color(settings_item_icons[i], btn_icon, 0);
                lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                lv_obj_set_style_transform_zoom(settings_item_icons[i], 282, 0);
                lv_label_set_text(settings_item_icons[i], filter_reset_icons[i]);
                lv_obj_set_width(settings_item_icons[i], 34);
                lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, (i == 0) ? -1 : 0, 0);
            }
            if (settings_item_titles[i]) {
                lv_obj_add_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
        }

        settings_v2_last_index = active;
        settings_v2_nav_dir = 0;
        return true;
    }

    if (settings_view == SETTINGS_VIEW_OVERDRIVE_WARNING) {
        int active = clamp_int(settings_index, 0, 1);

        const lv_color_t primary = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
        const lv_color_t secondary = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);
        const lv_color_t btn_border_idle = display_theme_dark ? lv_color_hex(0x3F3F46) : lv_color_hex(0xA1A1AA);
        const lv_color_t overdrive_accent = lv_color_hex(0xF59E0B);

        if (settings_v2_glow) {
            lv_obj_clear_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_v2_glow, 324, 324);
            lv_obj_align(settings_v2_glow, LV_ALIGN_CENTER, 0, -70);
            lv_obj_set_style_radius(settings_v2_glow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(settings_v2_glow, overdrive_accent, 0);
            lv_obj_set_style_bg_grad_color(settings_v2_glow, lv_color_black(), 0);
#if LV_USE_DRAW_SW_COMPLEX_GRADIENTS
            lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_RADIAL, 0);
            lv_obj_set_style_bg_opa(settings_v2_glow, (lv_opa_t)36, 0);
#else
            lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_NONE, 0);
            lv_obj_set_style_bg_opa(settings_v2_glow, LV_OPA_TRANSP, 0);
#endif
            lv_obj_set_style_border_width(settings_v2_glow, 0, 0);
        }

        lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(settings_v2_center_circle, 150, 150);
        lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, -108);
        lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_v2_center_circle, overdrive_accent, 0);
        lv_obj_set_style_bg_grad_color(settings_v2_center_circle, overdrive_accent, 0);
        lv_obj_set_style_bg_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_v2_center_circle, 0, 0);
        lv_obj_set_style_shadow_opa(settings_v2_center_circle, LV_OPA_TRANSP, 0);

        lv_label_set_text(settings_v2_center_icon, UI_ICON_LUCIDE_ZAP);
        lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_lucide_48_v2_icons, 0);
        lv_obj_set_style_text_color(settings_v2_center_icon, lv_color_white(), 0);
        lv_obj_set_style_text_opa(settings_v2_center_icon, LV_OPA_COVER, 0);
        lv_obj_set_style_transform_zoom(settings_v2_center_icon, 256, 0);
        lv_obj_center(settings_v2_center_icon);

        lv_label_set_text(settings_v2_center_title, ui_tr("OVERDRIVE", "ОВЕРДРАЙВ"));
        lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_semibold_28_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_title, 2, 0);
        lv_obj_set_style_text_color(settings_v2_center_title, primary, 0);
        lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, -4);

        char overdrive_status_buf[96];
        snprintf(overdrive_status_buf,
                 sizeof(overdrive_status_buf),
                 "%s",
                 allow_overdrive
                     ? ui_tr("#FBBF24 110% -# #71717A 20h max# #FBBF24 - ON#",
                             "#FBBF24 110% -# #71717A 20ч макс# #FBBF24 - ВКЛ#")
                     : ui_tr("#FBBF24 110% -# #71717A 20h max# #71717A - OFF#",
                             "#FBBF24 110% -# #71717A 20ч макс# #71717A - ВЫКЛ#"));
        lv_label_set_text(settings_v2_center_value, overdrive_status_buf);
        lv_label_set_recolor(settings_v2_center_value, true);
        lv_obj_set_style_text_font(settings_v2_center_value, &lv_font_inter_medium_22_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_value, 0, 0);
        lv_obj_set_style_text_color(settings_v2_center_value, secondary, 0);
        lv_obj_set_width(settings_v2_center_value, 320);
        lv_obj_set_style_text_align(settings_v2_center_value, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(settings_v2_center_value, LV_ALIGN_CENTER, 0, 38);

        for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
            if (!settings_items[i]) {
                continue;
            }
            if (i >= 2) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            bool selected = (i == active);
            lv_color_t btn_bg = lv_color_hex(0x000000);
            lv_opa_t btn_bg_opa = LV_OPA_TRANSP;
            lv_color_t btn_border = btn_border_idle;
            lv_color_t btn_icon = secondary;

            if (i == 0) {
                if (selected) {
                    btn_bg = display_theme_dark ? lv_color_hex(0xE4E4E7) : lv_color_hex(0x18181B);
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = btn_bg;
                    btn_icon = display_theme_dark ? lv_color_hex(0x18181B) : lv_color_white();
                }
            } else {
                if (selected) {
                    btn_bg = overdrive_accent;
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = overdrive_accent;
                    btn_icon = lv_color_white();
                } else {
                    btn_icon = overdrive_accent;
                }
            }

            lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_items[i], 100, 100);
            lv_obj_align(settings_items[i], LV_ALIGN_CENTER, (i == 0) ? -58 : 58, 132);
            lv_obj_set_style_radius(settings_items[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(settings_items[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(settings_items[i], 0, 0);
            lv_obj_set_style_pad_all(settings_items[i], 0, 0);
            lv_obj_set_style_clip_corner(settings_items[i], false, 0);
            lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], 92, 92);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], btn_bg, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], btn_bg_opa, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 2, 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], btn_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_item_icon_bgs[i]);
            }

            if (settings_item_icons[i]) {
                lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_lucide_48_v2_icons, 0);
                lv_obj_set_style_text_color(settings_item_icons[i], btn_icon, 0);
                lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                lv_label_set_text(settings_item_icons[i],
                                  (i == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_ZAP);
                lv_obj_set_width(settings_item_icons[i], 84);
                lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_move_foreground(settings_item_icons[i]);
            }

            if (settings_item_titles[i]) {
                lv_obj_add_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            lv_obj_move_foreground(settings_items[i]);
        }

        settings_v2_last_index = active;
        settings_v2_nav_dir = 0;
        return true;
    }

    if (settings_view == SETTINGS_VIEW_PURIFICATION_CONFIRM) {
        int active = clamp_int(settings_index, 0, 1);

        const lv_color_t secondary = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);
        const lv_color_t btn_border_idle = display_theme_dark ? lv_color_hex(0x3F3F46) : lv_color_hex(0xA1A1AA);
        const lv_color_t purify_accent = lv_color_hex(0x14D3A7);

        if (settings_v2_glow) {
            lv_obj_clear_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_v2_glow, 324, 324);
            lv_obj_align(settings_v2_glow, LV_ALIGN_CENTER, 0, -70);
            lv_obj_set_style_radius(settings_v2_glow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(settings_v2_glow, purify_accent, 0);
            lv_obj_set_style_bg_grad_color(settings_v2_glow, lv_color_black(), 0);
#if LV_USE_DRAW_SW_COMPLEX_GRADIENTS
            lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_RADIAL, 0);
            lv_obj_set_style_bg_opa(settings_v2_glow, (lv_opa_t)36, 0);
#else
            lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_NONE, 0);
            lv_obj_set_style_bg_opa(settings_v2_glow, LV_OPA_TRANSP, 0);
#endif
            lv_obj_set_style_border_width(settings_v2_glow, 0, 0);
        }

        lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(settings_v2_center_circle, 176, 176);
        lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, -108);
        lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_v2_center_circle, lv_color_hex(0x101114), 0);
        lv_obj_set_style_bg_grad_color(settings_v2_center_circle, lv_color_hex(0x101114), 0);
        lv_obj_set_style_bg_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_v2_center_circle, 6, 0);
        lv_obj_set_style_border_color(settings_v2_center_circle, purify_accent, 0);
        lv_obj_set_style_border_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
        lv_obj_set_style_shadow_opa(settings_v2_center_circle, LV_OPA_TRANSP, 0);

        lv_label_set_text(settings_v2_center_icon, UI_ICON_LUCIDE_WIND);
        lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_lucide_48_v2_icons, 0);
        lv_obj_set_style_text_color(settings_v2_center_icon, purify_accent, 0);
        lv_obj_set_style_text_opa(settings_v2_center_icon, LV_OPA_COVER, 0);
        lv_obj_set_style_transform_zoom(settings_v2_center_icon, 256, 0);
        lv_obj_center(settings_v2_center_icon);

        lv_label_set_text(settings_v2_center_title, ui_tr("PURIFY", "ОЧИСТКА"));
        lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_semibold_28_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_title, 2, 0);
        lv_obj_set_style_text_color(settings_v2_center_title, purify_accent, 0);
        lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, 16);

        lv_label_set_recolor(settings_v2_center_value, false);
        lv_label_set_text(settings_v2_center_value, ui_tr("INSTALL CLEANING FILTER", "УСТАНОВИТЕ ФИЛЬТР ОЧИСТКИ"));
        lv_obj_set_style_text_font(settings_v2_center_value, &lv_font_inter_medium_16_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_value, 2, 0);
        lv_obj_set_style_text_color(settings_v2_center_value, secondary, 0);
        lv_obj_set_width(settings_v2_center_value, 360);
        lv_obj_set_style_text_align(settings_v2_center_value, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(settings_v2_center_value, LV_ALIGN_CENTER, 0, 58);

        for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
            if (!settings_items[i]) {
                continue;
            }
            if (i >= 2) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            bool selected = (i == active);
            lv_color_t btn_bg = lv_color_hex(0x000000);
            lv_opa_t btn_bg_opa = LV_OPA_TRANSP;
            lv_color_t btn_border = btn_border_idle;
            lv_color_t btn_icon = secondary;

            if (i == 0) {
                if (selected) {
                    btn_bg = display_theme_dark ? lv_color_hex(0xE4E4E7) : lv_color_hex(0x18181B);
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = btn_bg;
                    btn_icon = display_theme_dark ? lv_color_hex(0x18181B) : lv_color_white();
                }
            } else {
                btn_icon = purify_accent;
                if (selected) {
                    btn_bg = purify_accent;
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = purify_accent;
                    btn_icon = lv_color_white();
                }
            }

            lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_items[i], 100, 100);
            lv_obj_align(settings_items[i], LV_ALIGN_CENTER, (i == 0) ? -58 : 58, 142);
            lv_obj_set_style_radius(settings_items[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(settings_items[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(settings_items[i], 0, 0);
            lv_obj_set_style_pad_all(settings_items[i], 0, 0);
            lv_obj_set_style_clip_corner(settings_items[i], false, 0);
            lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], 92, 92);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], btn_bg, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], btn_bg_opa, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 2, 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], btn_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_item_icon_bgs[i]);
            }

            if (settings_item_icons[i]) {
                lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_lucide_48_v2_icons, 0);
                lv_obj_set_style_text_color(settings_item_icons[i], btn_icon, 0);
                lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                lv_label_set_text(settings_item_icons[i],
                                  (i == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_WIND);
                lv_obj_set_width(settings_item_icons[i], 84);
                lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_move_foreground(settings_item_icons[i]);
            }

            if (settings_item_titles[i]) {
                lv_obj_add_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            lv_obj_move_foreground(settings_items[i]);
        }

        settings_v2_last_index = active;
        settings_v2_nav_dir = 0;
        return true;
    }

    if (settings_view == SETTINGS_VIEW_OVERDRIVE_MODE) {
        int active = clamp_int(settings_index, 0, 1);
        bool selected_on = (active == 0);

        const lv_color_t primary = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
        const lv_color_t secondary = display_theme_dark ? lv_color_hex(0x71717A) : lv_color_hex(0x52525B);
        const lv_color_t btn_border_idle = display_theme_dark ? lv_color_hex(0x3F3F46) : lv_color_hex(0xA1A1AA);
        const lv_color_t overdrive_accent = lv_color_hex(0xF59E0B);

        if (settings_v2_glow) {
            lv_obj_clear_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_v2_glow, 316, 316);
            lv_obj_align(settings_v2_glow, LV_ALIGN_CENTER, 0, -66);
            lv_obj_set_style_radius(settings_v2_glow, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(settings_v2_glow, overdrive_accent, 0);
            lv_obj_set_style_bg_grad_color(settings_v2_glow, lv_color_black(), 0);
#if LV_USE_DRAW_SW_COMPLEX_GRADIENTS
            lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_RADIAL, 0);
            lv_obj_set_style_bg_opa(settings_v2_glow, (lv_opa_t)36, 0);
#else
            lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_NONE, 0);
            lv_obj_set_style_bg_opa(settings_v2_glow, LV_OPA_TRANSP, 0);
#endif
            lv_obj_set_style_border_width(settings_v2_glow, 0, 0);
        }

        lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_size(settings_v2_center_circle, 176, 176);
        lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, -108);
        lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_v2_center_circle, lv_color_hex(0x101114), 0);
        lv_obj_set_style_bg_grad_color(settings_v2_center_circle, lv_color_hex(0x101114), 0);
        lv_obj_set_style_bg_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_v2_center_circle, 6, 0);
        lv_obj_set_style_border_color(settings_v2_center_circle, overdrive_accent, 0);
        lv_obj_set_style_border_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
        lv_obj_set_style_shadow_opa(settings_v2_center_circle, LV_OPA_TRANSP, 0);

        lv_label_set_text(settings_v2_center_icon, UI_ICON_LUCIDE_ZAP);
        lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_lucide_48_v2_icons, 0);
        lv_obj_set_style_text_color(settings_v2_center_icon, overdrive_accent, 0);
        lv_obj_set_style_text_opa(settings_v2_center_icon, LV_OPA_COVER, 0);
        lv_obj_set_style_transform_zoom(settings_v2_center_icon, 256, 0);
        lv_obj_center(settings_v2_center_icon);

        lv_label_set_text(settings_v2_center_title, selected_on ? ui_tr("ON", "ВКЛ")
                                                                : ui_tr("OFF", "ВЫКЛ"));
        lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_semibold_28_ui, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_title, 2, 0);
        lv_obj_set_style_text_color(settings_v2_center_title, selected_on ? lv_color_hex(0xFBBF24) : primary, 0);
        lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, 20);

        lv_obj_add_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);

        for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
            if (!settings_items[i]) {
                continue;
            }
            if (i >= 2) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            bool selected = (i == active);
            lv_color_t btn_bg = lv_color_hex(0x000000);
            lv_opa_t btn_bg_opa = LV_OPA_TRANSP;
            lv_color_t btn_border = btn_border_idle;

            if (i == 0) {
                if (selected) {
                    btn_bg = overdrive_accent;
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = overdrive_accent;
                } else {
                    btn_bg = lv_color_hex(0x11131A);
                    btn_bg_opa = (lv_opa_t)220;
                    btn_border = btn_border_idle;
                }
            } else {
                if (selected) {
                    btn_bg = lv_color_hex(0x6B7280);
                    btn_bg_opa = LV_OPA_COVER;
                    btn_border = lv_color_hex(0x8B93A3);
                }
            }

            lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_items[i], 100, 100);
            lv_obj_align(settings_items[i], LV_ALIGN_CENTER, (i == 0) ? -58 : 58, 132);
            lv_obj_set_style_radius(settings_items[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_opa(settings_items[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(settings_items[i], 0, 0);
            lv_obj_set_style_pad_all(settings_items[i], 0, 0);
            lv_obj_set_style_clip_corner(settings_items[i], false, 0);
            lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_OVERFLOW_VISIBLE);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], 92, 92);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i], btn_bg, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], btn_bg_opa, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 2, 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], btn_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_item_icon_bgs[i]);
            }

            if (settings_item_icons[i]) {
                if (i == 0) {
                    lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_lucide_48_v2_icons, 0);
                    lv_obj_set_style_text_color(settings_item_icons[i],
                                                selected ? lv_color_white() : overdrive_accent,
                                                0);
                    lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                    lv_label_set_text(settings_item_icons[i], UI_ICON_LUCIDE_ZAP);
                    lv_obj_set_width(settings_item_icons[i], 84);
                    lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                    lv_obj_move_foreground(settings_item_icons[i]);
                } else {
                    lv_obj_add_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                }
            }

            if (settings_item_titles[i]) {
                if (i == 1) {
                    lv_obj_clear_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
                    lv_label_set_text(settings_item_titles[i], ui_tr("OFF", "ВЫКЛ"));
                    lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_22_ui, 0);
                    lv_obj_set_style_text_letter_space(settings_item_titles[i], 1, 0);
                    lv_obj_set_style_text_color(settings_item_titles[i],
                                                selected ? lv_color_white() : secondary,
                                                0);
                    lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_set_width(settings_item_titles[i], 80);
                    lv_obj_align(settings_item_titles[i], LV_ALIGN_CENTER, 0, 0);
                    lv_obj_move_foreground(settings_item_titles[i]);
                } else {
                    lv_obj_add_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
                }
            }

            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            lv_obj_move_foreground(settings_items[i]);
        }

        settings_v2_last_index = active;
        settings_v2_nav_dir = 0;
        return true;
    }

    if (settings_is_custom_accent_edit_active()) {
        const float two_pi = 6.28318530718f;
        const lv_coord_t center_x = EXAMPLE_LCD_H_RES / 2;
        const lv_coord_t center_y = EXAMPLE_LCD_V_RES / 2;
        const lv_coord_t min_res = (EXAMPLE_LCD_H_RES < EXAMPLE_LCD_V_RES) ? EXAMPLE_LCD_H_RES : EXAMPLE_LCD_V_RES;
        const lv_coord_t ring_radius = (min_res / 2) - 18;
        const lv_coord_t knob_r = 15;

        settings_custom_hue_widgets_set_hidden(false);

        if (!settings_custom_hue_ui_ready) {
            if (settings_v2_glow) {
                lv_obj_add_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_v2_left_circle) {
                lv_obj_add_flag(settings_v2_left_circle, LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_v2_right_circle) {
                lv_obj_add_flag(settings_v2_right_circle, LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_v2_left_icon) {
                lv_obj_add_flag(settings_v2_left_icon, LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_v2_right_icon) {
                lv_obj_add_flag(settings_v2_right_icon, LV_OBJ_FLAG_HIDDEN);
            }
            for (int i = 0; i < SETTINGS_V2_DOTS_COUNT; ++i) {
                if (settings_v2_dots[i]) {
                    lv_obj_add_flag(settings_v2_dots[i], LV_OBJ_FLAG_HIDDEN);
                }
            }

            lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_v2_center_circle, 100, 100);
            lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, -8);
            lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_grad_dir(settings_v2_center_circle, LV_GRAD_DIR_NONE, 0);
            lv_obj_set_style_bg_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(settings_v2_center_circle, 3, 0);
            lv_obj_set_style_border_color(settings_v2_center_circle,
                                          display_theme_dark ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x000000),
                                          0);
            lv_obj_set_style_border_opa(settings_v2_center_circle,
                                        display_theme_dark ? (lv_opa_t)52 : (lv_opa_t)26,
                                        0);
            lv_obj_set_style_shadow_opa(settings_v2_center_circle, LV_OPA_TRANSP, 0);
            lv_obj_set_style_shadow_width(settings_v2_center_circle, 0, 0);
            lv_obj_set_style_shadow_spread(settings_v2_center_circle, 0, 0);
            lv_obj_move_foreground(settings_v2_center_circle);

            if (settings_v2_center_icon) {
                lv_obj_add_flag(settings_v2_center_icon, LV_OBJ_FLAG_HIDDEN);
            }

            if (settings_v2_center_title) {
                lv_obj_clear_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_medium_22_ui, 0);
                lv_obj_set_style_text_letter_space(settings_v2_center_title, 1, 0);
                lv_obj_set_style_text_color(settings_v2_center_title,
                                            display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B),
                                            0);
                lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, 74);
                lv_obj_move_foreground(settings_v2_center_title);
            }

            if (settings_v2_center_value) {
                lv_obj_clear_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_v2_center_value, &lv_font_inter_medium_16_ui, 0);
                lv_obj_set_style_text_letter_space(settings_v2_center_value, 2, 0);
                lv_obj_set_style_text_color(settings_v2_center_value, lv_color_hex(0x71717A), 0);
                lv_obj_align(settings_v2_center_value, LV_ALIGN_CENTER, 0, 106);
                lv_obj_move_foreground(settings_v2_center_value);
            }

            settings_custom_hue_ui_ready = true;
        }

        if (settings_custom_hue_knob) {
            float knob_angle_deg = (float)custom_accent_hue - 90.0f;
            float knob_angle = knob_angle_deg * two_pi / 360.0f;
            lv_coord_t knob_cx = center_x + (lv_coord_t)lroundf(cosf(knob_angle) * ring_radius);
            lv_coord_t knob_cy = center_y + (lv_coord_t)lroundf(sinf(knob_angle) * ring_radius);
            lv_obj_set_pos(settings_custom_hue_knob, knob_cx - knob_r, knob_cy - knob_r);
            lv_obj_set_style_bg_color(settings_custom_hue_knob, lv_color_hex(ui_accent_color_hex), 0);
            lv_obj_move_foreground(settings_custom_hue_knob);
        }

        lv_obj_set_style_bg_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_bg_grad_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);

        if (settings_v2_center_title) {
            char hex_buf[16];
            snprintf(hex_buf, sizeof(hex_buf), "#%06X", (unsigned int)(ui_accent_color_hex & 0xFFFFFFU));
            lv_label_set_text(settings_v2_center_title, hex_buf);
            lv_obj_move_foreground(settings_v2_center_title);
        }

        if (settings_v2_center_value) {
            char hue_buf[24];
            snprintf(hue_buf, sizeof(hue_buf), ui_tr("HUE %d DEG", "ТОН %d°"), custom_accent_hue);
            lv_label_set_text(settings_v2_center_value, hue_buf);
            lv_obj_move_foreground(settings_v2_center_value);
        }

        settings_v2_last_index = settings_index;
        settings_v2_nav_dir = 0;
        return true;
    }

    if (settings_v2_is_arc_view()) {
        int min_value = 0;
        int max_value = 100;
        int value = 0;
        bool emphasized_arc_view = (settings_view == SETTINGS_VIEW_SENSITIVITY) ||
                                   (settings_view == SETTINGS_VIEW_BRIGHTNESS);
        const char *title = ui_tr("VALUE", "ЗНАЧЕНИЕ");
        const char *subtitle = "";
        const char *icon = UI_ICON_LUCIDE_GAUGE;
        const char *suffix = "";
        bool show_title = true;
        bool show_subtitle = true;

        bool filter_lifetime = (settings_view == SETTINGS_VIEW_FILTER) && settings_edit_mode && (settings_index == 0);
        bool bright_like_arc = emphasized_arc_view || filter_lifetime;
        if (settings_view == SETTINGS_VIEW_SENSITIVITY) {
            min_value = SENSITIVITY_MIN_CLICKS;
            max_value = SENSITIVITY_MAX_CLICKS;
            value = speed_sensitivity_clicks;
            icon = UI_ICON_LUCIDE_GAUGE;
            show_title = false;
            show_subtitle = false;
        } else if (settings_view == SETTINGS_VIEW_BRIGHTNESS) {
            min_value = DISPLAY_BRIGHTNESS_MIN;
            max_value = DISPLAY_BRIGHTNESS_MAX;
            value = display_brightness_percent;
            icon = UI_ICON_LUCIDE_SUN;
            show_title = false;
            show_subtitle = false;
        } else if (filter_lifetime) {
            min_value = FILTER_LIFETIME_MIN_HOURS;
            max_value = FILTER_LIFETIME_MAX_HOURS;
            value = filter_lifetime_hours;
            title = ui_tr("FILTER LIFETIME", "РЕСУРС ФИЛЬТРА");
            subtitle = ui_tr("HOURS", "ЧАСЫ");
            icon = "";
            show_title = false;
        }

        int span = max_value - min_value;
        int pos = (span > 0) ? ((value - min_value) * 1000) / span : 0;
        if (filter_lifetime) {
            pos = ((value - FILTER_LIFETIME_MIN_HOURS) * 1000) /
                  (FILTER_LIFETIME_MAX_HOURS - FILTER_LIFETIME_MIN_HOURS);
        }
        pos = clamp_int(pos, 0, 1000);

        lv_obj_clear_flag(settings_v2_arc, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(settings_v2_arc,
                        bright_like_arc ? 438 : 260,
                        bright_like_arc ? 438 : 260);
        lv_obj_align(settings_v2_arc, LV_ALIGN_CENTER, 0, bright_like_arc ? 0 : -8);
        lv_obj_set_style_arc_width(settings_v2_arc, bright_like_arc ? 12 : 14, LV_PART_MAIN);
        lv_obj_set_style_arc_width(settings_v2_arc, bright_like_arc ? 12 : 14, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(settings_v2_arc, display_theme_dark ? lv_color_hex(0x1C1C1F) : lv_color_hex(0xD4D4D8), LV_PART_MAIN);
        lv_obj_set_style_arc_color(settings_v2_arc, lv_color_hex(ui_accent_color_hex), LV_PART_INDICATOR);
        lv_arc_set_value(settings_v2_arc, pos);

        if (icon[0] != '\0') {
            lv_obj_clear_flag(settings_v2_arc_icon, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_v2_arc_icon, icon);
            lv_obj_set_style_text_font(settings_v2_arc_icon,
                                       emphasized_arc_view ? &lv_font_lucide_120_v2_icons : &lv_font_lucide_48_v2_icons,
                                       0);
            lv_obj_set_style_text_color(settings_v2_arc_icon, lv_color_hex(ui_accent_color_hex), 0);
            lv_obj_set_style_text_opa(settings_v2_arc_icon, LV_OPA_COVER, 0);
            lv_obj_set_style_transform_zoom(settings_v2_arc_icon, 256, 0);
            lv_obj_set_style_translate_x(settings_v2_arc_icon, 0, 0);
            lv_obj_set_style_translate_y(settings_v2_arc_icon, 0, 0);
            lv_obj_set_style_pad_all(settings_v2_arc_icon, 0, 0);
            lv_obj_set_style_text_align(settings_v2_arc_icon, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_align(settings_v2_arc_icon, LV_ALIGN_CENTER, 0, bright_like_arc ? -136 : -66);
        }

        if (show_title) {
            lv_obj_clear_flag(settings_v2_arc_title, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_v2_arc_title, title);
            lv_obj_align(settings_v2_arc_title, LV_ALIGN_CENTER, 0, -150);
        } else {
            lv_obj_add_flag(settings_v2_arc_title, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_clear_flag(settings_v2_arc_value, LV_OBJ_FLAG_HIDDEN);
        char value_buf[16];
        snprintf(value_buf, sizeof(value_buf), "%d", value);
        lv_label_set_text(settings_v2_arc_value, value_buf);
        lv_obj_set_style_text_font(settings_v2_arc_value,
                                   bright_like_arc ? &lv_font_inter_bold_180_num
                                                  : &lv_font_inter_medium_145_num,
                                   0);
        lv_obj_set_style_transform_zoom(settings_v2_arc_value, 256, 0);
        lv_obj_set_style_text_letter_space(settings_v2_arc_value,
                                           bright_like_arc ? -2 : 0,
                                           0);
        lv_obj_align(settings_v2_arc_value, LV_ALIGN_CENTER, 0,
                     filter_lifetime ? 8 : (bright_like_arc ? 28 : -2));

        if (show_subtitle) {
            lv_obj_clear_flag(settings_v2_arc_subtitle, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_v2_arc_subtitle, subtitle);
            lv_obj_set_style_text_font(settings_v2_arc_subtitle,
                                       filter_lifetime ? &lv_font_inter_semibold_28_ui : &lv_font_inter_medium_16_ui,
                                       0);
            lv_obj_set_style_text_letter_space(settings_v2_arc_subtitle, filter_lifetime ? 3 : 2, 0);
            lv_obj_align(settings_v2_arc_subtitle, LV_ALIGN_CENTER, 0,
                         filter_lifetime ? 122 : (bright_like_arc ? 82 : 82));
        } else {
            lv_obj_add_flag(settings_v2_arc_subtitle, LV_OBJ_FLAG_HIDDEN);
        }

        if (suffix[0] != '\0') {
            lv_obj_clear_flag(settings_v2_arc_suffix, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_v2_arc_suffix, suffix);
            lv_obj_update_layout(settings_v2_layer);
            lv_coord_t x = lv_obj_get_x(settings_v2_arc_value);
            lv_coord_t w = lv_obj_get_width(settings_v2_arc_value);
            lv_coord_t y = lv_obj_get_y(settings_v2_arc_value);
            lv_obj_set_pos(settings_v2_arc_suffix, x + w + 6, y + 12);
        }
        settings_v2_last_index = settings_index;
        settings_v2_nav_dir = 0;
        return true;
    }

    /* Root + Swatch */
    lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(settings_v2_center_title, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(settings_v2_center_value, LV_OBJ_FLAG_HIDDEN);

    int count = settings_items_count > 0 ? settings_items_count : settings_get_item_count();
    int active = clamp_int(settings_index, 0, count - 1);
    int left = active - 1;
    int right = active + 1;
    bool swatch = settings_v2_is_swatch_view(settings_view);
    int selected_value_idx = settings_v2_selected_index_for_view(settings_view);

    uint32_t root_sig = (uint32_t)settings_view;
    root_sig = root_sig * 131U + (uint32_t)active;
    root_sig = root_sig * 131U + (uint32_t)count;
    root_sig = root_sig * 131U + (uint32_t)(swatch ? 1 : 0);
    root_sig = root_sig * 131U + (uint32_t)(selected_value_idx + 1);
    root_sig = root_sig * 131U + (uint32_t)(display_theme_dark ? 1 : 0);
    root_sig = root_sig * 131U + (uint32_t)ui_accent_color_hex;
    root_sig = root_sig * 131U + (uint32_t)display_accent_index;
    root_sig = root_sig * 131U + (uint32_t)current_lang;
    root_sig = root_sig * 131U + (uint32_t)(display_rotation_deg & 0x3FF);
    root_sig = root_sig * 131U + (uint32_t)(allow_child_lock ? 1 : 0);

    bool root_dirty = !settings_v2_root_cache_valid || (settings_v2_root_cache_sig != root_sig);

    lv_obj_set_size(settings_v2_center_circle, swatch ? 194 : 225, swatch ? 194 : 225);
    lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, swatch ? -52 : -64);
    lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
    if (settings_v2_glow) {
#if SETTINGS_V2_ENABLE_GLOW
        lv_obj_clear_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(settings_v2_glow, swatch ? 282 : 326, swatch ? 282 : 326);
        lv_obj_align(settings_v2_glow, LV_ALIGN_CENTER, 0, swatch ? -52 : -64);
#else
        lv_obj_add_flag(settings_v2_glow, LV_OBJ_FLAG_HIDDEN);
#endif
    }

    if (!root_dirty && settings_v2_nav_dir == 0) {
        settings_v2_update_dots(count, active);
        settings_v2_last_index = active;
        return true;
    }

    const char *title = (settings_item_titles[active]) ? lv_label_get_text(settings_item_titles[active]) : "";
    const char *value = (settings_item_values[active]) ? lv_label_get_text(settings_item_values[active]) : "";
    const char *icon_center = (settings_item_icons[active]) ? lv_label_get_text(settings_item_icons[active]) : "";

    lv_color_t center_bg = lv_color_hex(ui_accent_color_hex);
    lv_color_t center_icon_color = lv_color_white();
    lv_color_t center_title_color = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
    lv_color_t center_value_color = display_theme_dark ? lv_color_hex(0xA1A1AA) : lv_color_hex(0x71717A);

    if (swatch) {
        center_bg = display_theme_dark ? lv_color_hex(0x27272A) : lv_color_hex(0xD4D4D8);
        center_icon_color = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
        if (settings_view == SETTINGS_VIEW_ACCENT) {
            if (active < DISPLAY_ACCENT_COUNT) {
                center_bg = lv_color_hex(accent_palette_hex[active]);
                icon_center = (active == display_accent_index) ? UI_ICON_LUCIDE_CHECK : "";
                center_icon_color = lv_color_white();
            } else {
                /* Custom slot */
                center_bg = lv_color_hex(ui_accent_color_hex);
                icon_center = (active == display_accent_index) ? UI_ICON_LUCIDE_CHECK : UI_ICON_LUCIDE_PALETTE;
                center_icon_color = lv_color_white();
            }
        } else if (settings_view == SETTINGS_VIEW_LANGUAGE) {
            icon_center = settings_v2_lang_code(active);
            center_icon_color = (active == selected_value_idx) ? lv_color_hex(ui_accent_color_hex)
                                                                : (display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B));
        } else if (settings_view == SETTINGS_VIEW_THEME) {
            icon_center = (active == 0) ? UI_ICON_LUCIDE_MOON : UI_ICON_LUCIDE_SUN;
            center_bg = (active == 0) ? lv_color_hex(0x18181B) : lv_color_hex(0xE4E4E7);
            center_icon_color = (active == 0) ? lv_color_white() : lv_color_hex(0x18181B);
        } else if (settings_view == SETTINGS_VIEW_ROTATION) {
            icon_center = (active == 0) ? "0" : "180";
            center_icon_color = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
        } else if (settings_view == SETTINGS_VIEW_LOCK_MODE) {
            icon_center = (active == 0) ? UI_ICON_LUCIDE_LOCK : UI_ICON_LUCIDE_LOCK_OPEN;
        }
    } else {
        center_bg = lv_color_hex(ui_accent_color_hex);
        center_icon_color = lv_color_white();
    }

    lv_obj_set_style_bg_color(settings_v2_center_circle, center_bg, 0);
    lv_obj_set_style_bg_grad_color(settings_v2_center_circle, center_bg, 0);
    lv_obj_set_style_bg_grad_dir(settings_v2_center_circle, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(settings_v2_center_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(settings_v2_center_circle,
                                  (swatch && (active == selected_value_idx) && settings_view != SETTINGS_VIEW_ACCENT) ? 4 : 0,
                                  0);
    lv_obj_set_style_border_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
#if SETTINGS_V2_ENABLE_CENTER_SHADOW
    lv_obj_set_style_shadow_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_shadow_opa(settings_v2_center_circle, swatch ? (lv_opa_t)42 : (lv_opa_t)58, 0);
    lv_obj_set_style_shadow_width(settings_v2_center_circle, swatch ? 16 : 22, 0);
    lv_obj_set_style_shadow_spread(settings_v2_center_circle, 0, 0);
#else
    lv_obj_set_style_shadow_opa(settings_v2_center_circle, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(settings_v2_center_circle, 0, 0);
    lv_obj_set_style_shadow_spread(settings_v2_center_circle, 0, 0);
#endif

    if (settings_view == SETTINGS_VIEW_LANGUAGE || settings_view == SETTINGS_VIEW_ROTATION) {
        lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_inter_bold_72_lang, 0);
        lv_obj_set_style_transform_zoom(settings_v2_center_icon, 256, 0);
        lv_obj_set_style_translate_x(settings_v2_center_icon, 0, 0);
        lv_obj_set_style_translate_y(settings_v2_center_icon, 0, 0);
        lv_obj_set_style_text_letter_space(settings_v2_center_icon, 1, 0);
    } else {
        lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_lucide_120_v2_icons, 0);
        lv_obj_set_style_transform_zoom(settings_v2_center_icon, 256, 0);
    }
    lv_obj_set_style_text_color(settings_v2_center_icon, center_icon_color, 0);
    lv_label_set_text(settings_v2_center_icon, icon_center);
    lv_obj_center(settings_v2_center_icon);

    lv_label_set_text(settings_v2_center_title, title);
    lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_letter_space(settings_v2_center_title, 2, 0);
    lv_obj_set_style_text_color(settings_v2_center_title, center_title_color, 0);
    lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, swatch ? 86 : 94);

    lv_label_set_text(settings_v2_center_value, swatch ? "" : value);
    lv_obj_set_style_text_font(settings_v2_center_value, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(settings_v2_center_value, center_value_color, 0);
    lv_obj_align(settings_v2_center_value, LV_ALIGN_CENTER, 0, swatch ? 122 : 130);

    if (left >= 0) {
        lv_obj_clear_flag(settings_v2_left_circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(settings_v2_left_circle, 86, 86);
        lv_obj_set_style_bg_opa(settings_v2_left_circle, display_theme_dark ? (lv_opa_t)22 : (lv_opa_t)16, 0);
        lv_obj_set_style_border_opa(settings_v2_left_circle, LV_OPA_COVER, 0);
        const char *left_icon = (settings_item_icons[left]) ? lv_label_get_text(settings_item_icons[left]) : "";
        if (swatch && settings_view == SETTINGS_VIEW_LANGUAGE) {
            left_icon = settings_v2_lang_code(left);
            lv_obj_set_style_text_font(settings_v2_left_icon, &lv_font_inter_medium_20_caps, 0);
        } else if (swatch && settings_view == SETTINGS_VIEW_ROTATION) {
            left_icon = (left == 0) ? "0" : "180";
            lv_obj_set_style_text_font(settings_v2_left_icon, &lv_font_inter_medium_20_caps, 0);
        } else {
            lv_obj_set_style_text_font(settings_v2_left_icon, &lv_font_lucide_48_v2_icons, 0);
        }
        if (swatch && settings_view == SETTINGS_VIEW_ACCENT && left < DISPLAY_ACCENT_COUNT) {
            lv_obj_set_style_bg_color(settings_v2_left_circle, lv_color_hex(accent_palette_hex[left]), 0);
            left_icon = "";
        }
        lv_label_set_text(settings_v2_left_icon, left_icon);
        lv_obj_clear_flag(settings_v2_left_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(settings_v2_left_circle, LV_OBJ_FLAG_HIDDEN);
    }

    if (right < count) {
        lv_obj_clear_flag(settings_v2_right_circle, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_size(settings_v2_right_circle, 86, 86);
        lv_obj_set_style_bg_opa(settings_v2_right_circle, display_theme_dark ? (lv_opa_t)22 : (lv_opa_t)16, 0);
        lv_obj_set_style_border_opa(settings_v2_right_circle, LV_OPA_COVER, 0);
        const char *right_icon = (settings_item_icons[right]) ? lv_label_get_text(settings_item_icons[right]) : "";
        if (swatch && settings_view == SETTINGS_VIEW_LANGUAGE) {
            right_icon = settings_v2_lang_code(right);
            lv_obj_set_style_text_font(settings_v2_right_icon, &lv_font_inter_medium_20_caps, 0);
        } else if (swatch && settings_view == SETTINGS_VIEW_ROTATION) {
            right_icon = (right == 0) ? "0" : "180";
            lv_obj_set_style_text_font(settings_v2_right_icon, &lv_font_inter_medium_20_caps, 0);
        } else {
            lv_obj_set_style_text_font(settings_v2_right_icon, &lv_font_lucide_48_v2_icons, 0);
        }
        if (swatch && settings_view == SETTINGS_VIEW_ACCENT && right < DISPLAY_ACCENT_COUNT) {
            lv_obj_set_style_bg_color(settings_v2_right_circle, lv_color_hex(accent_palette_hex[right]), 0);
            right_icon = "";
        }
        lv_label_set_text(settings_v2_right_icon, right_icon);
        lv_obj_clear_flag(settings_v2_right_icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(settings_v2_right_circle, LV_OBJ_FLAG_HIDDEN);
    }

    settings_v2_update_dots(count, active);
    if (settings_v2_last_index != active && settings_v2_nav_dir != 0) {
#if SETTINGS_V2_ANIMATE_CENTER_CIRCLE
        settings_v2_play_slide_anim(settings_v2_center_circle, settings_v2_nav_dir);
#endif
        settings_v2_play_slide_anim(settings_v2_center_title, settings_v2_nav_dir);
        settings_v2_play_slide_anim(settings_v2_center_value, settings_v2_nav_dir);
    }
    settings_v2_root_cache_valid = true;
    settings_v2_root_cache_sig = root_sig;
    settings_v2_last_index = active;
    settings_v2_nav_dir = 0;
    return true;
}

static lv_coord_t settings_wheel_step_for_view(settings_view_t view)
{
    return (view == SETTINGS_VIEW_ROOT ||
            view == SETTINGS_VIEW_ACCENT ||
            view == SETTINGS_VIEW_LANGUAGE ||
            view == SETTINGS_VIEW_THEME ||
            view == SETTINGS_VIEW_ROTATION ||
            view == SETTINGS_VIEW_LOCK_MODE ||
            view == SETTINGS_VIEW_OVERDRIVE_MODE ||
            view == SETTINGS_VIEW_FILTER) ? SETTINGS_WHEEL_ROOT_STEP_PX : 98;
}

static bool settings_view_uses_wheel_list(settings_view_t view)
{
    (void)view;
    return false;
}

static bool settings_view_uses_compact_marker(settings_view_t view)
{
    return (view == SETTINGS_VIEW_ACCENT ||
            view == SETTINGS_VIEW_LANGUAGE ||
            view == SETTINGS_VIEW_THEME ||
            view == SETTINGS_VIEW_LOCK_MODE);
}

static int settings_compact_marker_count(settings_view_t view)
{
    switch (view) {
    case SETTINGS_VIEW_ACCENT: return DISPLAY_ACCENT_COUNT;
    case SETTINGS_VIEW_LANGUAGE: return SETTINGS_LANG_OPTIONS_COUNT;
    case SETTINGS_VIEW_THEME: return 2;
    case SETTINGS_VIEW_LOCK_MODE: return 2;
    default: return 0;
    }
}

static const char *settings_compact_marker_text(settings_view_t view, int index)
{
    (void)index;
    switch (view) {
    case SETTINGS_VIEW_LANGUAGE:
        switch (index) {
        case 0: return "EN";
        case 1: return "ET";
        case 2: return "RU";
        case 3: return "LV";
        case 4: return "FI";
        case 5: return "DE";
        case 6: return "ES";
        case 7: return "FR";
        case 8: return "UK";
        default: return "";
        }
    case SETTINGS_VIEW_THEME:
        return (index == 0) ? "DK" : ((index == 1) ? "LT" : "");
    case SETTINGS_VIEW_LOCK_MODE:
        return (index == 0) ? "A" : ((index == 1) ? "U" : "");
    case SETTINGS_VIEW_ACCENT:
    default:
        return "";
    }
}

static lv_color_t settings_compact_marker_bg_color(settings_view_t view, int index)
{
    if (view == SETTINGS_VIEW_ACCENT && index >= 0 && index < DISPLAY_ACCENT_COUNT) {
        return lv_color_hex(accent_palette_hex[index]);
    }
    if (view == SETTINGS_VIEW_THEME) {
        if (index == 0) {
            return lv_color_hex(0x111111);
        }
        if (index == 1) {
            return lv_color_white();
        }
    }
    return lv_color_white();
}

static lv_color_t settings_compact_marker_text_color(settings_view_t view, int index)
{
    if (view == SETTINGS_VIEW_THEME && index == 0) {
        return lv_color_white();
    }
    return lv_color_hex(0x101014);
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

static void fps_counter_create(void)
{
    if (!UI_SHOW_FPS_COUNTER) {
        label_fps_counter = NULL;
        fps_counter_frames = 0;
        fps_counter_last_update_us = 0;
        fps_counter_cpu_percent = 0xFF;
        fps_counter_last_cpu_sample_us = 0;
        fps_counter_prev_total_runtime = 0;
        fps_counter_prev_idle_runtime = 0;
        fps_counter_prev_runtime_valid = false;
        return;
    }

    lv_obj_t *parent = lv_layer_top();
    label_fps_counter = lv_label_create(parent);
    lv_obj_set_style_text_font(label_fps_counter, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(label_fps_counter, lv_color_hex(0xF4F4F5), 0);
    lv_obj_set_style_bg_color(label_fps_counter, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(label_fps_counter, (lv_opa_t)168, 0);
    lv_obj_set_style_pad_left(label_fps_counter, 6, 0);
    lv_obj_set_style_pad_right(label_fps_counter, 6, 0);
    lv_obj_set_style_pad_top(label_fps_counter, 2, 0);
    lv_obj_set_style_pad_bottom(label_fps_counter, 2, 0);
    lv_obj_set_style_radius(label_fps_counter, 6, 0);
    lv_obj_set_style_text_letter_space(label_fps_counter, 1, 0);
    lv_obj_set_style_border_width(label_fps_counter, 1, 0);
    lv_obj_set_style_border_color(label_fps_counter, lv_color_hex(0x52525B), 0);
    lv_obj_set_style_border_opa(label_fps_counter, (lv_opa_t)120, 0);
    lv_label_set_text(label_fps_counter, ui_tr("FPS -- CPU --", "К/С -- ЦП --"));
    lv_obj_align(label_fps_counter, LV_ALIGN_CENTER, UI_MAIN_CENTER_OFFSET_X, UI_MAIN_CENTER_OFFSET_Y + 106);
    lv_obj_clear_flag(label_fps_counter, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(label_fps_counter);

    fps_counter_frames = 0;
    fps_counter_last_update_us = esp_timer_get_time();
    fps_counter_cpu_percent = 0xFF;
    fps_counter_last_cpu_sample_us = 0;
    fps_counter_prev_total_runtime = 0;
    fps_counter_prev_idle_runtime = 0;
    fps_counter_prev_runtime_valid = false;
}

static uint32_t runtime_counter_delta_u32(uint32_t curr, uint32_t prev)
{
    return (curr >= prev) ? (curr - prev) : (uint32_t)(0xFFFFFFFFu - prev + 1u + curr);
}

static uint8_t sample_cpu_load_percent(void)
{
#if CONFIG_FREERTOS_USE_TRACE_FACILITY && CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    enum { FPS_CPU_MAX_TASKS = 48 };
    TaskStatus_t tasks[FPS_CPU_MAX_TASKS];

    uint32_t total_runtime = 0;
    UBaseType_t count = uxTaskGetSystemState(tasks, FPS_CPU_MAX_TASKS, &total_runtime);
    if (count == 0 || total_runtime == 0) {
        return 0xFF;
    }

    uint32_t idle_runtime = 0;
    for (UBaseType_t i = 0; i < count; ++i) {
        if (tasks[i].pcTaskName && strncmp(tasks[i].pcTaskName, "IDLE", 4) == 0) {
            idle_runtime += tasks[i].ulRunTimeCounter;
        }
    }
    if (!fps_counter_prev_runtime_valid) {
        fps_counter_prev_total_runtime = total_runtime;
        fps_counter_prev_idle_runtime = idle_runtime;
        fps_counter_prev_runtime_valid = true;
        return 0xFF;
    }

    uint32_t total_delta = runtime_counter_delta_u32(total_runtime, fps_counter_prev_total_runtime);
    uint32_t idle_delta = runtime_counter_delta_u32(idle_runtime, fps_counter_prev_idle_runtime);
    fps_counter_prev_total_runtime = total_runtime;
    fps_counter_prev_idle_runtime = idle_runtime;

    if (total_delta == 0) {
        return 0xFF;
    }
    if (idle_delta > total_delta) {
        idle_delta = total_delta;
    }

    uint32_t busy = 100U - (uint32_t)((idle_delta * 100ULL + (total_delta / 2ULL)) / total_delta);
    if (busy > 100U) {
        busy = 100U;
    }
    return (uint8_t)busy;
#else
    return 0xFF;
#endif
}

static void fps_display_refr_event_cb(lv_event_t *e)
{
    if (!e || lv_event_get_code(e) != LV_EVENT_REFR_START) {
        return;
    }

    ui_refresh_rate_tick();

    if (!label_fps_counter) {
        return;
    }
    if (lv_obj_has_flag(label_fps_counter, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    fps_counter_frames++;
    int64_t now_us = esp_timer_get_time();
    int64_t dt_us = now_us - fps_counter_last_update_us;
    if (dt_us < 250000) {
        return;
    }

    uint32_t fps = (uint32_t)((fps_counter_frames * 1000000ULL + (uint64_t)(dt_us / 2)) / (uint64_t)dt_us);
    if (fps_counter_last_cpu_sample_us == 0 || (now_us - fps_counter_last_cpu_sample_us) >= 1000000) {
        fps_counter_cpu_percent = sample_cpu_load_percent();
        fps_counter_last_cpu_sample_us = now_us;
    }

    char buf[24];
    if (fps_counter_cpu_percent <= 100) {
        snprintf(buf, sizeof(buf), ui_tr("FPS %02u CPU %02u%%", "К/С %02u ЦП %02u%%"), fps, fps_counter_cpu_percent);
    } else {
        snprintf(buf, sizeof(buf), ui_tr("FPS %02u CPU --", "К/С %02u ЦП --"), fps);
    }
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
    if (settings_v2_render_layout()) {
        return;
    }

    settings_v2_hide_layer();

    const bool root_view = (settings_view == SETTINGS_VIEW_ROOT);
    const bool filter_view = (settings_view == SETTINGS_VIEW_FILTER);
    const bool filter_reset_view = (settings_view == SETTINGS_VIEW_FILTER_RESET);
    const bool about_view = (settings_view == SETTINGS_VIEW_ABOUT);
    const bool overdrive_warning_view = (settings_view == SETTINGS_VIEW_OVERDRIVE_WARNING);
    const bool purification_confirm_view = (settings_view == SETTINGS_VIEW_PURIFICATION_CONFIRM);
    const bool overdrive_mode_view = (settings_view == SETTINGS_VIEW_OVERDRIVE_MODE);
    const bool accent_view = (settings_view == SETTINGS_VIEW_ACCENT);
    const bool compact_marker_view = settings_view_uses_compact_marker(settings_view);
    const lv_color_t theme_card_bg = display_theme_dark ? lv_color_hex(0x17191E) : lv_color_white();
    const lv_color_t theme_border = display_theme_dark ? lv_color_white() : lv_color_black();
    const lv_color_t theme_text_primary = display_theme_dark ? lv_color_white() : lv_color_black();
    const lv_color_t theme_text_secondary = display_theme_dark ? lv_color_hex(0x5D5D66) : lv_color_black();
    const lv_color_t theme_icon_bg = display_theme_dark ? lv_color_white() : lv_color_hex(0x27272A);
    const lv_color_t theme_button_idle_bg = display_theme_dark ? lv_color_hex(0x0E1117) : lv_color_white();
    const lv_color_t theme_button_idle_border = display_theme_dark ? lv_color_hex(0x2B3038) : lv_color_black();
    const lv_color_t theme_button_idle_text = display_theme_dark ? lv_color_hex(0x8B93A3) : lv_color_black();
    const lv_color_t theme_back_selected_bg = display_theme_dark ? lv_color_white() : lv_color_black();
    const lv_color_t theme_back_selected_text = display_theme_dark ? lv_color_hex(0x101014) : lv_color_white();
    const lv_color_t theme_slider_track = display_theme_dark ? lv_color_hex(0x2D2F38) : lv_color_hex(0xD1D5DB);
    const lv_color_t theme_panel_bg = display_theme_dark ? lv_color_hex(0x18181B) : lv_color_white();
    const lv_color_t theme_panel_ring = display_theme_dark ? lv_color_hex(0x27272A) : lv_color_black();
    const bool list_cards_view = root_view || compact_marker_view || filter_view || overdrive_mode_view;
    const bool filter_lifetime_panel_view = (settings_view == SETTINGS_VIEW_FILTER) &&
                                            settings_edit_mode &&
                                            (settings_index == 0);
    const bool value_panel_view = (settings_view == SETTINGS_VIEW_SENSITIVITY) ||
                                  (settings_view == SETTINGS_VIEW_BRIGHTNESS) ||
                                  filter_lifetime_panel_view;
    const bool slider_view = value_panel_view;
    const bool confirm_view = (settings_view == SETTINGS_VIEW_OVERDRIVE_WARNING) ||
                              (settings_view == SETTINGS_VIEW_PURIFICATION_CONFIRM);
    const lv_coord_t card_w = list_cards_view ? 350 : 360;
    const lv_coord_t card_h = list_cards_view ? 98 : 92;
    const lv_coord_t center_y = list_cards_view ? ((EXAMPLE_LCD_V_RES - card_h) / 2) : (((EXAMPLE_LCD_V_RES - card_h) / 2) + 24);
    const lv_coord_t list_x_off = 0;
    const lv_coord_t icon_bg_x = 8;
    const lv_coord_t text_x = 118;
    const lv_coord_t arrow_x = -3;
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
    int wheel_window_start = 0;
    int wheel_window_end = settings_items_count - 1;
    int wheel_preload_index = -1;
    if (wheel_list_view && list_cards_view && settings_items_count > 0) {
        /* Keep exactly 3 visible cards (prev/current/next) */
        wheel_window_start = visual_index - 1;
        wheel_window_end = wheel_window_start + 2;
        wheel_window_start = clamp_int(wheel_window_start, 0, settings_items_count - 1);
        wheel_window_end = clamp_int(wheel_window_end, 0, settings_items_count - 1);
        if ((wheel_window_end - wheel_window_start) < 2) {
            wheel_window_start = clamp_int(wheel_window_end - 2, 0, settings_items_count - 1);
            wheel_window_end = clamp_int(wheel_window_start + 2, 0, settings_items_count - 1);
        }

        /* Preload one extra card outside viewport to reduce pop-in */
        if (settings_items_count > 3) {
            float wheel_pos = scroll_top / wheel_step;
            int base = (int)floorf(wheel_pos);
            float frac = wheel_pos - (float)base;
            if (frac < 0.0f) {
                frac = 0.0f;
            } else if (frac > 1.0f) {
                frac = 1.0f;
            }
            if (frac >= 0.5f) {
                wheel_preload_index = wheel_window_end + 1;
            } else {
                wheel_preload_index = wheel_window_start - 1;
            }
            if (wheel_preload_index < 0 || wheel_preload_index >= settings_items_count) {
                wheel_preload_index = -1;
            }
        }
    }

    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        lv_obj_t *item = settings_items[i];
        if (!item) {
            continue;
        }
        if (about_view) {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        if (filter_reset_view && i > 1) {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        if (value_panel_view) {
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
        lv_opa_t wheel_item_opa = LV_OPA_COVER;
        lv_opa_t wheel_icon_opa = LV_OPA_COVER;
        lv_opa_t wheel_title_opa = LV_OPA_COVER;
        lv_opa_t wheel_value_opa = LV_OPA_COVER;
        lv_opa_t wheel_arrow_opa = (lv_opa_t)220;

        if (wheel_list_view) {
            signed_dist_px = ((float)i * wheel_step) - scroll_top;
            distance_px = fabsf(signed_dist_px);

            wheel_item_opa = LV_OPA_COVER;
            wheel_icon_opa = LV_OPA_COVER;
            wheel_title_opa = LV_OPA_COVER;
            wheel_value_opa = LV_OPA_COVER;
            wheel_arrow_opa = (lv_opa_t)220;
            if (list_cards_view) {
                bool keep_for_preload = (i == wheel_preload_index);
                if ((i < wheel_window_start || i > wheel_window_end) && !keep_for_preload) {
                    lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
                    continue;
                }
            } else if (distance_px > (SETTINGS_WHEEL_MAX_DIST_PX + (SETTINGS_WHEEL_CULL_EXTRA_STEPS * wheel_step))) {
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
            if (list_cards_view && i == wheel_preload_index) {
                y = (i > visual_index) ? (EXAMPLE_LCD_V_RES + card_h + 8) : (-card_h - 8);
            }
        }

        if (slider_view) {
            if (i == 0) {
                item_w = 352;
                item_h = 212;
                y = 66;
            }
        } else if (confirm_view) {
            if (i == 0) {
                item_w = 72;
                item_h = 72;
            } else {
                item_w = 170;
                item_h = 72;
            }
            y = (overdrive_warning_view || purification_confirm_view) ? 356 : 324;
        }

        if (filter_reset_view) {
            lv_coord_t xoff = (i == 0) ? -96 : 52;
            lv_coord_t by = 356;
            if (i == 0) {
                item_w = 72;
                item_h = 72;
                lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(item, selected ? theme_back_selected_bg : theme_button_idle_bg, 0);
                lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(item, 2, 0);
                lv_obj_set_style_border_color(item, selected ? theme_border : theme_button_idle_border, 0);
                lv_obj_set_style_border_opa(item, LV_OPA_COVER, 0);
            } else {
                item_w = 170;
                item_h = 72;
                lv_obj_set_style_radius(item, 36, 0);
                lv_obj_set_style_bg_color(item, selected ? lv_color_hex(UI_ACCENT_COLOR_HEX) : theme_button_idle_bg, 0);
                lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(item, 2, 0);
                lv_obj_set_style_border_color(item, selected ? lv_color_hex(UI_ACCENT_COLOR_HEX) : theme_button_idle_border, 0);
                lv_obj_set_style_border_opa(item, LV_OPA_COVER, 0);
            }
            lv_obj_set_size(item, item_w, item_h);
            lv_obj_align(item, LV_ALIGN_TOP_MID, xoff, by);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], item_w, item_h);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 0, 0);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_icons[i]) {
                lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
                lv_obj_set_style_text_color(settings_item_icons[i],
                                            selected ? ((i == 0) ? theme_back_selected_text : lv_color_white())
                                                     : theme_button_idle_text,
                                            0);
                lv_obj_set_style_text_opa(settings_item_icons[i], LV_OPA_COVER, 0);
                lv_label_set_text(settings_item_icons[i], (i == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_REFRESH_CW);
                lv_obj_set_width(settings_item_icons[i], (i == 0) ? 40 : 42);
                lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_align(settings_item_icons[i], (i == 0) ? LV_ALIGN_CENTER : LV_ALIGN_LEFT_MID, (i == 0) ? -6 : 24, 0);
            }
            if (settings_item_titles[i]) {
                if (i == 0) {
                    lv_label_set_text(settings_item_titles[i], "");
                    lv_obj_clear_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_clear_flag(settings_item_titles[i], LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(settings_item_titles[i], ui_tr("RESET", "СБРОС"));
                    lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_22_ui, 0);
                    lv_obj_set_style_text_color(settings_item_titles[i], selected ? lv_color_white() : theme_button_idle_text, 0);
                    lv_obj_set_style_text_opa(settings_item_titles[i], LV_OPA_COVER, 0);
                    lv_obj_set_width(settings_item_titles[i], 106);
                    lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_LEFT, 0);
                    lv_obj_align(settings_item_titles[i], LV_ALIGN_LEFT_MID, 64, 0);
                }
            }
            lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        if (confirm_view) {
            lv_coord_t xoff = (overdrive_warning_view || purification_confirm_view)
                                  ? ((i == 0) ? -96 : 52)
                                  : ((i == 0) ? -70 : 66);
            lv_obj_set_size(item, item_w, item_h);
            lv_obj_align(item, LV_ALIGN_TOP_MID, xoff, y);
            lv_obj_set_style_radius(item, (i == 0) ? LV_RADIUS_CIRCLE : 36, 0);
            if (overdrive_warning_view) {
                if (i == 0) {
                    lv_obj_set_style_bg_color(item, selected ? theme_back_selected_bg : theme_button_idle_bg, 0);
                    lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                    lv_obj_set_style_border_width(item, 2, 0);
                    lv_obj_set_style_border_color(item, selected ? theme_border : theme_button_idle_border, 0);
                    lv_obj_set_style_border_opa(item, LV_OPA_COVER, 0);
                } else {
                    lv_obj_set_style_bg_color(item, selected ? lv_color_hex(0xE10600) : theme_button_idle_bg, 0);
                    lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                    lv_obj_set_style_border_width(item, 2, 0);
                    lv_obj_set_style_border_color(item, selected ? lv_color_hex(0xE10600) : theme_button_idle_border, 0);
                    lv_obj_set_style_border_opa(item, LV_OPA_COVER, 0);
                }
            } else if (purification_confirm_view) {
                if (i == 0) {
                    lv_obj_set_style_bg_color(item, selected ? theme_back_selected_bg : theme_button_idle_bg, 0);
                    lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                    lv_obj_set_style_border_width(item, 2, 0);
                    lv_obj_set_style_border_color(item, selected ? theme_border : theme_button_idle_border, 0);
                    lv_obj_set_style_border_opa(item, LV_OPA_COVER, 0);
                } else {
                    lv_obj_set_style_bg_color(item, selected ? lv_color_hex(0x0DBF84) : theme_button_idle_bg, 0);
                    lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                    lv_obj_set_style_border_width(item, 2, 0);
                    lv_obj_set_style_border_color(item, selected ? lv_color_hex(0x0DBF84) : theme_button_idle_border, 0);
                    lv_obj_set_style_border_opa(item, LV_OPA_COVER, 0);
                }
            } else {
                lv_obj_set_style_bg_color(item, (i == 0) ? lv_color_hex(0xF4F4F5) : lv_color_hex(0x27272A), 0);
                lv_obj_set_style_bg_opa(item, selected ? LV_OPA_COVER : (lv_opa_t)88, 0);
                lv_obj_set_style_border_width(item, selected ? 2 : 1, 0);
                lv_obj_set_style_border_color(item, selected ? lv_color_hex(0xA1A1AA) : lv_palette_darken(LV_PALETTE_GREY, 3), 0);
                lv_obj_set_style_border_opa(item, selected ? (lv_opa_t)86 : (lv_opa_t)35, 0);
            }
            lv_obj_set_style_opa(item, LV_OPA_COVER, 0);

            if (settings_item_icon_bgs[i]) {
                lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_item_icon_bgs[i], item_w, item_h);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_CENTER, 0, 0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(settings_item_icon_bgs[i], 0, 0);
            }
            if (settings_item_values[i]) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_arrows[i]) {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
            if (settings_item_icons[i]) {
                lv_color_t confirm_icon_color = lv_color_hex(0x7A7A83);
                if (overdrive_warning_view || purification_confirm_view) {
                    confirm_icon_color = (i == 0)
                                             ? (selected ? theme_back_selected_text : theme_button_idle_text)
                                             : (selected ? lv_color_white() : theme_button_idle_text);
                } else {
                    confirm_icon_color = (i == 0) ? lv_color_hex(0x18181B) : lv_color_hex(0x7A7A83);
                }
                lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
                lv_obj_set_style_text_color(settings_item_icons[i], confirm_icon_color, 0);
                lv_label_set_text(settings_item_icons[i], (i == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : "");
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, -6, 0);
                if (i == 0) {
                    lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_add_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                }
            }
            if (settings_item_titles[i]) {
                lv_color_t confirm_title_color = lv_color_hex(0x7A7A83);
                if (overdrive_warning_view || purification_confirm_view) {
                    confirm_title_color = (i == 0)
                                              ? lv_color_hex(0x18181B)
                                              : (selected ? lv_color_white() : theme_button_idle_text);
                    if (i == 0 && selected) {
                        confirm_title_color = theme_back_selected_text;
                    }
                } else {
                    confirm_title_color = (i == 0) ? lv_color_hex(0x18181B) : lv_color_hex(0x7A7A83);
                }
                lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_22_ui, 0);
                lv_obj_set_style_text_color(settings_item_titles[i], confirm_title_color, 0);
                lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_width(settings_item_titles[i], item_w);
                lv_obj_align(settings_item_titles[i], LV_ALIGN_CENTER, 0, 0);
            }
            lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        lv_obj_set_size(item, item_w, item_h);
        if (list_cards_view) {
            lv_obj_align(item, LV_ALIGN_TOP_MID, list_x_off, y);
        } else {
            lv_obj_align(item, LV_ALIGN_TOP_MID, 0, y);
        }

        lv_obj_set_style_opa(item, wheel_item_opa, 0);
        if (list_cards_view && i == wheel_preload_index) {
            /* Keep one card pre-created outside the viewport without showing it. */
            lv_obj_set_style_opa(item, LV_OPA_TRANSP, 0);
        }

        bool visible = wheel_list_view
                           ? (list_cards_view
                                  ? (i >= wheel_window_start && i <= wheel_window_end)
                                  : (distance_px <= (SETTINGS_WHEEL_MAX_DIST_PX + (SETTINGS_WHEEL_VISIBLE_EXTRA_STEPS * wheel_step)))
                             )
                           : (list_cards_view ? (ad <= 3) : (ad <= 2));

        /* Performance path: while scrolling animate only geometry/visibility */
        if (wheel_list_view && settings_root_scroll_animating) {
            if (visible || (list_cards_view && i == wheel_preload_index)) {
                lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }

        if (fast_anim_pass) {
            if (list_cards_view) {
                lv_obj_set_style_bg_color(item, theme_card_bg, 0);
                lv_opa_t fast_bg_opa = (lv_opa_t)92;
                lv_obj_set_style_bg_opa(item, fast_bg_opa, 0);
                lv_obj_set_style_border_width(item, 1, 0);
                lv_opa_t fast_border_opa = LV_OPA_COVER;
                lv_obj_set_style_border_opa(item, fast_border_opa, 0);
                lv_obj_set_style_border_color(item, theme_border, 0);

                if (settings_item_icon_bgs[i]) {
                    lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
                    bool compact_marker = compact_marker_view && (i < settings_compact_marker_count(settings_view));
                    if (compact_marker) {
                        lv_obj_set_size(settings_item_icon_bgs[i], 76, 76);
                        lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, icon_bg_x, 0);
                    } else {
                        lv_obj_set_size(settings_item_icon_bgs[i], 76, 76);
                        lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, icon_bg_x, 0);
                    }
                    if (compact_marker) {
                        lv_obj_set_style_bg_color(settings_item_icon_bgs[i], settings_compact_marker_bg_color(settings_view, i), 0);
                        lv_obj_set_style_border_width(settings_item_icon_bgs[i], 1, 0);
                    } else {
                        lv_obj_set_style_bg_color(settings_item_icon_bgs[i], theme_icon_bg, 0);
                        lv_obj_set_style_border_width(settings_item_icon_bgs[i], 1, 0);
                    }
                    lv_obj_set_style_bg_opa(settings_item_icon_bgs[i],
                                            LV_OPA_COVER,
                                            0);
                    lv_obj_set_style_border_color(settings_item_icon_bgs[i], theme_border, 0);
                    lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
                }
                if (settings_item_icons[i]) {
                    bool compact_marker = compact_marker_view && (i < settings_compact_marker_count(settings_view));
                    if (compact_marker) {
                        const char *badge = settings_compact_marker_text(settings_view, i);
                        if (badge[0] == '\0') {
                            lv_obj_add_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                        } else {
                            lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                            lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_inter_medium_22_ui, 0);
                            lv_obj_set_style_text_color(settings_item_icons[i], settings_compact_marker_text_color(settings_view, i), 0);
                            lv_obj_set_style_text_opa(settings_item_icons[i], wheel_icon_opa, 0);
                            lv_label_set_text(settings_item_icons[i], badge);
                            lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                        }
                    } else {
                        lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                        lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
                        lv_obj_set_style_text_color(settings_item_icons[i], lv_color_hex(ui_accent_color_hex), 0);
                        lv_obj_set_style_text_opa(settings_item_icons[i], wheel_icon_opa, 0);
                        lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 1, 1);
                    }
                }
                if (settings_item_titles[i]) {
                    lv_obj_set_style_text_color(settings_item_titles[i], theme_text_primary, 0);
                    lv_obj_set_style_text_opa(settings_item_titles[i], wheel_title_opa, 0);
                }
                if (settings_item_values[i]) {
                    if (settings_item_value_is_icon[i]) {
                        if (accent_view && i < DISPLAY_ACCENT_COUNT) {
                            lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(accent_palette_hex[i]), 0);
                        } else {
                            lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(ui_accent_color_hex), 0);
                        }
                    } else {
                        lv_obj_set_style_text_color(settings_item_values[i], theme_text_secondary, 0);
                    }
                    lv_obj_set_style_text_opa(settings_item_values[i], wheel_value_opa, 0);
                }
                if (settings_item_arrows[i]) {
                    lv_obj_set_style_text_color(settings_item_arrows[i], theme_text_secondary, 0);
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

        lv_opa_t bg_opa = wheel_list_view ? (lv_opa_t)92 : (lv_opa_t)(selected ? 84 : 64);
        lv_obj_set_style_bg_color(item, theme_card_bg, 0);
        lv_obj_set_style_bg_opa(item, bg_opa, 0);
        lv_obj_set_style_radius(item, 30, 0);
        if (list_cards_view) {
            lv_obj_set_style_border_width(item, 1, 0);
            lv_opa_t root_border_opa = LV_OPA_COVER;
            lv_obj_set_style_border_opa(item, root_border_opa, 0);
            lv_obj_set_style_border_color(item, theme_border, 0);
        } else {
            lv_obj_set_style_border_width(item, selected ? 3 : 1, 0);
            lv_obj_set_style_border_opa(item, selected ? (lv_opa_t)86 : (lv_opa_t)34, 0);
            lv_obj_set_style_border_color(item,
                                          selected ? (settings_edit_mode ? lv_color_hex(ui_accent_color_hex)
                                                                         : theme_border)
                                                   : theme_border,
                                          0);
        }

        if (settings_item_icon_bgs[i]) {
            lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_HIDDEN);
            int icon_bg_size = list_cards_view ? 76 : (selected ? 74 : 64);
            bool compact_marker = compact_marker_view && (i < settings_compact_marker_count(settings_view));
            if (compact_marker) {
                lv_obj_set_size(settings_item_icon_bgs[i], 76, 76);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, icon_bg_x, 0);
            } else {
                lv_obj_set_size(settings_item_icon_bgs[i], icon_bg_size, icon_bg_size);
                lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, icon_bg_x, 0);
            }
            if (list_cards_view && wheel_list_view) {
                if (compact_marker) {
                    lv_obj_set_style_bg_color(settings_item_icon_bgs[i], settings_compact_marker_bg_color(settings_view, i), 0);
                    lv_obj_set_style_border_width(settings_item_icon_bgs[i], 1, 0);
                } else {
                    lv_obj_set_style_bg_color(settings_item_icon_bgs[i], theme_icon_bg, 0);
                    lv_obj_set_style_border_width(settings_item_icon_bgs[i], 1, 0);
                }
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i],
                                        LV_OPA_COVER,
                                        0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], theme_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
            } else {
                lv_obj_set_style_bg_color(settings_item_icon_bgs[i],
                                          display_theme_dark
                                              ? (selected ? lv_color_hex(0x2C2D31) : lv_color_hex(0x222327))
                                              : lv_color_hex(0xF3F4F6),
                                          0);
                lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], selected ? (lv_opa_t)102 : ((ad == 1) ? (lv_opa_t)88 : ((ad == 2) ? (lv_opa_t)58 : (lv_opa_t)34)), 0);
                lv_obj_set_style_border_color(settings_item_icon_bgs[i], theme_border, 0);
                lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
            }
        }

        if (settings_item_icons[i]) {
            bool compact_marker = compact_marker_view && (i < settings_compact_marker_count(settings_view));
            if (compact_marker) {
                const char *badge = settings_compact_marker_text(settings_view, i);
                if (badge[0] == '\0') {
                    lv_obj_add_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_style_text_font(settings_item_icons[i], &lv_font_inter_medium_22_ui, 0);
                    lv_obj_set_style_text_color(settings_item_icons[i], settings_compact_marker_text_color(settings_view, i), 0);
                    lv_obj_set_style_text_opa(settings_item_icons[i],
                                              wheel_list_view ? wheel_icon_opa
                                                              : (selected ? LV_OPA_COVER
                                                                          : ((ad == 1) ? (lv_opa_t)92 : ((ad == 2) ? (lv_opa_t)66 : (lv_opa_t)42))),
                                              0);
                    lv_label_set_text(settings_item_icons[i], badge);
                    lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 0, 0);
                }
            } else {
                lv_obj_clear_flag(settings_item_icons[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
            }
            lv_color_t icon_color = list_cards_view ? lv_color_hex(ui_accent_color_hex)
                                              : (selected ? lv_color_hex(ui_accent_color_hex) : lv_color_hex(0x8A8A91));
            if (!compact_marker) {
                lv_obj_set_style_text_color(settings_item_icons[i], icon_color, 0);
                lv_obj_set_style_text_opa(settings_item_icons[i],
                                          wheel_list_view ? wheel_icon_opa
                                                          : (selected ? LV_OPA_COVER
                                                                      : ((ad == 1) ? (lv_opa_t)92 : ((ad == 2) ? (lv_opa_t)66 : (lv_opa_t)42))),
                                          0);
                lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 1, 1);
            }
        }

        if (settings_item_titles[i]) {
            const lv_font_t *title_font = selected ? &lv_font_inter_medium_22_ui : &lv_font_inter_medium_16_ui;
            lv_obj_set_style_text_font(settings_item_titles[i], title_font, 0);
            lv_obj_set_style_text_color(settings_item_titles[i], list_cards_view ? theme_text_primary
                                                                            : (selected ? theme_text_primary : theme_text_secondary),
                                        0);
            lv_obj_set_style_text_opa(settings_item_titles[i],
                                      wheel_list_view ? wheel_title_opa
                                                      : (selected ? LV_OPA_COVER
                                                                  : ((ad == 1) ? (lv_opa_t)88 : ((ad == 2) ? (lv_opa_t)54 : (lv_opa_t)34))),
                                      0);
            lv_obj_align(settings_item_titles[i], LV_ALIGN_LEFT_MID, text_x,
                         (compact_marker_view || filter_view) ? 0 : ((slider_view && i == 0) ? -54 : -18));
            lv_obj_set_width(settings_item_titles[i], item_w - 140);
            lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_LEFT, 0);
        }

        if (settings_item_values[i]) {
            if (filter_view) {
                lv_obj_add_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(settings_item_values[i], LV_OBJ_FLAG_HIDDEN);
                if (settings_item_value_is_icon[i]) {
                    if (accent_view && i < DISPLAY_ACCENT_COUNT) {
                        lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(accent_palette_hex[i]), 0);
                    } else {
                        lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(ui_accent_color_hex), 0);
                    }
                    lv_obj_set_style_text_opa(settings_item_values[i],
                                              wheel_list_view ? wheel_value_opa
                                                              : (selected ? LV_OPA_COVER : (lv_opa_t)80),
                                              0);
                    lv_obj_align(settings_item_values[i], LV_ALIGN_RIGHT_MID, -22, 0);
                } else {
                    const lv_font_t *value_font = &lv_font_inter_medium_16_ui;
                    lv_obj_set_style_text_font(settings_item_values[i], value_font, 0);
                    lv_obj_set_style_text_color(settings_item_values[i], list_cards_view ? theme_text_secondary
                                                                                    : (selected ? theme_text_primary : theme_text_secondary),
                                                0);
                    lv_obj_set_style_text_opa(settings_item_values[i],
                                              wheel_list_view ? wheel_value_opa
                                                              : (selected ? LV_OPA_COVER
                                                                          : ((ad == 1) ? (lv_opa_t)86 : ((ad == 2) ? (lv_opa_t)52 : (lv_opa_t)32))),
                                              0);
                    lv_obj_align(settings_item_values[i], LV_ALIGN_LEFT_MID, text_x,
                                 compact_marker_view ? 0 : ((slider_view && i == 0) ? -8 : 22));
                }
            }
        }

        if (settings_item_arrows[i]) {
            lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
        }

        if (settings_item_arrows[i]) {
            lv_obj_set_style_text_color(settings_item_arrows[i],
                                        list_cards_view ? theme_text_secondary
                                                  : (selected ? theme_text_primary : theme_text_secondary),
                                        0);
            lv_obj_set_style_text_opa(settings_item_arrows[i],
                                      wheel_list_view ? wheel_arrow_opa
                                                      : (selected ? (lv_opa_t)98 : ((ad == 1) ? (lv_opa_t)74 : ((ad == 2) ? (lv_opa_t)46 : (lv_opa_t)28))),
                                      0);
            lv_obj_align(settings_item_arrows[i], LV_ALIGN_RIGHT_MID, arrow_x, 0);
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
                    lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_22_ui, 0);
                    lv_obj_set_style_text_color(settings_item_titles[i], theme_text_secondary, 0);
                    lv_obj_set_style_text_align(settings_item_titles[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_set_width(settings_item_titles[i], 280);
                    lv_obj_align(settings_item_titles[i], LV_ALIGN_TOP_MID, 0, 36);
                }
                if (settings_item_values[i]) {
                    lv_obj_set_style_text_font(settings_item_values[i], &lv_font_inter_medium_145_num, 0);
                    lv_obj_set_style_text_color(settings_item_values[i], theme_text_primary, 0);
                    lv_obj_set_style_text_align(settings_item_values[i], LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_set_width(settings_item_values[i], 280);
                    lv_obj_align(settings_item_values[i], LV_ALIGN_TOP_MID, 0, 98);
                }
            }
        }

        if (visible || (list_cards_view && i == wheel_preload_index)) {
            lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_focus_frame) {
        if (list_cards_view && !value_panel_view && !filter_reset_view) {
            lv_obj_clear_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(settings_focus_frame, card_w, card_h);
            lv_obj_align(settings_focus_frame, LV_ALIGN_TOP_MID, list_x_off, center_y);
            lv_obj_move_foreground(settings_focus_frame);
        } else {
            lv_obj_add_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_blur_top_overlay) {
        lv_obj_add_flag(settings_blur_top_overlay, LV_OBJ_FLAG_HIDDEN);
    }
    if (settings_blur_bottom_overlay) {
        lv_obj_add_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_HIDDEN);
    }

    if (settings_dim_overlay) {
        lv_obj_add_flag(settings_dim_overlay, LV_OBJ_FLAG_HIDDEN);
    }

    if (settings_value_slider_track && settings_value_slider_fill && settings_value_slider_knob) {
        bool show_slider = slider_view;
        if (show_slider) {
            lv_obj_clear_flag(settings_value_slider_track, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_value_slider_fill, LV_OBJ_FLAG_HIDDEN);
            if (value_panel_view) {
                lv_obj_set_size(settings_value_slider_track, 316, 16);
                lv_obj_align(settings_value_slider_track, LV_ALIGN_TOP_MID, 0, 316);
                lv_obj_set_style_bg_color(settings_value_slider_track, theme_slider_track, 0);
            } else {
                lv_obj_clear_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
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
            } else if (filter_lifetime_panel_view) {
                min_value = FILTER_LIFETIME_MIN_HOURS;
                max_value = FILTER_LIFETIME_MAX_HOURS;
                current = filter_lifetime_hours;
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
            lv_obj_set_style_radius(settings_value_slider_fill,
                                    LV_RADIUS_CIRCLE,
                                    0);
            lv_coord_t fill_w = (lv_coord_t)((track_w * pos + 500) / 1000);
            if (fill_w < 0) {
                fill_w = 0;
            }
            if (fill_w > track_w) {
                fill_w = track_w;
            }
            lv_obj_set_width(settings_value_slider_fill, fill_w);

            if (settings_value_slider_knob) {
                lv_obj_add_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
            }

        } else {
            lv_obj_add_flag(settings_value_slider_track, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_value_slider_fill, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (label_settings_title) {
        if (value_panel_view || filter_reset_view || confirm_view || about_view) {
            lv_obj_add_flag(label_settings_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(label_settings_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(label_settings_title);
        }
    }

    if (settings_sensitivity_title && settings_sensitivity_value && settings_sensitivity_subtitle) {
        if (value_panel_view) {
            char value_buf[12];
            int panel_value = 0;
            if (settings_view == SETTINGS_VIEW_SENSITIVITY) {
                panel_value = speed_sensitivity_clicks;
                snprintf(value_buf, sizeof(value_buf), "%d", panel_value);
            } else if (filter_lifetime_panel_view) {
                panel_value = filter_lifetime_hours;
                snprintf(value_buf, sizeof(value_buf), "%d", panel_value);
            } else {
                panel_value = display_brightness_percent;
                snprintf(value_buf, sizeof(value_buf), "%d", panel_value);
            }
            lv_label_set_text(settings_sensitivity_value, value_buf);
            lv_label_set_text(settings_sensitivity_title,
                              (settings_view == SETTINGS_VIEW_SENSITIVITY) ? ui_tr("SENSITIVITY", "ЧУВСТВИТ.") :
                              (filter_lifetime_panel_view ? ui_tr("FILTER LIFETIME", "РЕСУРС ФИЛЬТРА")
                                                          : ui_tr("BRIGHTNESS", "ЯРКОСТЬ")));
            lv_label_set_text(settings_sensitivity_subtitle,
                              (settings_view == SETTINGS_VIEW_SENSITIVITY) ? ui_tr("CLICKS PER 100%", "КЛИКОВ НА 100%") :
                              (filter_lifetime_panel_view ? ui_tr("TOTAL HOURS", "ВСЕГО ЧАСОВ")
                                                          : ui_tr("PERCENT", "ПРОЦЕНТ")));
            lv_obj_align(settings_sensitivity_title, LV_ALIGN_TOP_MID, 0, 74);
            lv_obj_align(settings_sensitivity_value, LV_ALIGN_TOP_MID, 0, 120);
            lv_obj_set_style_text_font(settings_sensitivity_subtitle, &lv_font_inter_medium_16_ui, 0);
            lv_obj_align(settings_sensitivity_subtitle, LV_ALIGN_TOP_MID, 0, 266);
            lv_obj_clear_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(settings_sensitivity_title);
            lv_obj_move_foreground(settings_sensitivity_subtitle);
            lv_obj_move_foreground(settings_sensitivity_value);
            if (settings_value_slider_track) {
                lv_obj_set_size(settings_value_slider_track, 316, 16);
                lv_obj_align(settings_value_slider_track, LV_ALIGN_TOP_MID, 0, 316);
                lv_obj_set_style_bg_color(settings_value_slider_track, theme_slider_track, 0);
                lv_obj_set_style_bg_opa(settings_value_slider_track, LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_value_slider_track);
            }
            if (settings_value_slider_fill) {
                lv_obj_set_style_radius(settings_value_slider_fill, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_value_slider_fill,
                                          filter_lifetime_panel_view ? lv_color_hex(ui_accent_color_hex)
                                                                     : lv_color_hex(ui_accent_color_hex),
                                          0);
                lv_obj_set_style_bg_opa(settings_value_slider_fill, LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_value_slider_fill);
            }
            if (settings_value_slider_knob) {
                lv_obj_add_flag(settings_value_slider_knob, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            lv_obj_add_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_sensitivity_unit) {
        if (filter_lifetime_panel_view) {
            lv_obj_set_style_text_font(settings_sensitivity_unit, &lv_font_inter_semibold_28_ui, 0);
            lv_obj_set_style_text_color(settings_sensitivity_unit, theme_text_primary, 0);
            lv_obj_clear_flag(settings_sensitivity_unit, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_parent(settings_sensitivity_unit, settings_screen);
            lv_obj_update_layout(settings_screen);
            lv_coord_t val_x = lv_obj_get_x(settings_sensitivity_value);
            lv_coord_t val_w = lv_obj_get_width(settings_sensitivity_value);
            lv_coord_t val_y = lv_obj_get_y(settings_sensitivity_value);
            lv_obj_set_pos(settings_sensitivity_unit, val_x + val_w + 8, val_y + 0);
            lv_obj_move_foreground(settings_sensitivity_unit);
        } else {
            lv_obj_add_flag(settings_sensitivity_unit, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_filter_used_arc && settings_filter_used_label &&
        settings_filter_used_value && settings_filter_used_unit &&
        settings_filter_used_subtitle) {
        if (filter_reset_view) {
            int used_src = settings_filter_reset_animating ? settings_filter_reset_anim_value : filter_used_hours;
            int used = clamp_int(used_src, 0, filter_lifetime_hours);
            int total = (filter_lifetime_hours > 0) ? filter_lifetime_hours : 1;
            int val = (used * 1000) / total;
            val = clamp_int(val, 0, 1000);
            lv_obj_align(settings_filter_used_arc, LV_ALIGN_TOP_MID, 0, 36);
            lv_obj_set_style_bg_color(settings_filter_used_arc, theme_panel_bg, 0);
            lv_obj_set_style_bg_opa(settings_filter_used_arc, LV_OPA_COVER, 0);
            lv_obj_set_style_arc_width(settings_filter_used_arc, 13, LV_PART_MAIN);
            lv_obj_set_style_arc_color(settings_filter_used_arc, theme_panel_ring, LV_PART_MAIN);
            lv_obj_set_style_arc_opa(settings_filter_used_arc, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_arc_width(settings_filter_used_arc, 13, LV_PART_INDICATOR);
            lv_obj_set_style_arc_color(settings_filter_used_arc, lv_color_hex(ui_accent_color_hex), LV_PART_INDICATOR);
            lv_obj_set_style_arc_opa(settings_filter_used_arc, LV_OPA_COVER, LV_PART_INDICATOR);
            lv_obj_set_size(settings_filter_used_arc, 300, 300);
            lv_arc_set_value(settings_filter_used_arc, val);

            char used_buf[16];
            snprintf(used_buf, sizeof(used_buf), "%d", used);
            lv_label_set_text(settings_filter_used_value, used_buf);
            char of_buf[32];
            snprintf(of_buf, sizeof(of_buf), ui_tr("of %dh", "из %dч"), total);
            lv_label_set_text(settings_filter_used_subtitle, of_buf);
            lv_obj_set_style_text_font(settings_filter_used_subtitle, &lv_font_inter_medium_16_ui, 0);
            lv_obj_set_style_text_color(settings_filter_used_label, theme_text_secondary, 0);
            lv_obj_set_style_text_color(settings_filter_used_unit, theme_text_secondary, 0);
            lv_obj_set_style_text_color(settings_filter_used_subtitle, theme_text_secondary, 0);
            lv_obj_align(settings_filter_used_label, LV_ALIGN_TOP_MID, 0, 76);
            lv_obj_align(settings_filter_used_value, LV_ALIGN_TOP_MID, 0, 124);
            lv_obj_set_parent(settings_filter_used_unit, settings_screen);
            lv_obj_set_style_text_font(settings_filter_used_unit, &lv_font_inter_semibold_28_ui, 0);
            lv_obj_update_layout(settings_screen);
            lv_coord_t used_x = lv_obj_get_x(settings_filter_used_value);
            lv_coord_t used_w = lv_obj_get_width(settings_filter_used_value);
            lv_coord_t used_y = lv_obj_get_y(settings_filter_used_value);
            lv_obj_set_pos(settings_filter_used_unit, used_x + used_w + 8, used_y + 12);
            lv_obj_align(settings_filter_used_subtitle, LV_ALIGN_TOP_MID, 0, 262);

            lv_obj_clear_flag(settings_filter_used_arc, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_filter_used_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_filter_used_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_filter_used_unit, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_filter_used_subtitle, LV_OBJ_FLAG_HIDDEN);

            lv_obj_move_foreground(settings_filter_used_arc);
            lv_obj_move_foreground(settings_filter_used_label);
            lv_obj_move_foreground(settings_filter_used_value);
            lv_obj_move_foreground(settings_filter_used_unit);
            lv_obj_move_foreground(settings_filter_used_subtitle);
        } else {
            lv_obj_add_flag(settings_filter_used_arc, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_filter_used_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_filter_used_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_filter_used_unit, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_filter_used_subtitle, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_about_title &&
        settings_about_model_caption && settings_about_model_value &&
        settings_about_fw_caption && settings_about_fw_value &&
        settings_about_if_caption && settings_about_if_value) {
        if (about_view) {
            const lv_color_t about_accent = lv_color_hex(ui_accent_color_hex);
            const lv_color_t about_primary = display_theme_dark ? lv_color_white() : lv_color_hex(0x18181B);
            const lv_color_t about_secondary = display_theme_dark ? lv_color_hex(0x71717A) : lv_color_hex(0x52525B);

            if (settings_overlay_icon_bg) {
                lv_obj_clear_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(settings_overlay_icon_bg, 132, 132);
                lv_obj_align(settings_overlay_icon_bg, LV_ALIGN_TOP_MID, 0, 38);
                lv_obj_set_style_radius(settings_overlay_icon_bg, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(settings_overlay_icon_bg, lv_color_hex(0x14161D), 0);
                lv_obj_set_style_bg_opa(settings_overlay_icon_bg, LV_OPA_COVER, 0);
                lv_obj_set_style_border_width(settings_overlay_icon_bg, 3, 0);
                lv_obj_set_style_border_color(settings_overlay_icon_bg, about_accent, 0);
                lv_obj_set_style_border_opa(settings_overlay_icon_bg, LV_OPA_COVER, 0);
                lv_obj_move_foreground(settings_overlay_icon_bg);
            }
            if (settings_overlay_icon) {
                lv_obj_clear_flag(settings_overlay_icon, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(settings_overlay_icon, "A2");
                lv_obj_set_style_text_font(settings_overlay_icon, &lv_font_montserrat_48, 0);
                lv_obj_set_style_text_color(settings_overlay_icon, about_accent, 0);
                lv_obj_set_style_text_opa(settings_overlay_icon, LV_OPA_COVER, 0);
                lv_obj_set_style_text_letter_space(settings_overlay_icon, 1, 0);
                lv_obj_center(settings_overlay_icon);
            }

            lv_obj_clear_flag(settings_about_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_about_model_caption, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_about_model_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_about_fw_caption, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(settings_about_fw_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_if_caption, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_if_value, LV_OBJ_FLAG_HIDDEN);

            lv_label_set_text(settings_about_title, "BELOM A2");
            lv_obj_set_style_text_font(settings_about_title, &lv_font_montserrat_48, 0);
            lv_obj_set_style_text_color(settings_about_title, about_primary, 0);
            lv_obj_set_style_text_letter_space(settings_about_title, 1, 0);
            lv_obj_set_style_text_align(settings_about_title, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_width(settings_about_title, 360);
            lv_obj_align(settings_about_title, LV_ALIGN_TOP_MID, 0, 202);

            lv_label_set_text(settings_about_model_caption, ui_tr("FW", "ПРОШ"));
            lv_obj_set_style_text_font(settings_about_model_caption, &lv_font_inter_medium_22_ui, 0);
            lv_obj_set_style_text_color(settings_about_model_caption, about_secondary, 0);
            lv_obj_set_style_text_letter_space(settings_about_model_caption, 2, 0);
            lv_obj_align(settings_about_model_caption, LV_ALIGN_TOP_MID, -78, 314);

            lv_label_set_text(settings_about_model_value, "2.2.1");
            lv_obj_set_style_text_font(settings_about_model_value, &lv_font_montserrat_48, 0);
            lv_obj_set_style_text_color(settings_about_model_value, about_primary, 0);
            lv_obj_set_style_text_letter_space(settings_about_model_value, 1, 0);
            lv_obj_align(settings_about_model_value, LV_ALIGN_TOP_MID, -78, 340);

            lv_label_set_text(settings_about_fw_caption, ui_tr("UI", "ИНТ"));
            lv_obj_set_style_text_font(settings_about_fw_caption, &lv_font_inter_medium_22_ui, 0);
            lv_obj_set_style_text_color(settings_about_fw_caption, about_secondary, 0);
            lv_obj_set_style_text_letter_space(settings_about_fw_caption, 2, 0);
            lv_obj_align(settings_about_fw_caption, LV_ALIGN_TOP_MID, 78, 314);

            lv_label_set_text(settings_about_fw_value, "1.2.1");
            lv_obj_set_style_text_font(settings_about_fw_value, &lv_font_montserrat_48, 0);
            lv_obj_set_style_text_color(settings_about_fw_value, about_primary, 0);
            lv_obj_set_style_text_letter_space(settings_about_fw_value, 1, 0);
            lv_obj_align(settings_about_fw_value, LV_ALIGN_TOP_MID, 78, 340);

            lv_obj_move_foreground(settings_about_title);
            lv_obj_move_foreground(settings_about_model_caption);
            lv_obj_move_foreground(settings_about_model_value);
            lv_obj_move_foreground(settings_about_fw_caption);
            lv_obj_move_foreground(settings_about_fw_value);
            if (settings_overlay_icon_bg) {
                lv_obj_move_foreground(settings_overlay_icon_bg);
            }
        } else {
            lv_obj_add_flag(settings_about_title, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_model_caption, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_model_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_fw_caption, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_fw_value, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_if_caption, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(settings_about_if_value, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

