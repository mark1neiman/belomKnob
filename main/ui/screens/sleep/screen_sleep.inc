static void create_sleep_screen(void)
{
    sleep_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(sleep_screen, LV_OBJ_FLAG_SCROLLABLE);
    apply_dark_screen_style(sleep_screen);
}


