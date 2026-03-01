static void create_main_screen(void)
{
    main_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(main_screen);

    main_bg_glow_outer = NULL;
    main_bg_glow_inner = NULL;
    main_bg_glow_core = NULL;
    main_center_disc = NULL;

    build_segment_ring(main_screen);

    label_speed = lv_label_create(main_screen);
    lv_obj_set_style_text_font(label_speed, &lv_font_inter_bold_180_num, 0);
    lv_obj_set_style_text_letter_space(label_speed, -4, 0);
    lv_obj_set_style_text_color(label_speed, lv_color_white(), 0);
    lv_obj_set_style_text_align(label_speed, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_speed, EXAMPLE_LCD_H_RES);
    lv_obj_align(label_speed, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, UI_SPEED_CENTER_OFFSET_Y - 12);
    lv_obj_move_foreground(label_speed);

    label_speed_caption = NULL;

    lv_obj_t *status_row = lv_obj_create(main_screen);
    lv_obj_remove_style_all(status_row);
    lv_obj_clear_flag(status_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(status_row, 196, 40);
    lv_obj_align(status_row, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, UI_SPEED_CENTER_OFFSET_Y + 106);
    lv_obj_set_flex_flow(status_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(status_row, 12, 0);
    lv_obj_set_style_pad_right(status_row, 12, 0);
    lv_obj_set_style_pad_top(status_row, 0, 0);
    lv_obj_set_style_pad_bottom(status_row, 0, 0);
    lv_obj_set_style_pad_column(status_row, 8, 0);
    lv_obj_set_style_radius(status_row, 18, 0);
    lv_obj_set_style_bg_color(status_row, lv_color_hex(UI_FILTER_GOOD_HEX), 0);
    lv_obj_set_style_bg_opa(status_row, (lv_opa_t)38, 0);
    lv_obj_set_style_border_width(status_row, 1, 0);
    lv_obj_set_style_border_color(status_row, lv_color_hex(UI_FILTER_GOOD_HEX), 0);
    lv_obj_set_style_border_opa(status_row, LV_OPA_COVER, 0);
    status_divider = status_row;

    label_lock_icon = NULL;
    label_lock_status = NULL;

    label_filter_icon = lv_obj_create(status_row);
    lv_obj_clear_flag(label_filter_icon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(label_filter_icon, 10, 10);
    lv_obj_set_style_radius(label_filter_icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(label_filter_icon, 0, 0);
    lv_obj_set_style_bg_color(label_filter_icon, lv_color_hex(UI_FILTER_GOOD_HEX), 0);
    lv_obj_set_style_bg_opa(label_filter_icon, LV_OPA_COVER, 0);

    label_filter_status = lv_label_create(status_row);
    lv_obj_set_style_text_font(label_filter_status, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_filter_status, lv_color_hex(UI_FILTER_GOOD_HEX), 0);
    lv_obj_set_style_text_letter_space(label_filter_status, 1, 0);
    lv_label_set_text(label_filter_status, ui_tr("FILTER", "ФИЛЬТР"));

    lock_overlay = NULL;

    lv_obj_t *overdrive_badge = lv_obj_create(main_screen);
    lv_obj_remove_style_all(overdrive_badge);
    lv_obj_clear_flag(overdrive_badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(overdrive_badge, 196, 40);
    lv_obj_align(overdrive_badge, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, UI_SPEED_CENTER_OFFSET_Y + UI_MAIN_OVERDRIVE_Y - 10);
    lv_obj_set_flex_flow(overdrive_badge, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(overdrive_badge, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(overdrive_badge, 12, 0);
    lv_obj_set_style_pad_right(overdrive_badge, 12, 0);
    lv_obj_set_style_pad_top(overdrive_badge, 0, 0);
    lv_obj_set_style_pad_bottom(overdrive_badge, 0, 0);
    lv_obj_set_style_radius(overdrive_badge, 18, 0);
    lv_obj_set_style_bg_color(overdrive_badge, lv_color_hex(0xE10600), 0);
    lv_obj_set_style_bg_opa(overdrive_badge, (lv_opa_t)38, 0);
    lv_obj_set_style_border_width(overdrive_badge, 1, 0);
    lv_obj_set_style_border_color(overdrive_badge, lv_color_hex(0xE10600), 0);
    lv_obj_set_style_border_opa(overdrive_badge, LV_OPA_COVER, 0);
    lv_obj_add_flag(overdrive_badge, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *overdrive_text = lv_label_create(overdrive_badge);
    lv_obj_set_style_text_font(overdrive_text, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(overdrive_text, lv_color_hex(0xE10600), 0);
    lv_obj_set_style_text_letter_space(overdrive_text, 2, 0);
    lv_label_set_text(overdrive_text, ui_tr("OVERDRIVE", "ОВЕРДРАЙВ"));

    /* Reuse existing handle for show/hide logic in update_main_ui(). */
    label_overdrive_warning = overdrive_badge;

    lv_anim_t overdrive_pulse;
    lv_anim_init(&overdrive_pulse);
    lv_anim_set_var(&overdrive_pulse, overdrive_badge);
    lv_anim_set_exec_cb(&overdrive_pulse, set_obj_opa_anim);
    lv_anim_set_values(&overdrive_pulse, (int32_t)LV_OPA_COVER, (int32_t)96);
    lv_anim_set_time(&overdrive_pulse, 1400);
    lv_anim_set_playback_time(&overdrive_pulse, 1400);
    lv_anim_set_repeat_count(&overdrive_pulse, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&overdrive_pulse, lv_anim_path_ease_in_out);
    lv_anim_start(&overdrive_pulse);

    main_ring_active_count = -1;
    main_ring_boundary_index = -1;
    fan_speed_arc_displayed = fan_speed_percent;
    update_segment_ring(fan_speed_percent, false);
    update_status_row();
}


