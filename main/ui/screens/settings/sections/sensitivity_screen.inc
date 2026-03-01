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
    snprintf(buf, sizeof(buf), ui_tr("%d clicks", "%d кликов"), get_sensitivity_from_roller());
    lv_label_set_text(label_sensitivity_value, buf);
}

static void create_sensitivity_screen(void)
{
    sensitivity_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(sensitivity_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(sensitivity_screen);

    label_sensitivity_title = lv_label_create(sensitivity_screen);
    lv_obj_set_style_text_font(label_sensitivity_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_sensitivity_title, lv_color_white(), 0);
    lv_label_set_text(label_sensitivity_title, ui_tr("Sensitivity", "Чувствит."));
    lv_obj_align(label_sensitivity_title, LV_ALIGN_TOP_MID, 0, 12);

    label_sensitivity_value = lv_label_create(sensitivity_screen);
    lv_obj_set_style_text_font(label_sensitivity_value, &lv_font_inter_medium_16_ui, 0);
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


