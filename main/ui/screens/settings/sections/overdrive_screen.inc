static void create_overdrive_screen(void)
{
    overdrive_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(overdrive_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(overdrive_screen);

    label_overdrive_title = lv_label_create(overdrive_screen);
    lv_obj_set_style_text_font(label_overdrive_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_overdrive_title, lv_color_white(), 0);
    lv_label_set_text(label_overdrive_title, ui_tr("Overdrive", "Овердрайв"));
    lv_obj_align(label_overdrive_title, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *desc = lv_label_create(overdrive_screen);
    lv_obj_set_style_text_font(desc, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(desc, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
    lv_label_set_text(desc, ui_tr("Allows 110% when filter is critical",
                                  "Разрешает 110% при критическом фильтре"));
    lv_obj_align(desc, LV_ALIGN_TOP_MID, 0, 48);

    roller_overdrive = lv_roller_create(overdrive_screen);
    lv_obj_set_width(roller_overdrive, 220);
    lv_obj_align(roller_overdrive, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(roller_overdrive, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_overdrive, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_overdrive, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_overdrive, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_overdrive, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_overdrive, lv_color_white(), LV_PART_SELECTED);
    lv_roller_set_options(roller_overdrive,
                          ui_tr("Disabled\nEnabled", "Выключено\nВключено"),
                          LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(roller_overdrive, 3);
    lv_roller_set_selected(roller_overdrive, allow_overdrive ? 1 : 0, LV_ANIM_OFF);
}


