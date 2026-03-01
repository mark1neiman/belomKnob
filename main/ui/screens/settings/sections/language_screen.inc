static void create_language_screen(void)
{
    language_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(language_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(language_screen);

    label_language_title = lv_label_create(language_screen);
    lv_obj_set_style_text_font(label_language_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_language_title, lv_color_white(), 0);
    lv_label_set_text(label_language_title, ui_strings[current_lang].language);
    lv_obj_align(label_language_title, LV_ALIGN_TOP_MID, 0, 12);

    roller_language = lv_roller_create(language_screen);
    lv_obj_set_width(roller_language, 220);
    lv_obj_align(roller_language, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(roller_language, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_language, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_language, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_language, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_language, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_language, lv_color_white(), LV_PART_SELECTED);
    lv_obj_set_style_text_font(roller_language, &lv_font_inter_medium_16_ui, LV_PART_MAIN);
    lv_obj_set_style_text_font(roller_language, &lv_font_inter_medium_16_ui, LV_PART_SELECTED);

    rebuild_language_options();
    lv_roller_set_options(roller_language, language_roller_options, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_language, 4);
    lv_roller_set_selected(roller_language, current_lang, LV_ANIM_OFF);
}


