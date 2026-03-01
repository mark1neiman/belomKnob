static void create_settings_screen(void)
{
    settings_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(settings_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(settings_screen);

    label_settings_title = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(label_settings_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_settings_title, lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX), 0);
    lv_obj_set_style_text_letter_space(label_settings_title, 2, 0);
    lv_obj_align(label_settings_title, LV_ALIGN_TOP_MID, 0, 38);

    settings_dim_overlay = lv_obj_create(settings_screen);
    lv_obj_remove_style_all(settings_dim_overlay);
    lv_obj_clear_flag(settings_dim_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_dim_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_dim_overlay, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
    lv_obj_set_pos(settings_dim_overlay, 0, 0);
    lv_obj_set_style_bg_color(settings_dim_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(settings_dim_overlay, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(settings_dim_overlay, LV_OBJ_FLAG_HIDDEN);

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
    lv_obj_set_style_text_font(settings_overlay_title, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_color(settings_overlay_title, lv_color_white(), 0);
    lv_obj_set_style_text_align(settings_overlay_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_title, 320);
    lv_obj_add_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_line1 = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_line1, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_overlay_line1, lv_color_hex(0xC3CAD6), 0);
    lv_obj_set_style_text_align(settings_overlay_line1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_line1, 320);
    lv_obj_add_flag(settings_overlay_line1, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_line2 = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_line2, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_overlay_line2, lv_color_hex(0x9CA3AF), 0);
    lv_obj_set_style_text_align(settings_overlay_line2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_line2, 320);
    lv_obj_add_flag(settings_overlay_line2, LV_OBJ_FLAG_HIDDEN);

    settings_overlay_line3 = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_overlay_line3, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_overlay_line3, lv_color_hex(0x7A7A83), 0);
    lv_obj_set_style_text_align(settings_overlay_line3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_overlay_line3, 320);
    lv_obj_add_flag(settings_overlay_line3, LV_OBJ_FLAG_HIDDEN);

    settings_blur_top_overlay = lv_obj_create(settings_screen);
    lv_obj_remove_style_all(settings_blur_top_overlay);
    lv_obj_clear_flag(settings_blur_top_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_blur_top_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(settings_blur_top_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(settings_blur_top_overlay, (lv_opa_t)SETTINGS_BLUR_BG_OPA, 0);
    lv_obj_set_style_border_width(settings_blur_top_overlay, 0, 0);
    lv_obj_set_style_radius(settings_blur_top_overlay, 30, 0);
    lv_obj_set_style_blur_radius(settings_blur_top_overlay, SETTINGS_BLUR_RADIUS, 0);
    lv_obj_set_style_blur_backdrop(settings_blur_top_overlay, false, 0);
    lv_obj_set_style_blur_quality(settings_blur_top_overlay, LV_BLUR_QUALITY_SPEED, 0);
    lv_obj_set_pos(settings_blur_top_overlay, 0, 0);
    lv_obj_add_flag(settings_blur_top_overlay, LV_OBJ_FLAG_HIDDEN);

    settings_blur_bottom_overlay = lv_obj_create(settings_screen);
    lv_obj_remove_style_all(settings_blur_bottom_overlay);
    lv_obj_clear_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(settings_blur_bottom_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(settings_blur_bottom_overlay, (lv_opa_t)SETTINGS_BLUR_BG_OPA, 0);
    lv_obj_set_style_border_width(settings_blur_bottom_overlay, 0, 0);
    lv_obj_set_style_radius(settings_blur_bottom_overlay, 30, 0);
    lv_obj_set_style_blur_radius(settings_blur_bottom_overlay, SETTINGS_BLUR_RADIUS, 0);
    lv_obj_set_style_blur_backdrop(settings_blur_bottom_overlay, false, 0);
    lv_obj_set_style_blur_quality(settings_blur_bottom_overlay, LV_BLUR_QUALITY_SPEED, 0);
    lv_obj_set_pos(settings_blur_bottom_overlay, 0, 0);
    lv_obj_add_flag(settings_blur_bottom_overlay, LV_OBJ_FLAG_HIDDEN);

    settings_slider_caption = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_slider_caption, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_slider_caption, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_align(settings_slider_caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_slider_caption, 300);
    lv_obj_add_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_title = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(settings_sensitivity_title, lv_color_hex(0xA1A1AA), 0);
    lv_obj_set_style_text_letter_space(settings_sensitivity_title, 2, 0);
    lv_label_set_text(settings_sensitivity_title, ui_tr("SENSITIVITY", "ЧУВСТВИТ."));
    lv_obj_align(settings_sensitivity_title, LV_ALIGN_TOP_MID, 0, 74);
    lv_obj_add_flag(settings_sensitivity_title, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_value = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_value, &lv_font_inter_medium_145_num, 0);
    lv_obj_set_style_text_color(settings_sensitivity_value, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(settings_sensitivity_value, -2, 0);
    lv_obj_set_style_text_align(settings_sensitivity_value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_sensitivity_value, LV_SIZE_CONTENT);
    lv_label_set_text(settings_sensitivity_value, "15");
    lv_obj_align(settings_sensitivity_value, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_add_flag(settings_sensitivity_value, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_subtitle = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_subtitle, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_sensitivity_subtitle, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_letter_space(settings_sensitivity_subtitle, 2, 0);
    lv_label_set_text(settings_sensitivity_subtitle, ui_tr("CLICKS PER 100%", "КЛИКОВ НА 100%"));
    lv_obj_align(settings_sensitivity_subtitle, LV_ALIGN_TOP_MID, 0, 266);
    lv_obj_add_flag(settings_sensitivity_subtitle, LV_OBJ_FLAG_HIDDEN);

    settings_sensitivity_unit = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_sensitivity_unit, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_color(settings_sensitivity_unit, lv_color_hex(0x6B7280), 0);
    lv_label_set_text(settings_sensitivity_unit, ui_tr("h", "ч"));
    lv_obj_align(settings_sensitivity_unit, LV_ALIGN_OUT_RIGHT_MID, 8, -16);
    lv_obj_add_flag(settings_sensitivity_unit, LV_OBJ_FLAG_HIDDEN);

    settings_filter_used_arc = lv_arc_create(settings_screen);
    lv_obj_set_size(settings_filter_used_arc, 300, 300);
    lv_obj_align(settings_filter_used_arc, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_radius(settings_filter_used_arc, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_filter_used_arc, lv_color_hex(0x18181B), 0);
    lv_obj_set_style_bg_opa(settings_filter_used_arc, LV_OPA_COVER, 0);
    lv_obj_set_style_arc_width(settings_filter_used_arc, 13, LV_PART_MAIN);
    lv_obj_set_style_arc_color(settings_filter_used_arc, lv_color_hex(0x27272A), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(settings_filter_used_arc, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_arc_width(settings_filter_used_arc, 13, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(settings_filter_used_arc, lv_color_hex(UI_ACCENT_COLOR_HEX), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(settings_filter_used_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(settings_filter_used_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_arc_set_bg_angles(settings_filter_used_arc, 0, 360);
    lv_arc_set_range(settings_filter_used_arc, 0, 1000);
    lv_arc_set_rotation(settings_filter_used_arc, 270);
    lv_arc_set_value(settings_filter_used_arc, 0);
    lv_obj_clear_flag(settings_filter_used_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(settings_filter_used_arc, LV_OBJ_FLAG_HIDDEN);

    settings_filter_used_label = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_filter_used_label, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_filter_used_label, lv_color_hex(0x7C8596), 0);
    lv_label_set_text(settings_filter_used_label, ui_tr("USED", "ИСПОЛЬЗ."));
    lv_obj_align(settings_filter_used_label, LV_ALIGN_TOP_MID, 0, 124);
    lv_obj_add_flag(settings_filter_used_label, LV_OBJ_FLAG_HIDDEN);

    settings_filter_used_value = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_filter_used_value, &lv_font_inter_medium_130_num, 0);
    lv_obj_set_style_text_color(settings_filter_used_value, lv_color_hex(UI_ACCENT_COLOR_HEX), 0);
    lv_obj_set_style_text_align(settings_filter_used_value, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_filter_used_value, LV_SIZE_CONTENT);
    lv_label_set_text(settings_filter_used_value, "0");
    lv_obj_align(settings_filter_used_value, LV_ALIGN_TOP_MID, 0, 146);
    lv_obj_add_flag(settings_filter_used_value, LV_OBJ_FLAG_HIDDEN);

    settings_filter_used_unit = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_filter_used_unit, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_color(settings_filter_used_unit, lv_color_hex(0x6B7280), 0);
    lv_label_set_text(settings_filter_used_unit, ui_tr("h", "ч"));
    lv_obj_align(settings_filter_used_unit, LV_ALIGN_OUT_RIGHT_MID, 8, -16);
    lv_obj_add_flag(settings_filter_used_unit, LV_OBJ_FLAG_HIDDEN);

    settings_filter_used_subtitle = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_filter_used_subtitle, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_filter_used_subtitle, lv_color_hex(0x9CA3AF), 0);
    lv_obj_set_style_text_align(settings_filter_used_subtitle, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(settings_filter_used_subtitle, 200);
    lv_label_set_text(settings_filter_used_subtitle, ui_tr("of 0h", "из 0ч"));
    lv_obj_align(settings_filter_used_subtitle, LV_ALIGN_TOP_MID, 0, 228);
    lv_obj_add_flag(settings_filter_used_subtitle, LV_OBJ_FLAG_HIDDEN);

    settings_about_title = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(settings_about_title, lv_color_hex(0xA1A1AA), 0);
    lv_obj_set_style_text_letter_space(settings_about_title, 2, 0);
    lv_label_set_text(settings_about_title, ui_tr("ABOUT", "О СИСТЕМЕ"));
    lv_obj_align(settings_about_title, LV_ALIGN_TOP_MID, 0, 74);
    lv_obj_add_flag(settings_about_title, LV_OBJ_FLAG_HIDDEN);

    settings_about_model_caption = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_model_caption, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_about_model_caption, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_letter_space(settings_about_model_caption, 2, 0);
    lv_label_set_text(settings_about_model_caption, ui_tr("MODEL", "МОДЕЛЬ"));
    lv_obj_align(settings_about_model_caption, LV_ALIGN_TOP_MID, 0, 122);
    lv_obj_add_flag(settings_about_model_caption, LV_OBJ_FLAG_HIDDEN);

    settings_about_model_value = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_model_value, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(settings_about_model_value, lv_color_white(), 0);
    lv_obj_set_style_text_align(settings_about_model_value, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(settings_about_model_value, "BELOM A2");
    lv_obj_align(settings_about_model_value, LV_ALIGN_TOP_MID, 0, 154);
    lv_obj_add_flag(settings_about_model_value, LV_OBJ_FLAG_HIDDEN);

    settings_about_fw_caption = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_fw_caption, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_about_fw_caption, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_letter_space(settings_about_fw_caption, 2, 0);
    lv_label_set_text(settings_about_fw_caption, ui_tr("FIRMWARE", "ПРОШИВКА"));
    lv_obj_align(settings_about_fw_caption, LV_ALIGN_TOP_LEFT, 96, 286);
    lv_obj_add_flag(settings_about_fw_caption, LV_OBJ_FLAG_HIDDEN);

    settings_about_fw_value = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_fw_value, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(settings_about_fw_value, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(settings_about_fw_value, 1, 0);
    lv_label_set_text(settings_about_fw_value, "2.2.1");
    lv_obj_align(settings_about_fw_value, LV_ALIGN_TOP_LEFT, 96, 318);
    lv_obj_add_flag(settings_about_fw_value, LV_OBJ_FLAG_HIDDEN);

    settings_about_if_caption = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_if_caption, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_about_if_caption, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_letter_space(settings_about_if_caption, 2, 0);
    lv_label_set_text(settings_about_if_caption, ui_tr("INTERFACE", "ИНТЕРФЕЙС"));
    lv_obj_align(settings_about_if_caption, LV_ALIGN_TOP_LEFT, 264, 286);
    lv_obj_add_flag(settings_about_if_caption, LV_OBJ_FLAG_HIDDEN);

    settings_about_if_value = lv_label_create(settings_screen);
    lv_obj_set_style_text_font(settings_about_if_value, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(settings_about_if_value, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(settings_about_if_value, 1, 0);
    lv_label_set_text(settings_about_if_value, "1.2.1");
    lv_obj_align(settings_about_if_value, LV_ALIGN_TOP_LEFT, 264, 318);
    lv_obj_add_flag(settings_about_if_value, LV_OBJ_FLAG_HIDDEN);

    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        settings_items[i] = lv_obj_create(settings_screen);
        lv_obj_clear_flag(settings_items[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(settings_items[i], 350, 98);
        lv_obj_set_style_bg_color(settings_items[i], lv_color_hex(0x17191E), 0);
        lv_obj_set_style_bg_opa(settings_items[i], (lv_opa_t)64, 0);
        lv_obj_set_style_border_width(settings_items[i], 1, 0);
        lv_obj_set_style_border_color(settings_items[i], lv_color_white(), 0);
        lv_obj_set_style_radius(settings_items[i], 30, 0);
        lv_obj_set_style_pad_left(settings_items[i], 24, 0);
        lv_obj_set_style_pad_right(settings_items[i], 16, 0);
        lv_obj_set_style_pad_top(settings_items[i], 10, 0);
        lv_obj_set_style_pad_bottom(settings_items[i], 10, 0);

        settings_item_icon_bgs[i] = lv_obj_create(settings_items[i]);
        lv_obj_clear_flag(settings_item_icon_bgs[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(settings_item_icon_bgs[i], 76, 76);
        lv_obj_align(settings_item_icon_bgs[i], LV_ALIGN_LEFT_MID, 8, 0);
        lv_obj_set_style_radius(settings_item_icon_bgs[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_item_icon_bgs[i], lv_color_white(), 0);
        lv_obj_set_style_bg_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_item_icon_bgs[i], 1, 0);
        lv_obj_set_style_border_color(settings_item_icon_bgs[i], lv_color_white(), 0);
        lv_obj_set_style_border_opa(settings_item_icon_bgs[i], LV_OPA_COVER, 0);

        lv_obj_set_style_pad_all(settings_item_icon_bgs[i], 0, 0);
        settings_item_icons[i] = lv_label_create(settings_item_icon_bgs[i]);
        lv_obj_set_style_text_font(settings_item_icons[i], ui_font_lucide_20(), 0);
        lv_obj_set_style_text_color(settings_item_icons[i], lv_color_hex(UI_TEXT_MUTED_HEX), 0);
        lv_obj_set_style_text_align(settings_item_icons[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(settings_item_icons[i], LV_ALIGN_CENTER, 1, 1);

        settings_item_titles[i] = lv_label_create(settings_items[i]);
        lv_obj_set_style_text_font(settings_item_titles[i], &lv_font_inter_medium_16_ui, 0);
        lv_obj_set_style_text_color(settings_item_titles[i], lv_color_hex(0xDCE2ED), 0);
        lv_obj_align(settings_item_titles[i], LV_ALIGN_LEFT_MID, 118, -16);
        settings_item_values[i] = lv_label_create(settings_items[i]);
        lv_obj_set_style_text_font(settings_item_values[i], &lv_font_inter_medium_16_ui, 0);
        lv_obj_set_style_text_color(settings_item_values[i], lv_color_hex(0x5D5D66), 0);
        lv_obj_set_style_text_opa(settings_item_values[i], (lv_opa_t)130, 0);
        lv_obj_align(settings_item_values[i], LV_ALIGN_LEFT_MID, 118, 20);
        settings_item_arrows[i] = lv_label_create(settings_items[i]);
        lv_obj_set_style_text_font(settings_item_arrows[i], ui_font_lucide_20(), 0);
        lv_obj_set_style_text_color(settings_item_arrows[i], lv_color_white(), 0);
        lv_obj_set_style_text_letter_space(settings_item_arrows[i], 0, 0);
        lv_label_set_text(settings_item_arrows[i], UI_ICON_LUCIDE_CHEVRON_RIGHT);
        lv_obj_set_width(settings_item_arrows[i], 64);
        lv_obj_set_style_text_align(settings_item_arrows[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(settings_item_arrows[i], LV_ALIGN_RIGHT_MID, -3, 0);
    }

    settings_focus_frame = lv_obj_create(settings_screen);
    lv_obj_clear_flag(settings_focus_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_focus_frame, 350, 98);
    lv_obj_set_style_radius(settings_focus_frame, 30, 0);
    lv_obj_set_style_bg_opa(settings_focus_frame, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(settings_focus_frame, 3, 0);
    lv_obj_set_style_border_color(settings_focus_frame, lv_color_white(), 0);
    lv_obj_set_style_border_opa(settings_focus_frame, LV_OPA_COVER, 0);
    lv_obj_add_flag(settings_focus_frame, LV_OBJ_FLAG_HIDDEN);
    settings_value_slider_track = lv_obj_create(settings_screen);
    lv_obj_remove_style_all(settings_value_slider_track);
    lv_obj_clear_flag(settings_value_slider_track, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(settings_value_slider_track, 316, 16);
    lv_obj_align(settings_value_slider_track, LV_ALIGN_TOP_MID, 0, 316);
    lv_obj_set_style_radius(settings_value_slider_track, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_value_slider_track, lv_color_hex(0x2D2F38), 0);
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

    settings_v2_layer = lv_obj_create(settings_screen);
    lv_obj_remove_style_all(settings_v2_layer);
    lv_obj_clear_flag(settings_v2_layer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_v2_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_v2_layer, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
    lv_obj_set_pos(settings_v2_layer, 0, 0);
    lv_obj_add_flag(settings_v2_layer, LV_OBJ_FLAG_HIDDEN);

    settings_v2_glow = lv_obj_create(settings_v2_layer);
    lv_obj_remove_style_all(settings_v2_glow);
    lv_obj_clear_flag(settings_v2_glow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_v2_glow, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_v2_glow, 326, 326);
    lv_obj_align(settings_v2_glow, LV_ALIGN_CENTER, 0, -64);
    lv_obj_set_style_radius(settings_v2_glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_v2_glow, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_bg_grad_color(settings_v2_glow, lv_color_black(), 0);
#if LV_USE_DRAW_SW_COMPLEX_GRADIENTS
    lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_RADIAL, 0);
    lv_obj_set_style_bg_opa(settings_v2_glow, (lv_opa_t)28, 0);
#else
    lv_obj_set_style_bg_grad_dir(settings_v2_glow, LV_GRAD_DIR_NONE, 0);
    lv_obj_set_style_bg_opa(settings_v2_glow, LV_OPA_TRANSP, 0);
#endif
    lv_obj_set_style_border_width(settings_v2_glow, 0, 0);

    settings_v2_center_circle = lv_obj_create(settings_v2_layer);
    lv_obj_clear_flag(settings_v2_center_circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_v2_center_circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_v2_center_circle, 225, 225);
    lv_obj_align(settings_v2_center_circle, LV_ALIGN_CENTER, 0, -64);
    lv_obj_set_style_radius(settings_v2_center_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(settings_v2_center_circle, 0, 0);
    lv_obj_set_style_bg_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_bg_grad_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_bg_grad_dir(settings_v2_center_circle, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(settings_v2_center_circle, (lv_opa_t)230, 0);
    lv_obj_set_style_shadow_color(settings_v2_center_circle, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_shadow_opa(settings_v2_center_circle, (lv_opa_t)58, 0);
    lv_obj_set_style_shadow_width(settings_v2_center_circle, 22, 0);
    lv_obj_set_style_shadow_spread(settings_v2_center_circle, 0, 0);
    lv_obj_set_style_shadow_ofs_y(settings_v2_center_circle, 0, 0);

    settings_v2_center_icon = lv_label_create(settings_v2_center_circle);
    lv_obj_set_style_text_font(settings_v2_center_icon, &lv_font_lucide_120_v2_icons, 0);
    lv_obj_set_style_text_color(settings_v2_center_icon, lv_color_white(), 0);
    lv_obj_center(settings_v2_center_icon);

    settings_v2_center_title = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_center_title, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_color(settings_v2_center_title, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(settings_v2_center_title, 2, 0);
    lv_obj_align(settings_v2_center_title, LV_ALIGN_CENTER, 0, 94);

    settings_v2_center_value = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_center_value, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(settings_v2_center_value, lv_color_hex(0xA1A1AA), 0);
    lv_obj_set_style_text_letter_space(settings_v2_center_value, 1, 0);
    lv_obj_align(settings_v2_center_value, LV_ALIGN_CENTER, 0, 130);

    settings_v2_left_circle = lv_obj_create(settings_v2_layer);
    lv_obj_clear_flag(settings_v2_left_circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_v2_left_circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_v2_left_circle, 86, 86);
    lv_obj_align(settings_v2_left_circle, LV_ALIGN_LEFT_MID, 16, -34);
    lv_obj_set_style_radius(settings_v2_left_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(settings_v2_left_circle, 2, 0);
    lv_obj_set_style_border_color(settings_v2_left_circle, lv_color_hex(0x3F3F46), 0);
    lv_obj_set_style_bg_color(settings_v2_left_circle, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(settings_v2_left_circle, (lv_opa_t)20, 0);

    settings_v2_left_icon = lv_label_create(settings_v2_left_circle);
    lv_obj_set_style_text_font(settings_v2_left_icon, &lv_font_lucide_48_v2_icons, 0);
    lv_obj_set_style_text_color(settings_v2_left_icon, lv_color_hex(0xA1A1AA), 0);
    lv_obj_center(settings_v2_left_icon);

    settings_v2_right_circle = lv_obj_create(settings_v2_layer);
    lv_obj_clear_flag(settings_v2_right_circle, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(settings_v2_right_circle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_v2_right_circle, 86, 86);
    lv_obj_align(settings_v2_right_circle, LV_ALIGN_RIGHT_MID, -16, -34);
    lv_obj_set_style_radius(settings_v2_right_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(settings_v2_right_circle, 2, 0);
    lv_obj_set_style_border_color(settings_v2_right_circle, lv_color_hex(0x3F3F46), 0);
    lv_obj_set_style_bg_color(settings_v2_right_circle, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(settings_v2_right_circle, (lv_opa_t)20, 0);

    settings_v2_right_icon = lv_label_create(settings_v2_right_circle);
    lv_obj_set_style_text_font(settings_v2_right_icon, &lv_font_lucide_48_v2_icons, 0);
    lv_obj_set_style_text_color(settings_v2_right_icon, lv_color_hex(0xA1A1AA), 0);
    lv_obj_center(settings_v2_right_icon);

    for (int i = 0; i < SETTINGS_V2_DOTS_COUNT; ++i) {
        settings_v2_dots[i] = lv_obj_create(settings_v2_layer);
        lv_obj_remove_style_all(settings_v2_dots[i]);
        lv_obj_remove_flag(settings_v2_dots[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(settings_v2_dots[i], 10, 10);
        lv_obj_set_style_radius(settings_v2_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_v2_dots[i], lv_color_hex(0x52525B), 0);
        lv_obj_set_style_bg_opa(settings_v2_dots[i], LV_OPA_COVER, 0);
    }

    settings_v2_arc = lv_arc_create(settings_v2_layer);
    lv_obj_set_size(settings_v2_arc, 438, 438);
    lv_obj_align(settings_v2_arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_arc_width(settings_v2_arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_width(settings_v2_arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(settings_v2_arc, lv_color_hex(0x1C1C1F), LV_PART_MAIN);
    lv_obj_set_style_arc_color(settings_v2_arc, lv_color_hex(ui_accent_color_hex), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(settings_v2_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_arc_set_bg_angles(settings_v2_arc, 0, 360);
    lv_arc_set_rotation(settings_v2_arc, 270);
    lv_arc_set_range(settings_v2_arc, 0, 1000);
    lv_arc_set_value(settings_v2_arc, 0);
    lv_obj_clear_flag(settings_v2_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(settings_v2_arc, LV_OBJ_FLAG_HIDDEN);

    settings_v2_arc_icon = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_arc_icon, &lv_font_lucide_48_v2_icons, 0);
    lv_obj_set_style_text_color(settings_v2_arc_icon, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_text_opa(settings_v2_arc_icon, LV_OPA_COVER, 0);
    lv_label_set_text(settings_v2_arc_icon, UI_ICON_LUCIDE_GAUGE);
    lv_obj_align(settings_v2_arc_icon, LV_ALIGN_CENTER, 0, -146);
    lv_obj_add_flag(settings_v2_arc_icon, LV_OBJ_FLAG_HIDDEN);

    settings_v2_arc_title = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_arc_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(settings_v2_arc_title, lv_color_hex(0xA1A1AA), 0);
    lv_obj_set_style_text_letter_space(settings_v2_arc_title, 2, 0);
    lv_label_set_text(settings_v2_arc_title, ui_tr("SENSITIVITY", "ЧУВСТВИТ."));
    lv_obj_align(settings_v2_arc_title, LV_ALIGN_CENTER, 0, -138);
    lv_obj_add_flag(settings_v2_arc_title, LV_OBJ_FLAG_HIDDEN);

    settings_v2_arc_value = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_arc_value, &lv_font_inter_bold_180_num, 0);
    lv_obj_set_style_text_color(settings_v2_arc_value, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(settings_v2_arc_value, -2, 0);
    lv_label_set_text(settings_v2_arc_value, "15");
    lv_obj_align(settings_v2_arc_value, LV_ALIGN_CENTER, 0, 28);
    lv_obj_add_flag(settings_v2_arc_value, LV_OBJ_FLAG_HIDDEN);

    settings_v2_arc_suffix = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_arc_suffix, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(settings_v2_arc_suffix, lv_color_hex(0x71717A), 0);
    lv_label_set_text(settings_v2_arc_suffix, "");
    lv_obj_add_flag(settings_v2_arc_suffix, LV_OBJ_FLAG_HIDDEN);

    settings_v2_arc_subtitle = lv_label_create(settings_v2_layer);
    lv_obj_set_style_text_font(settings_v2_arc_subtitle, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(settings_v2_arc_subtitle, lv_color_hex(0x5D5D66), 0);
    lv_obj_set_style_text_letter_space(settings_v2_arc_subtitle, 2, 0);
    lv_label_set_text(settings_v2_arc_subtitle, ui_tr("CLICKS PER 100%", "КЛИКОВ НА 100%"));
    lv_obj_align(settings_v2_arc_subtitle, LV_ALIGN_CENTER, 0, 74);
    lv_obj_add_flag(settings_v2_arc_subtitle, LV_OBJ_FLAG_HIDDEN);

#if UI_ENABLE_CUSTOM_ACCENT
    const float hue_ring_two_pi = 6.28318530718f;
    const lv_coord_t hue_ring_center_x = EXAMPLE_LCD_H_RES / 2;
    const lv_coord_t hue_ring_center_y = EXAMPLE_LCD_V_RES / 2;
    const lv_coord_t hue_ring_min_res = (EXAMPLE_LCD_H_RES < EXAMPLE_LCD_V_RES) ? EXAMPLE_LCD_H_RES : EXAMPLE_LCD_V_RES;
    const lv_coord_t hue_ring_radius = (hue_ring_min_res / 2) - 18;
    const lv_coord_t hue_seg_w = 10;
    const lv_coord_t hue_seg_h = 24;

    for (int i = 0; i < SETTINGS_CUSTOM_HUE_SEGMENT_COUNT; ++i) {
        settings_custom_hue_ring_segments[i] = lv_obj_create(settings_v2_layer);
        if (!settings_custom_hue_ring_segments[i]) {
            break;
        }
        lv_obj_remove_style_all(settings_custom_hue_ring_segments[i]);
        lv_obj_clear_flag(settings_custom_hue_ring_segments[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(settings_custom_hue_ring_segments[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(settings_custom_hue_ring_segments[i], hue_seg_w, hue_seg_h);
        lv_obj_set_style_radius(settings_custom_hue_ring_segments[i], 5, 0);
        lv_obj_set_style_bg_opa(settings_custom_hue_ring_segments[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_custom_hue_ring_segments[i], 0, 0);
        int seg_hue = (i * 360) / SETTINGS_CUSTOM_HUE_SEGMENT_COUNT;
        float angle_deg = (float)seg_hue - 90.0f;
        float angle = angle_deg * hue_ring_two_pi / 360.0f;
        lv_coord_t x = hue_ring_center_x + (lv_coord_t)lroundf(cosf(angle) * hue_ring_radius) - (hue_seg_w / 2);
        lv_coord_t y = hue_ring_center_y + (lv_coord_t)lroundf(sinf(angle) * hue_ring_radius) - (hue_seg_h / 2);
        lv_obj_set_pos(settings_custom_hue_ring_segments[i], x, y);
        lv_obj_set_style_transform_angle(settings_custom_hue_ring_segments[i], (int16_t)((angle_deg + 90.0f) * 10.0f), 0);
        lv_obj_set_style_bg_color(settings_custom_hue_ring_segments[i], lv_color_hex(accent_hex_from_hue(seg_hue)), 0);
        lv_obj_add_flag(settings_custom_hue_ring_segments[i], LV_OBJ_FLAG_HIDDEN);
    }

    settings_custom_hue_knob = lv_obj_create(settings_v2_layer);
    if (settings_custom_hue_knob) {
        lv_obj_remove_style_all(settings_custom_hue_knob);
        lv_obj_clear_flag(settings_custom_hue_knob, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(settings_custom_hue_knob, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(settings_custom_hue_knob, 30, 30);
        lv_obj_set_style_radius(settings_custom_hue_knob, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(settings_custom_hue_knob, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_bg_opa(settings_custom_hue_knob, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(settings_custom_hue_knob, 3, 0);
        lv_obj_set_style_border_color(settings_custom_hue_knob, lv_color_white(), 0);
        lv_obj_set_style_border_opa(settings_custom_hue_knob, LV_OPA_COVER, 0);
        lv_obj_set_style_shadow_color(settings_custom_hue_knob, lv_color_black(), 0);
        lv_obj_set_style_shadow_opa(settings_custom_hue_knob, (lv_opa_t)70, 0);
        lv_obj_set_style_shadow_width(settings_custom_hue_knob, 6, 0);
        lv_obj_set_style_shadow_spread(settings_custom_hue_knob, 0, 0);
        lv_obj_add_flag(settings_custom_hue_knob, LV_OBJ_FLAG_HIDDEN);
    }
#else
    for (int i = 0; i < SETTINGS_CUSTOM_HUE_SEGMENT_COUNT; ++i) {
        settings_custom_hue_ring_segments[i] = NULL;
    }
    settings_custom_hue_knob = NULL;
    settings_custom_hue_ui_ready = false;
#endif

    settings_enter_view(SETTINGS_VIEW_ROOT);
}

