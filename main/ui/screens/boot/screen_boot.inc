static void create_boot_screen(void)
{
    boot_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(boot_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(boot_screen);
    lv_obj_set_style_bg_color(boot_screen, lv_color_hex(UI_BG_HEX), 0);

    const float two_pi = 6.28318530718f;
    const lv_coord_t center_x = (EXAMPLE_LCD_H_RES / 2) + UI_SPEED_CENTER_OFFSET_X;
    const lv_coord_t center_y = (EXAMPLE_LCD_V_RES / 2) + UI_SPEED_CENTER_OFFSET_Y;
    const lv_coord_t ring_radius = UI_BOOT_RING_RADIUS + 3; /* Match speed screen geometry */
    const lv_coord_t seg_h = UI_SEGMENT_H + 4;
    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lv_obj_t *seg = lv_obj_create(boot_screen);
        boot_ring_segments[i] = seg;
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
    lv_obj_align(boot_logo, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, -16 + UI_SPEED_CENTER_OFFSET_Y);
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
    lv_obj_align(boot_logo, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, -16 + UI_SPEED_CENTER_OFFSET_Y);
#endif
    set_boot_theme_mode(display_theme_dark, 0, -1);

    boot_subtitle = lv_label_create(boot_screen);
    lv_label_set_text(boot_subtitle, "BELOM  A2");
    lv_obj_set_style_text_font(boot_subtitle, &lv_font_inter_medium_22_caps, 0);
    lv_obj_set_style_text_letter_space(boot_subtitle, 1, 0);
    lv_obj_set_style_text_color(boot_subtitle, lv_color_hex(UI_BOOT_SUBTITLE_COLOR_FROM_HEX), 0);
    lv_obj_align(boot_subtitle, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, 96 + UI_SPEED_CENTER_OFFSET_Y);

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
    lv_obj_align(boot_progress_track, LV_ALIGN_CENTER, UI_SPEED_CENTER_OFFSET_X, 114 + UI_SPEED_CENTER_OFFSET_Y);
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
    lv_obj_set_style_bg_color(boot_progress_fill, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_bg_opa(boot_progress_fill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(boot_progress_fill, 0, 0);
    lv_obj_set_style_pad_all(boot_progress_fill, 0, 0);

    update_boot_screen_visual(0);
}


