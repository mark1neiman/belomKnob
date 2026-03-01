static void create_owner_screen(void)
{
    owner_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(owner_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(owner_screen);

    label_owner_title = lv_label_create(owner_screen);
    lv_obj_set_style_text_font(label_owner_title, &lv_font_inter_medium_22_ui, 0);
    lv_obj_set_style_text_color(label_owner_title, lv_color_white(), 0);
    lv_label_set_text(label_owner_title, ui_strings[current_lang].owner_name);
    lv_obj_align(label_owner_title, LV_ALIGN_TOP_MID, 0, 12);

    label_owner_value = lv_label_create(owner_screen);
    lv_label_set_text(label_owner_value, owner_name_len ? owner_name : "-");
    lv_obj_set_style_text_font(label_owner_value, &lv_font_inter_medium_16_ui, 0);
    lv_obj_set_style_text_color(label_owner_value, lv_color_white(), 0);
    lv_obj_align(label_owner_value, LV_ALIGN_TOP_MID, 0, 46);

    roller_owner = lv_roller_create(owner_screen);
    lv_obj_set_width(roller_owner, 200);
    lv_obj_align(roller_owner, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(roller_owner, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(roller_owner, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(roller_owner, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_owner, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_owner, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_owner, lv_color_white(), LV_PART_SELECTED);
    lv_roller_set_visible_row_count(roller_owner, 4);
    lv_roller_set_options(roller_owner,
                          "A\nB\nC\nD\nE\nF\nG\nH\nI\nJ\nK\nL\nM\nN\nO\nP\nQ\nR\nS\nT\nU\nV\nW\nX\nY\nZ\n"
                          "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl\nm\nn\no\np\nq\nr\ns\nt\nu\nv\nw\nx\ny\nz\n"
                          "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n"
                          "пробел\n<-\nСохранить",
                          LV_ROLLER_MODE_NORMAL);
}


