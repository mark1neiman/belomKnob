static void lock_ring_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!lock_screen) {
        return;
    }
    if (lv_obj_has_flag(lock_screen, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    static int phase = 0;
    static bool initialized = false;
    static int prev_heads[5] = {-1, -1, -1, -1, -1};
    const lv_opa_t base_opa = (lv_opa_t)46;
    if (!initialized) {
        initialized = true;
        for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
            if (lock_ring_segments[i]) {
                lv_obj_set_style_bg_opa(lock_ring_segments[i], base_opa, 0);
            }
        }
    }

    /* Restore only previous trail segments to base opacity to keep updates cheap. */
    for (int i = 0; i < 5; ++i) {
        int idx = prev_heads[i];
        if (idx >= 0 && idx < UI_SEGMENT_COUNT && lock_ring_segments[idx]) {
            lv_obj_set_style_bg_opa(lock_ring_segments[idx], base_opa, 0);
        }
    }

    phase = (phase + 1) % UI_SEGMENT_COUNT;
    int head0 = phase;
    int head1 = (phase + 1) % UI_SEGMENT_COUNT;
    int head2 = (phase + 2) % UI_SEGMENT_COUNT;
    int head3 = (phase + 3) % UI_SEGMENT_COUNT;
    int head4 = (phase + 4) % UI_SEGMENT_COUNT;

    if (lock_ring_segments[head0]) {
        lv_obj_set_style_bg_opa(lock_ring_segments[head0], LV_OPA_COVER, 0);
    }
    if (lock_ring_segments[head1]) {
        lv_obj_set_style_bg_opa(lock_ring_segments[head1], (lv_opa_t)212, 0);
    }
    if (lock_ring_segments[head2]) {
        lv_obj_set_style_bg_opa(lock_ring_segments[head2], (lv_opa_t)168, 0);
    }
    if (lock_ring_segments[head3]) {
        lv_obj_set_style_bg_opa(lock_ring_segments[head3], (lv_opa_t)136, 0);
    }
    if (lock_ring_segments[head4]) {
        lv_obj_set_style_bg_opa(lock_ring_segments[head4], (lv_opa_t)108, 0);
    }

    prev_heads[0] = head0;
    prev_heads[1] = head1;
    prev_heads[2] = head2;
    prev_heads[3] = head3;
    prev_heads[4] = head4;
}

static void create_lock_screen(void)
{
    const lv_coord_t lock_center_offset_x = -6;
    const lv_coord_t lock_center_offset_y = -10;
    const lv_coord_t lock_ring_offset_x = -5;
    const lv_coord_t lock_ring_offset_y = 2;
    const lv_coord_t lock_icon_offset_x = 4;

    lock_screen = lv_obj_create(lv_layer_top());
    lv_obj_clear_flag(lock_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(lock_screen, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
    lv_obj_set_pos(lock_screen, 0, 0);
    lv_obj_set_style_bg_color(lock_screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lock_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(lock_screen, 0, 0);

    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lock_ring_segments[i] = NULL;
    }
    if (lock_ring_timer) {
        lv_timer_del(lock_ring_timer);
        lock_ring_timer = NULL;
    }

    lock_ring_container = NULL;

    const float two_pi = 6.28318530718f;
    const lv_coord_t center_x = (EXAMPLE_LCD_H_RES / 2) + lock_center_offset_x + lock_ring_offset_x;
    const lv_coord_t center_y = (EXAMPLE_LCD_V_RES / 2) + lock_center_offset_y + lock_ring_offset_y;
    const lv_coord_t ring_radius = (EXAMPLE_LCD_V_RES / 2) - 6;
    const lv_coord_t dash_w = 18;
    const lv_coord_t dash_h = 14;

    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lv_obj_t *seg = lv_obj_create(lock_screen);
        lock_ring_segments[i] = seg;
        lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(seg, dash_w, dash_h);
        lv_obj_set_style_radius(seg, 3, 0);
        lv_obj_set_style_border_width(seg, 0, 0);
        lv_obj_set_style_bg_color(seg, lv_color_hex(ui_accent_color_hex), 0);
        lv_obj_set_style_bg_opa(seg, (lv_opa_t)42, 0);

        float t = ((float)i / (float)UI_SEGMENT_COUNT);
        float angle = t * two_pi;
        float angle_deg = t * 360.0f;
        lv_coord_t x = center_x + (lv_coord_t)(sinf(angle) * ring_radius) - (dash_w / 2);
        lv_coord_t y = center_y - (lv_coord_t)(cosf(angle) * ring_radius) - (dash_h / 2);
        lv_obj_set_pos(seg, x, y);
        lv_obj_set_style_transform_angle(seg, (int16_t)(angle_deg * 10.0f), 0);
    }

    if (lock_ring_timer) {
        lv_timer_del(lock_ring_timer);
        lock_ring_timer = NULL;
    }
    /* Faster lock-ring update for snappier motion while keeping smooth trail. */
    lock_ring_timer = lv_timer_create(lock_ring_timer_cb, 20, NULL);

    lock_screen_icon = lv_label_create(lock_screen);
    lv_obj_set_style_text_font(lock_screen_icon, &lv_font_lucide_162_lock, 0);
    lv_obj_set_style_text_color(lock_screen_icon, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_text_opa(lock_screen_icon, (lv_opa_t)210, 0);
    lv_label_set_text(lock_screen_icon, UI_ICON_LUCIDE_LOCK);
    lv_obj_align(lock_screen_icon, LV_ALIGN_CENTER, lock_center_offset_x + lock_ring_offset_x + lock_icon_offset_x, -2 + lock_center_offset_y);
    lock_screen_icon_shackle = NULL;
    lock_screen_icon_shackle_cut = NULL;
    lock_screen_icon_body = NULL;

    lock_screen_title = lv_label_create(lock_screen);
    lv_obj_set_style_text_font(lock_screen_title, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_color(lock_screen_title, lv_color_hex(ui_accent_color_hex), 0);
    lv_obj_set_style_text_letter_space(lock_screen_title, 2, 0);
    lv_label_set_text(lock_screen_title, "");
    lv_obj_align(lock_screen_title, LV_ALIGN_CENTER, lock_center_offset_x + 17, 132 + lock_center_offset_y);

    lock_screen_hint = NULL;
    lv_obj_add_flag(lock_screen, LV_OBJ_FLAG_HIDDEN);
}
