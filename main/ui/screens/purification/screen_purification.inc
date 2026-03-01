static lv_obj_t *purification_ring_segments[UI_SEGMENT_COUNT] = {0};
static lv_timer_t *purification_ring_timer = NULL;
static int purification_ring_phase = 0;
static bool purification_ring_initialized = false;
static int purification_ring_prev_heads[5] = {-1, -1, -1, -1, -1};
#if LVGL_VERSION_MAJOR >= 9
extern const lv_image_dsc_t purify_wind_144;
#else
extern const lv_img_dsc_t purify_wind_144;
#endif

static void purification_ring_apply_phase(int phase)
{
    const lv_opa_t base_opa = (lv_opa_t)46;

    for (int i = 0; i < 5; ++i) {
        int idx = purification_ring_prev_heads[i];
        if (idx >= 0 && idx < UI_SEGMENT_COUNT && purification_ring_segments[idx]) {
            lv_obj_set_style_bg_opa(purification_ring_segments[idx], base_opa, 0);
        }
    }

    int head0 = phase % UI_SEGMENT_COUNT;
    int head1 = (phase + 1) % UI_SEGMENT_COUNT;
    int head2 = (phase + 2) % UI_SEGMENT_COUNT;
    int head3 = (phase + 3) % UI_SEGMENT_COUNT;
    int head4 = (phase + 4) % UI_SEGMENT_COUNT;

    if (purification_ring_segments[head0]) {
        lv_obj_set_style_bg_opa(purification_ring_segments[head0], LV_OPA_COVER, 0);
    }
    if (purification_ring_segments[head1]) {
        lv_obj_set_style_bg_opa(purification_ring_segments[head1], (lv_opa_t)212, 0);
    }
    if (purification_ring_segments[head2]) {
        lv_obj_set_style_bg_opa(purification_ring_segments[head2], (lv_opa_t)168, 0);
    }
    if (purification_ring_segments[head3]) {
        lv_obj_set_style_bg_opa(purification_ring_segments[head3], (lv_opa_t)136, 0);
    }
    if (purification_ring_segments[head4]) {
        lv_obj_set_style_bg_opa(purification_ring_segments[head4], (lv_opa_t)108, 0);
    }

    purification_ring_prev_heads[0] = head0;
    purification_ring_prev_heads[1] = head1;
    purification_ring_prev_heads[2] = head2;
    purification_ring_prev_heads[3] = head3;
    purification_ring_prev_heads[4] = head4;
}

static void purification_ring_reset_animation(void)
{
    const lv_opa_t base_opa = (lv_opa_t)46;
    purification_ring_phase = 0;
    purification_ring_initialized = true;
    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        if (purification_ring_segments[i]) {
            lv_obj_set_style_bg_opa(purification_ring_segments[i], base_opa, 0);
        }
    }
    for (int i = 0; i < 5; ++i) {
        purification_ring_prev_heads[i] = -1;
    }
    purification_ring_apply_phase(purification_ring_phase);
}

static void purification_ring_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!purification_screen ||
        lv_obj_has_flag(purification_screen, LV_OBJ_FLAG_HIDDEN) ||
        !is_purifying ||
        ui_screen != UI_SCREEN_PURIFICATION) {
        return;
    }
    if (!purification_ring_initialized) {
        purification_ring_reset_animation();
        return;
    }

    purification_ring_phase = (purification_ring_phase + 1) % UI_SEGMENT_COUNT;
    purification_ring_apply_phase(purification_ring_phase);
}

static void create_purification_screen(void)
{
    const lv_color_t purify_accent = lv_color_hex(0x14D3A7);
    const float two_pi = 6.28318530718f;
    const lv_coord_t center_x = (EXAMPLE_LCD_H_RES / 2) + UI_MAIN_CENTER_OFFSET_X;
    const lv_coord_t center_y = (EXAMPLE_LCD_V_RES / 2) + UI_MAIN_CENTER_OFFSET_Y;
    const lv_coord_t ring_radius = (EXAMPLE_LCD_V_RES / 2) - 6;
    const lv_coord_t dash_w = 18;
    const lv_coord_t dash_h = 14;

    purification_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(purification_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(purification_screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(purification_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(purification_screen, 0, 0);

    purification_spinner = NULL;

    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        purification_ring_segments[i] = NULL;
    }
    purification_ring_initialized = false;
    for (int i = 0; i < 5; ++i) {
        purification_ring_prev_heads[i] = -1;
    }

    for (int i = 0; i < UI_SEGMENT_COUNT; ++i) {
        lv_obj_t *seg = lv_obj_create(purification_screen);
        purification_ring_segments[i] = seg;
        lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(seg, dash_w, dash_h);
        lv_obj_set_style_radius(seg, 3, 0);
        lv_obj_set_style_border_width(seg, 0, 0);
        lv_obj_set_style_bg_color(seg, purify_accent, 0);
        lv_obj_set_style_bg_opa(seg, (lv_opa_t)46, 0);

        float t = ((float)i / (float)UI_SEGMENT_COUNT);
        float angle = t * two_pi;
        float angle_deg = t * 360.0f;
        lv_coord_t x = center_x + (lv_coord_t)(sinf(angle) * ring_radius) - (dash_w / 2);
        lv_coord_t y = center_y - (lv_coord_t)(cosf(angle) * ring_radius) - (dash_h / 2);
        lv_obj_set_pos(seg, x, y);
        lv_obj_set_style_transform_angle(seg, (int16_t)(angle_deg * 10.0f), 0);
    }

    if (purification_ring_timer) {
        lv_timer_del(purification_ring_timer);
        purification_ring_timer = NULL;
    }
    purification_ring_timer = lv_timer_create(purification_ring_timer_cb, 20, NULL);
    if (purification_ring_timer) {
        lv_timer_pause(purification_ring_timer);
    }
    purification_ring_reset_animation();

#if LVGL_VERSION_MAJOR >= 9
    purification_spinner_icon = lv_image_create(purification_screen);
    lv_image_set_src(purification_spinner_icon, &purify_wind_144);
#else
    purification_spinner_icon = lv_img_create(purification_screen);
    lv_img_set_src(purification_spinner_icon, &purify_wind_144);
#endif
    lv_obj_align(purification_spinner_icon, LV_ALIGN_CENTER, 0, -98);

    label_purification_title = lv_label_create(purification_screen);
    lv_obj_set_style_text_font(label_purification_title, &lv_font_inter_semibold_28_ui, 0);
    lv_obj_set_style_text_color(label_purification_title, purify_accent, 0);
    lv_obj_set_style_text_letter_space(label_purification_title, 2, 0);
    lv_label_set_text(label_purification_title, ui_tr("PURIFYING", "ОЧИЩЕНИЕ"));
    lv_obj_align(label_purification_title, LV_ALIGN_CENTER, 0, 31);

    label_purification_state = NULL;

    label_purification_hint = lv_label_create(purification_screen);
    lv_obj_set_style_text_font(label_purification_hint, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_purification_hint, lv_color_hex(0xA1A1AA), 0);
    lv_obj_set_style_text_letter_space(label_purification_hint, 0, 0);
    lv_obj_set_width(label_purification_hint, 320);
    lv_obj_set_style_text_align(label_purification_hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_purification_hint, ui_tr("DOUBLE PRESS TO EXIT", "ДВА НАЖАТИЯ: ВЫХОД"));
    lv_obj_align(label_purification_hint, LV_ALIGN_CENTER, 0, 108);
}


