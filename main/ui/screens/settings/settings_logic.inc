static const char *settings_get_view_title(void)
{
    switch (settings_view) {
    case SETTINGS_VIEW_SENSITIVITY:
        return ui_tr("SENSITIVITY", "ЧУВСТВИТ.");
    case SETTINGS_VIEW_ACCENT:
        return ui_tr("ACCENT COLOR", "ЦВЕТ АКЦЕНТА");
    case SETTINGS_VIEW_LANGUAGE:
        return ui_tr("LANGUAGE", "ЯЗЫК");
    case SETTINGS_VIEW_BRIGHTNESS:
        return ui_tr("BRIGHTNESS", "ЯРКОСТЬ");
    case SETTINGS_VIEW_THEME:
        return ui_tr("THEME", "ТЕМА");
    case SETTINGS_VIEW_ROTATION:
        return ui_tr("ROTATION", "ПОВОРОТ");
    case SETTINGS_VIEW_LOCK_MODE:
        return ui_tr("LOCK MODE", "РЕЖИМ ЗАМКА");
    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return ui_tr("OVERDRIVE WARNING", "ОВЕРДРАЙВ: ВНИМ.");
    case SETTINGS_VIEW_OVERDRIVE_MODE:
        return ui_tr("OVERDRIVE MODE", "РЕЖИМ ОВЕРДРАЙВ");
    case SETTINGS_VIEW_FILTER:
        return settings_edit_mode ? ui_tr("FILTER  EDIT", "ФИЛЬТР  НАСТР.")
                                  : ui_tr("FILTER", "ФИЛЬТР");
    case SETTINGS_VIEW_FILTER_RESET:
        return ui_tr("FILTER USAGE", "РЕСУРС ФИЛЬТРА");
    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return ui_tr("AIR PURIFICATION", "ОЧИСТКА ВОЗДУХА");
    case SETTINGS_VIEW_ABOUT:
        return ui_tr("ABOUT", "О СИСТЕМЕ");
    case SETTINGS_VIEW_ROOT:
    default:
        return ui_tr("SETTINGS", "НАСТРОЙКИ");
    }
}

static int settings_get_item_count(void)
{
    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        return SETTINGS_ROOT_ITEMS_COUNT;
    case SETTINGS_VIEW_SENSITIVITY:
        return 1;
    case SETTINGS_VIEW_ACCENT:
        return DISPLAY_ACCENT_OPTIONS_COUNT;
    case SETTINGS_VIEW_LANGUAGE:
        return SETTINGS_LANG_OPTIONS_COUNT;
    case SETTINGS_VIEW_BRIGHTNESS:
        return 1;
    case SETTINGS_VIEW_THEME:
        return 2;
    case SETTINGS_VIEW_ROTATION:
        return 2;
    case SETTINGS_VIEW_LOCK_MODE:
        return 2;
    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return 2;
    case SETTINGS_VIEW_OVERDRIVE_MODE:
        return 2;
    case SETTINGS_VIEW_FILTER:
        return settings_filter_reset_confirm_open ? 2 : 3;
    case SETTINGS_VIEW_FILTER_RESET:
        return 2;
    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return 2;
    case SETTINGS_VIEW_ABOUT:
        return 1;
    default:
        return 1;
    }
}

static bool settings_item_has_arrow(int index)
{
    (void)index;
    return false;
}

static const ui_lang_t settings_language_order[SETTINGS_LANG_OPTIONS_COUNT] = {
    LANG_EN,
    LANG_ET,
    LANG_RU,
    LANG_LV,
    LANG_FI,
    LANG_DE,
    LANG_ES,
    LANG_FR,
    LANG_UK,
    LANG_PL,
    LANG_LT,
    LANG_IT,
};

static ui_lang_t settings_lang_from_option_index(int index)
{
    if (index < 0 || index >= SETTINGS_LANG_OPTIONS_COUNT) {
        return LANG_EN;
    }
    return settings_language_order[index];
}

static int settings_option_index_from_lang(ui_lang_t lang)
{
    for (int i = 0; i < SETTINGS_LANG_OPTIONS_COUNT; ++i) {
        if (settings_language_order[i] == lang) {
            return i;
        }
    }
    return 0;
}

static const char *settings_get_item_title(int index)
{
    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (index) {
        case 0: return ui_tr("Filter", "Фильтр");
        case 1: return ui_tr("Sensitivity", "Чувствит.");
        case 2: return ui_tr("Overdrive Mode", "Режим Овердрайв");
        case 3: return ui_tr("Accent Color", "Цвет акцента");
        case 4: return ui_tr("Language", "Язык");
        case 5: return ui_tr("Brightness", "Яркость");
        case 6: return ui_tr("Lock Mode", "Режим замка");
        case 7: return ui_tr("Rotation", "Поворот");
        case 8: return ui_tr("Theme", "Тема");
        case 9: return ui_tr("Air Purification", "Очистка воздуха");
        default: return ui_tr("About", "О системе");
        }

    case SETTINGS_VIEW_SENSITIVITY:
        return (index == 0) ? ui_tr("Sensitivity", "Чувствит.") : ui_tr("Back", "Назад");

    case SETTINGS_VIEW_ACCENT:
        return accent_palette_name(index);

    case SETTINGS_VIEW_LANGUAGE:
        if (index >= 0 && index < SETTINGS_LANG_OPTIONS_COUNT) {
            ui_lang_t lang = settings_lang_from_option_index(index);
            return language_names[lang];
        }
        return "";

    case SETTINGS_VIEW_BRIGHTNESS:
        return (index == 0) ? ui_tr("Brightness", "Яркость") : ui_tr("Back", "Назад");

    case SETTINGS_VIEW_THEME:
        switch (index) {
        case 0: return ui_tr("Dark", "Темная");
        case 1: return ui_tr("Light", "Светлая");
        default: return "";
        }

    case SETTINGS_VIEW_ROTATION:
        switch (index) {
        case 0: return ui_tr("0 DEG", "0°");
        case 1: return ui_tr("180 DEG", "180°");
        default: return "";
        }

    case SETTINGS_VIEW_LOCK_MODE:
        switch (index) {
        case 0: return ui_tr("Auto Lock", "Автоблок.");
        case 1: return ui_tr("Unlocked", "Открыто");
        default: return "";
        }

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return (index == 0) ? ui_tr("Back", "Назад") : ui_tr("Continue", "Продолжить");

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        switch (index) {
        case 0: return ui_tr("Enabled", "Включено");
        case 1: return ui_tr("Disabled", "Выключено");
        default: return "";
        }

    case SETTINGS_VIEW_FILTER:
        if (settings_filter_reset_confirm_open) {
            return (index == 0) ? ui_tr("Back", "Назад") : ui_tr("Reset usage", "Сбросить ресурс");
        }
        switch (index) {
        case 0: return ui_tr("Set lifetime", "Срок службы");
        case 1: return ui_tr("Reset usage", "Сбросить ресурс");
        default: return ui_tr("Back", "Назад");
        }

    case SETTINGS_VIEW_FILTER_RESET:
        return (index == 0) ? "" : ui_tr("RESET", "СБРОС");

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return (index == 0) ? ui_tr("Back", "Назад") : ui_tr("Start", "Старт");

    case SETTINGS_VIEW_ABOUT:
        switch (index) {
        case 0: return ui_tr("Model", "Модель");
        case 1: return ui_tr("Firmware", "Прошивка");
        case 2: return ui_tr("Interface", "Интерфейс");
        default: return ui_tr("Back", "Назад");
        }

    default:
        return "";
    }
}

static const char *settings_get_item_icon(int index)
{
    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (index) {
        case 0: return UI_ICON_LUCIDE_FILTER;
        case 1: return UI_ICON_LUCIDE_GAUGE;
        case 2: return UI_ICON_LUCIDE_ZAP;
        case 3: return UI_ICON_LUCIDE_PALETTE;
        case 4: return UI_ICON_LUCIDE_GLOBE;
        case 5: return UI_ICON_LUCIDE_SUN;
        case 6: return UI_ICON_LUCIDE_LOCK;
        case 7: return UI_ICON_LUCIDE_ROTATE_CW;
        case 8: return UI_ICON_LUCIDE_MOON;
        case 9: return UI_ICON_LUCIDE_WIND;
        default: return UI_ICON_LUCIDE_INFO;
        }

    case SETTINGS_VIEW_SENSITIVITY:
        return (index == 0) ? UI_ICON_LUCIDE_GAUGE : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_ACCENT:
        return "";

    case SETTINGS_VIEW_LANGUAGE:
        return "";

    case SETTINGS_VIEW_BRIGHTNESS:
        return (index == 0) ? UI_ICON_LUCIDE_SUN : UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_THEME:
        return "";

    case SETTINGS_VIEW_ROTATION:
        return UI_ICON_LUCIDE_ROTATE_CW;

    case SETTINGS_VIEW_LOCK_MODE:
        return "";

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        return (index == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_ALERT_TRIANGLE;

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        return UI_ICON_LUCIDE_ZAP;

    case SETTINGS_VIEW_FILTER:
        if (settings_filter_reset_confirm_open) {
            return (index == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_REFRESH_CW;
        }
        if (index == 0) {
            return UI_ICON_LUCIDE_FILTER;
        }
        if (index == 1) {
            return UI_ICON_LUCIDE_REFRESH_CW;
        }
        return UI_ICON_LUCIDE_CHEVRON_LEFT;

    case SETTINGS_VIEW_FILTER_RESET:
        return (index == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_REFRESH_CW;

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        return (index == 0) ? UI_ICON_LUCIDE_CHEVRON_LEFT : UI_ICON_LUCIDE_WIND;

    case SETTINGS_VIEW_ABOUT:
        return (index < 3) ? UI_ICON_LUCIDE_INFO : UI_ICON_LUCIDE_CHEVRON_LEFT;

    default:
        return UI_ICON_LUCIDE_INFO;
    }
}

static void settings_get_item_value(int index, char *buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return;
    }
    buf[0] = '\0';

    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (index) {
        case 0:
            snprintf(buf, buf_len, ui_tr("%dh / %dh", "%dч / %dч"), filter_used_hours, filter_lifetime_hours);
            break;
        case 1:
            snprintf(buf, buf_len, ui_tr("%d clicks", "%d кликов"), speed_sensitivity_clicks);
            break;
        case 2:
            snprintf(buf, buf_len, "%s", allow_overdrive ? ui_tr("Enabled", "Включено")
                                                         : ui_tr("Disabled", "Выключено"));
            break;
        case 3:
            snprintf(buf, buf_len, "%s",
                     (display_accent_index < DISPLAY_ACCENT_COUNT)
                         ? accent_palette_name(display_accent_index)
                         : ui_tr("Custom", "Польз."));
            break;
        case 4:
            snprintf(buf, buf_len, "%s", language_names[current_lang]);
            break;
        case 5:
            snprintf(buf, buf_len, "%d%%", display_brightness_percent);
            break;
        case 6:
            snprintf(buf, buf_len, "%s", allow_child_lock ? ui_tr("Auto lock", "Автоблок.")
                                                          : ui_tr("Unlocked", "Разблок."));
            break;
        case 7:
            snprintf(buf, buf_len, ui_tr("%d deg", "%d°"), display_rotation_deg % 360);
            break;
        case 8:
            snprintf(buf, buf_len, "%s", display_theme_dark ? ui_tr("Dark", "Темная")
                                                            : ui_tr("Light", "Светлая"));
            break;
        case 9:
            snprintf(buf, buf_len, "%s", is_purifying ? ui_tr("Running", "Работает")
                                                      : ui_tr("Clean air mode", "Режим очистки"));
            break;
        default:
            snprintf(buf, buf_len, ui_tr("FW 2.2.1", "ПРОШ 2.2.1"));
            break;
        }
        break;

    case SETTINGS_VIEW_SENSITIVITY:
        if (index == 0) {
            snprintf(buf, buf_len, "%d", speed_sensitivity_clicks);
        }
        break;

    case SETTINGS_VIEW_ACCENT:
        snprintf(buf, buf_len, "%s", (index == display_accent_index) ? UI_ICON_LUCIDE_CHECK : "");
        break;

    case SETTINGS_VIEW_LANGUAGE:
        if (index < SETTINGS_LANG_OPTIONS_COUNT) {
            int selected = (current_lang == settings_lang_from_option_index(index));
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_BRIGHTNESS:
        if (index == 0) {
            snprintf(buf, buf_len, "%d", display_brightness_percent);
        }
        break;

    case SETTINGS_VIEW_THEME:
        if (index < 2) {
            bool selected = (display_theme_dark && index == 0) || (!display_theme_dark && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_ROTATION:
        if (index < 2) {
            bool selected = ((display_rotation_deg % 360) == 0 && index == 0) ||
                            ((display_rotation_deg % 360) == 180 && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_LOCK_MODE:
        if (index < 2) {
            bool selected = (allow_child_lock && index == 0) || (!allow_child_lock && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        if (index == 1) {
            snprintf(buf, buf_len, "%s", ui_tr("Motor wear warning", "Износ мотора!"));
        }
        break;

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        if (index < 2) {
            bool selected = (allow_overdrive && index == 0) || (!allow_overdrive && index == 1);
            snprintf(buf, buf_len, "%s", selected ? UI_ICON_LUCIDE_CHECK : "");
        }
        break;

    case SETTINGS_VIEW_FILTER:
        if (settings_filter_reset_confirm_open) {
            break;
        }
        if (index == 0) {
            snprintf(buf, buf_len, ui_tr("%dh", "%dч"), filter_lifetime_hours);
        } else if (index == 1) {
            snprintf(buf, buf_len, ui_tr("used %dh", "исп. %dч"), filter_used_hours);
        }
        break;

    case SETTINGS_VIEW_FILTER_RESET:
        break;

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        if (index == 1) {
            snprintf(buf, buf_len, "%s", ui_tr("Clean air mode", "Режим очистки"));
        }
        break;

    case SETTINGS_VIEW_ABOUT:
        if (index == 0) {
            snprintf(buf, buf_len, "BELOM A2");
        } else if (index == 1) {
            snprintf(buf, buf_len, "2.2.1");
        } else if (index == 2) {
            snprintf(buf, buf_len, "1.2.1");
        }
        break;

    default:
        break;
    }
}

static void settings_refresh_view(void)
{
    const bool filter_lifetime_panel_view = (settings_view == SETTINGS_VIEW_FILTER) &&
                                            settings_edit_mode &&
                                            (settings_index == 0);
    const bool value_panel_view = (settings_view == SETTINGS_VIEW_SENSITIVITY) ||
                                  (settings_view == SETTINGS_VIEW_BRIGHTNESS) ||
                                  filter_lifetime_panel_view;
    const bool slider_view = value_panel_view;
    const bool overdrive_warning_view = (settings_view == SETTINGS_VIEW_OVERDRIVE_WARNING);
    const bool purification_confirm_view = (settings_view == SETTINGS_VIEW_PURIFICATION_CONFIRM);
    const bool confirm_view = overdrive_warning_view || purification_confirm_view;

    settings_items_count = settings_get_item_count();
    if (settings_items_count < 1) {
        settings_items_count = 1;
    }
    settings_index = clamp_int(settings_index, 0, settings_items_count - 1);

    if (label_settings_title) {
        lv_label_set_text(label_settings_title, settings_get_view_title());
    }

    char value_buf[48];
    for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
        settings_item_value_is_icon[i] = false;
        if (value_panel_view) {
            if (settings_items[i]) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }
        if (i >= settings_items_count) {
            if (settings_items[i]) {
                lv_obj_add_flag(settings_items[i], LV_OBJ_FLAG_HIDDEN);
            }
            continue;
        }

        if (settings_item_icons[i]) {
            lv_obj_set_style_transform_zoom(settings_item_icons[i], 256, 0);
            lv_label_set_text(settings_item_icons[i], settings_get_item_icon(i));
        }

        if (settings_item_titles[i]) {
            lv_label_set_text(settings_item_titles[i], settings_get_item_title(i));
        }

        if (settings_item_values[i]) {
            settings_get_item_value(i, value_buf, sizeof(value_buf));
            lv_label_set_text(settings_item_values[i], value_buf);
            bool is_icon_value = (value_buf[0] != '\0' && ((unsigned char)value_buf[0] >= 0xE0));
            settings_item_value_is_icon[i] = is_icon_value;
            if (is_icon_value) {
                lv_obj_set_style_text_font(settings_item_values[i], ui_font_lucide_20(), 0);
            } else {
                lv_obj_set_style_text_font(settings_item_values[i], &lv_font_inter_medium_16_ui, 0);
            }
        }

        if (settings_item_arrows[i]) {
            if (settings_item_has_arrow(i)) {
                lv_obj_clear_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(settings_item_arrows[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    if (settings_slider_caption) {
        if (slider_view) {
            lv_obj_add_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(settings_slider_caption, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (confirm_view) {
        if (settings_item_titles[0]) {
            lv_label_set_text(settings_item_titles[0], "");
        }
        if (settings_item_icons[0]) {
            lv_label_set_text(settings_item_icons[0], UI_ICON_LUCIDE_CHEVRON_LEFT);
        }
        if (settings_item_values[0]) {
            lv_obj_add_flag(settings_item_values[0], LV_OBJ_FLAG_HIDDEN);
        }

        if (settings_item_titles[1]) {
            lv_label_set_text(settings_item_titles[1],
                              overdrive_warning_view ? ui_tr("CONTINUE", "ПРОДОЛЖИТЬ")
                                                     : ui_tr("START", "СТАРТ"));
        }
        if (settings_item_values[1]) {
            lv_obj_add_flag(settings_item_values[1], LV_OBJ_FLAG_HIDDEN);
        }

        if (settings_overlay_icon_bg) {
            lv_obj_clear_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);
            if (overdrive_warning_view) {
                lv_obj_set_size(settings_overlay_icon_bg, 112, 112);
                lv_obj_align(settings_overlay_icon_bg, LV_ALIGN_TOP_MID, 0, 78);
                if (display_theme_dark) {
                    lv_obj_set_style_bg_color(settings_overlay_icon_bg, lv_color_hex(0x2A1408), 0);
                    lv_obj_set_style_bg_opa(settings_overlay_icon_bg, (lv_opa_t)145, 0);
                    lv_obj_set_style_border_width(settings_overlay_icon_bg, 2, 0);
                    lv_obj_set_style_border_color(settings_overlay_icon_bg, lv_color_hex(0xE10600), 0);
                    lv_obj_set_style_border_opa(settings_overlay_icon_bg, LV_OPA_COVER, 0);
                } else {
                    lv_obj_set_style_bg_opa(settings_overlay_icon_bg, LV_OPA_TRANSP, 0);
                    lv_obj_set_style_border_width(settings_overlay_icon_bg, 0, 0);
                    lv_obj_set_style_border_opa(settings_overlay_icon_bg, LV_OPA_TRANSP, 0);
                }
            } else if (purification_confirm_view) {
                lv_obj_set_size(settings_overlay_icon_bg, 112, 112);
                lv_obj_align(settings_overlay_icon_bg, LV_ALIGN_TOP_MID, 0, 78);
                lv_obj_set_style_bg_color(settings_overlay_icon_bg, lv_color_hex(0x063226), 0);
                lv_obj_set_style_bg_opa(settings_overlay_icon_bg, (lv_opa_t)145, 0);
                lv_obj_set_style_border_width(settings_overlay_icon_bg, 2, 0);
                lv_obj_set_style_border_color(settings_overlay_icon_bg, lv_color_hex(0x0DBF84), 0);
                lv_obj_set_style_border_opa(settings_overlay_icon_bg, LV_OPA_COVER, 0);
            }
        }
        if (settings_overlay_icon) {
            if (overdrive_warning_view) {
                lv_label_set_text(settings_overlay_icon, "!");
                lv_obj_set_style_text_font(settings_overlay_icon, &lv_font_montserrat_48, 0);
                lv_obj_set_style_text_color(settings_overlay_icon, lv_color_hex(0xE10600), 0);
                lv_obj_set_style_text_opa(settings_overlay_icon, LV_OPA_COVER, 0);
            } else {
                lv_label_set_text(settings_overlay_icon, UI_ICON_LUCIDE_WIND);
                lv_obj_set_style_text_font(settings_overlay_icon, ui_font_lucide_20(), 0);
                lv_obj_set_style_text_color(settings_overlay_icon, lv_color_hex(0x0DBF84), 0);
            }
        }
        if (settings_overlay_title) {
            lv_obj_clear_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_title,
                              overdrive_warning_view ? ui_tr("Overdrive Mode", "Режим Овердрайв")
                                                     : ui_tr("Air Purification", "Очистка воздуха"));
            lv_obj_set_style_text_font(settings_overlay_title,
                                       &lv_font_inter_semibold_28_ui,
                                       0);
            lv_obj_set_style_text_color(settings_overlay_title,
                                        overdrive_warning_view ? lv_color_hex(0xE10600)
                                                               : (display_theme_dark ? lv_color_white() : lv_color_black()),
                                        0);
            lv_obj_set_width(settings_overlay_title, overdrive_warning_view ? 380 : 320);
            lv_obj_align(settings_overlay_title, LV_ALIGN_TOP_MID, 0, overdrive_warning_view ? 202 : 198);
        }
        if (settings_overlay_line1) {
            lv_obj_clear_flag(settings_overlay_line1, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_line1,
                              overdrive_warning_view ? ui_tr("Allows fan speed up to 110% when filter is critical",
                                                             "Скорость до 110% при критическом фильтре")
                                                     : ui_tr("Please ensure an appropriate filter",
                                                             "Убедитесь, что установлен подходящий фильтр"));
            lv_obj_set_style_text_font(settings_overlay_line1,
                                       &lv_font_inter_medium_16_ui,
                                       0);
            lv_obj_set_style_text_color(settings_overlay_line1,
                                        overdrive_warning_view ? lv_color_hex(0xC3CAD6)
                                                               : (display_theme_dark ? lv_color_hex(0xC3CAD6) : lv_color_black()),
                                        0);
            lv_obj_set_width(settings_overlay_line1, overdrive_warning_view ? 380 : 320);
            lv_obj_align(settings_overlay_line1, LV_ALIGN_TOP_MID, 0, overdrive_warning_view ? 246 : 252);
        }
        if (settings_overlay_line2) {
            lv_obj_clear_flag(settings_overlay_line2, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_line2,
                              overdrive_warning_view ? ui_tr("Active only for 20 hours,",
                                                             "Работает только 20 часов,")
                                                     : ui_tr("is installed for air cleaning.",
                                                             "для очистки воздуха."));
            lv_obj_set_style_text_font(settings_overlay_line2,
                                       &lv_font_inter_medium_16_ui,
                                       0);
            lv_obj_set_style_text_color(settings_overlay_line2,
                                        overdrive_warning_view ? lv_color_hex(0xE10600)
                                                               : (display_theme_dark ? lv_color_hex(0x9CA3AF) : lv_color_black()),
                                        0);
            lv_obj_set_width(settings_overlay_line2, overdrive_warning_view ? 380 : 320);
            lv_obj_align(settings_overlay_line2, LV_ALIGN_TOP_MID, 0, overdrive_warning_view ? 280 : 280);
        }
        if (settings_overlay_line3) {
            lv_obj_clear_flag(settings_overlay_line3, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(settings_overlay_line3,
                              overdrive_warning_view ? ui_tr("then automatically returns to 100%",
                                                             "после чего вернется к 100%")
                                                     : ui_tr("Fan will run at 10% speed.",
                                                             "Вентилятор будет работать на 10%."));
            lv_obj_set_style_text_font(settings_overlay_line3,
                                       &lv_font_inter_medium_16_ui,
                                       0);
            lv_obj_set_style_text_color(settings_overlay_line3,
                                        overdrive_warning_view ? lv_color_hex(0xC3CAD6)
                                                               : (display_theme_dark ? lv_color_hex(0x7A7A83) : lv_color_black()),
                                        0);
            lv_obj_set_width(settings_overlay_line3, overdrive_warning_view ? 380 : 320);
            lv_obj_align(settings_overlay_line3, LV_ALIGN_TOP_MID, 0, overdrive_warning_view ? 314 : 308);
        }
    } else {
        if (settings_overlay_icon_bg) {
            lv_obj_add_flag(settings_overlay_icon_bg, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_title) {
            lv_obj_add_flag(settings_overlay_title, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_line1) {
            lv_obj_add_flag(settings_overlay_line1, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_line2) {
            lv_obj_add_flag(settings_overlay_line2, LV_OBJ_FLAG_HIDDEN);
        }
        if (settings_overlay_line3) {
            lv_obj_add_flag(settings_overlay_line3, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (settings_view_uses_wheel_list(settings_view) && !settings_root_scroll_animating) {
        settings_sync_root_scroll_to_index();
    }

    update_settings_wheel_layout();
    if (settings_view == SETTINGS_VIEW_ROOT && !settings_root_scroll_animating) {
        settings_update_blur_layers();
    }
}

static void settings_enter_view(settings_view_t view)
{
    settings_v2_nav_dir = 0;
    settings_v2_last_index = -1;
    if (view != SETTINGS_VIEW_FILTER) {
        settings_filter_reset_confirm_open = false;
    }
    settings_view_t prev_view = settings_view;
    if (view != prev_view) {
        settings_v2_transition_mode = (view == SETTINGS_VIEW_ROOT && prev_view != SETTINGS_VIEW_ROOT) ? -1 : 1;
    } else {
        settings_v2_transition_mode = 0;
    }
    settings_view = view;
    settings_edit_mode = (view == SETTINGS_VIEW_SENSITIVITY) || (view == SETTINGS_VIEW_BRIGHTNESS);

    if (view == SETTINGS_VIEW_ROOT) {
        int count = settings_get_item_count();
        settings_root_saved_index = 0;
        settings_index = clamp_int(settings_root_saved_index, 0, count - 1);
        settings_sync_root_scroll_to_index();
        for (int i = 0; i < SETTINGS_MENU_ITEMS_COUNT; ++i) {
            if (!settings_items[i]) {
                continue;
            }
            lv_obj_set_style_translate_x(settings_items[i], 0, 0);
            lv_obj_set_style_translate_y(settings_items[i], 0, 0);
            lv_obj_align(settings_items[i], LV_ALIGN_TOP_MID, 0, 0);
        }
        if (settings_focus_frame) {
            lv_obj_set_style_translate_x(settings_focus_frame, 0, 0);
            lv_obj_set_style_translate_y(settings_focus_frame, 0, 0);
            lv_obj_align(settings_focus_frame, LV_ALIGN_TOP_MID, 0, 0);
        }
    } else if (view == SETTINGS_VIEW_ACCENT) {
        int count = settings_get_item_count();
        settings_index = clamp_int(display_accent_index, 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else if (view == SETTINGS_VIEW_LANGUAGE) {
        int count = settings_get_item_count();
        settings_index = clamp_int(settings_option_index_from_lang(current_lang), 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else if (view == SETTINGS_VIEW_THEME) {
        int count = settings_get_item_count();
        settings_index = clamp_int(display_theme_dark ? 0 : 1, 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else if (view == SETTINGS_VIEW_ROTATION) {
        int count = settings_get_item_count();
        settings_index = clamp_int(((display_rotation_deg % 360) == 180) ? 1 : 0, 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else if (view == SETTINGS_VIEW_LOCK_MODE) {
        int count = settings_get_item_count();
        settings_index = clamp_int(allow_child_lock ? 0 : 1, 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else if (view == SETTINGS_VIEW_OVERDRIVE_MODE) {
        int count = settings_get_item_count();
        settings_index = clamp_int(allow_overdrive ? 0 : 1, 0, count - 1);
        settings_sync_root_scroll_to_index();
    } else if (view == SETTINGS_VIEW_FILTER_RESET) {
        settings_view = SETTINGS_VIEW_FILTER;
        settings_filter_reset_confirm_open = true;
        settings_index = 0;
        lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
        settings_root_scroll_animating = false;
    } else if (view == SETTINGS_VIEW_ABOUT) {
        settings_index = 0;
        lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
        settings_root_scroll_animating = false;
    } else {
        settings_index = 0;
        if (settings_view_uses_wheel_list(view)) {
            settings_sync_root_scroll_to_index();
        } else {
            lv_anim_delete(&settings_root_scroll_q, settings_root_scroll_anim_exec_cb);
            settings_root_scroll_animating = false;
        }
    }

    settings_refresh_view();
}

static void settings_go_back(void)
{
    if (settings_edit_mode) {
        if (settings_is_custom_accent_edit_active()) {
            if (settings_custom_edit_in_progress) {
                custom_accent_hue = settings_custom_edit_prev_hue;
                display_accent_index = settings_custom_edit_prev_accent_index;
                settings_custom_edit_in_progress = false;
                apply_runtime_visual_settings();
                update_main_ui();
            }
            settings_edit_mode = false;
            settings_enter_view(SETTINGS_VIEW_ACCENT);
            return;
        }
        settings_edit_mode = false;
        settings_refresh_view();
        return;
    }

    if (settings_view == SETTINGS_VIEW_ROOT) {
        ui_screen = UI_SCREEN_MAIN;
        show_screen(main_screen);
        set_lock_overlay(ui_locked);
        update_main_ui();
        return;
    }

    if (settings_view == SETTINGS_VIEW_OVERDRIVE_MODE) {
        settings_enter_view(SETTINGS_VIEW_ROOT);
        return;
    }

    if (settings_view == SETTINGS_VIEW_FILTER && settings_filter_reset_confirm_open) {
        settings_filter_reset_confirm_open = false;
        settings_index = 1;
        settings_refresh_view();
        return;
    }

    if (settings_view == SETTINGS_VIEW_FILTER_RESET) {
        settings_enter_view(SETTINGS_VIEW_FILTER);
        return;
    }

    settings_enter_view(SETTINGS_VIEW_ROOT);
}

static void settings_adjust_edited_value(int direction)
{
    if (!settings_edit_mode || direction == 0) {
        return;
    }

    bool fast_custom_hue_update = false;

    if (settings_view == SETTINGS_VIEW_SENSITIVITY && settings_index == 0) {
        speed_sensitivity_clicks = clamp_int(speed_sensitivity_clicks + direction * 2,
                                             SENSITIVITY_MIN_CLICKS,
                                             SENSITIVITY_MAX_CLICKS);
    } else if (settings_view == SETTINGS_VIEW_BRIGHTNESS && settings_index == 0) {
        int next_brightness = clamp_int(display_brightness_percent + direction * 10,
                                        DISPLAY_BRIGHTNESS_MIN,
                                        DISPLAY_BRIGHTNESS_MAX);
        display_set_brightness_percent(next_brightness);
    } else if (settings_view == SETTINGS_VIEW_FILTER && settings_index == 0) {
        filter_lifetime_hours = clamp_int(filter_lifetime_hours + direction * 5,
                                          FILTER_LIFETIME_MIN_HOURS,
                                          FILTER_LIFETIME_MAX_HOURS);
        if (filter_used_hours > filter_lifetime_hours) {
            filter_used_hours = filter_lifetime_hours;
        }
        {
            uint32_t max_filter_seconds = (uint32_t)filter_lifetime_hours * 3600U;
            if (filter_used_seconds_total > max_filter_seconds) {
                filter_used_seconds_total = max_filter_seconds;
                filter_runtime_partial_us = 0;
            }
        }
        refresh_filter_status_from_usage();
        update_main_ui();
    } else if (settings_is_custom_accent_edit_active()) {
        accent_set_custom_hue(custom_accent_hue + direction * ACCENT_CUSTOM_HUE_STEP_DEG);
        ui_accent_color_hex = accent_hex_from_hue(custom_accent_hue);
        fast_custom_hue_update = true;
    }

    if (fast_custom_hue_update) {
        update_settings_wheel_layout();
        return;
    }

    settings_refresh_view();
}

static void settings_filter_reset_anim_exec_cb(void *var, int32_t value)
{
    (void)var;
    settings_filter_reset_anim_value = value;
    update_settings_wheel_layout();
}

static void settings_filter_reset_anim_ready_cb(lv_anim_t *a)
{
    (void)a;
    settings_filter_reset_animating = false;
    settings_filter_reset_anim_value = -1;
    filter_usage_reset_and_save();
    update_main_ui();
    settings_filter_reset_confirm_open = false;
    settings_index = 1;
    settings_refresh_view();
}

static void settings_start_filter_reset_anim(void)
{
    if (settings_filter_reset_animating) {
        return;
    }

    int start_used = clamp_int(filter_used_hours, 0, filter_lifetime_hours);
    if (start_used <= 0) {
        filter_usage_reset_and_save();
        update_main_ui();
        settings_filter_reset_confirm_open = false;
        settings_index = 1;
        settings_refresh_view();
        return;
    }

    settings_filter_reset_animating = true;
    settings_filter_reset_anim_value = start_used;
    lv_anim_delete(&settings_filter_reset_anim_value, settings_filter_reset_anim_exec_cb);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, &settings_filter_reset_anim_value);
    lv_anim_set_exec_cb(&a, settings_filter_reset_anim_exec_cb);
    lv_anim_set_values(&a, start_used, 0);
    lv_anim_set_time(&a, 1);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, settings_filter_reset_anim_ready_cb);
    lv_anim_start(&a);
}

static void settings_activate_item(void)
{
    if (settings_edit_mode) {
        if (settings_is_custom_accent_edit_active()) {
            settings_edit_mode = false;
            settings_custom_edit_in_progress = false;
            display_accent_index = DISPLAY_ACCENT_CUSTOM_INDEX;
            apply_runtime_visual_settings();
            update_main_ui();
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_ROOT);
            return;
        }
        settings_edit_mode = false;
        save_control_settings();
        if (settings_view == SETTINGS_VIEW_SENSITIVITY || settings_view == SETTINGS_VIEW_BRIGHTNESS) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_refresh_view();
        }
        return;
    }

    switch (settings_view) {
    case SETTINGS_VIEW_ROOT:
        switch (settings_index) {
        case 0: settings_enter_view(SETTINGS_VIEW_FILTER); return;
        case 1: settings_enter_view(SETTINGS_VIEW_SENSITIVITY); return;
        case 2: settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING); return;
        case 3: settings_enter_view(SETTINGS_VIEW_ACCENT); return;
        case 4: settings_enter_view(SETTINGS_VIEW_LANGUAGE); return;
        case 5: settings_enter_view(SETTINGS_VIEW_BRIGHTNESS); return;
        case 6: settings_enter_view(SETTINGS_VIEW_LOCK_MODE); return;
        case 7: settings_enter_view(SETTINGS_VIEW_ROTATION); return;
        case 8: settings_enter_view(SETTINGS_VIEW_THEME); return;
        case 9: settings_enter_view(SETTINGS_VIEW_PURIFICATION_CONFIRM); return;
        default: settings_enter_view(SETTINGS_VIEW_ABOUT); return;
        }

    case SETTINGS_VIEW_SENSITIVITY:
        if (settings_index == 0) {
            settings_edit_mode = true;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_ACCENT:
        if (settings_index < DISPLAY_ACCENT_COUNT) {
            display_accent_index = settings_index;
            apply_runtime_visual_settings();
            update_main_ui();
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_ROOT);
#if UI_ENABLE_CUSTOM_ACCENT
        } else if (settings_index == DISPLAY_ACCENT_CUSTOM_INDEX) {
            settings_custom_edit_prev_accent_index = display_accent_index;
            settings_custom_edit_prev_hue = custom_accent_hue;
            settings_custom_edit_in_progress = true;
            display_accent_index = DISPLAY_ACCENT_CUSTOM_INDEX;
            settings_edit_mode = true;
            apply_runtime_visual_settings();
            update_main_ui();
            settings_refresh_view();
#endif
        } else {
            settings_enter_view(SETTINGS_VIEW_ACCENT);
        }
        return;

    case SETTINGS_VIEW_LANGUAGE:
        if (settings_index >= 0 && settings_index < SETTINGS_LANG_OPTIONS_COUNT) {
            ui_lang_t selected_lang = settings_lang_from_option_index(settings_index);
            if (current_lang == selected_lang) {
                settings_enter_view(SETTINGS_VIEW_ROOT);
                return;
            }
            current_lang = selected_lang;
            save_language();
            apply_language();
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_BRIGHTNESS:
        if (settings_index == 0) {
            settings_edit_mode = true;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_THEME:
        if (settings_index == 0) {
            display_theme_dark = true;
            save_control_settings();
            apply_theme_now();
            restart_current_screen_after_theme_change();
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else if (settings_index == 1) {
            display_theme_dark = false;
            save_control_settings();
            apply_theme_now();
            restart_current_screen_after_theme_change();
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_ROTATION:
        if (settings_index == 0) {
            display_rotation_deg = 0;
            apply_display_rotation_now();
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else if (settings_index == 1) {
            display_rotation_deg = 180;
            apply_display_rotation_now();
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_LOCK_MODE:
        if (settings_index == 0) {
            allow_child_lock = true;
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else if (settings_index == 1) {
            allow_child_lock = false;
            ui_locked = false;
            set_lock_overlay(false);
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_OVERDRIVE_WARNING:
        if (settings_index == 0) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            settings_enter_view(SETTINGS_VIEW_OVERDRIVE_MODE);
        }
        return;

    case SETTINGS_VIEW_OVERDRIVE_MODE:
        if (settings_index == 0) {
            allow_overdrive = true;
            clamp_speed_to_limits();
            apply_fan_pwm(fan_speed_percent);
            update_main_ui();
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING);
        } else if (settings_index == 1) {
            allow_overdrive = false;
            clamp_speed_to_limits();
            apply_fan_pwm(fan_speed_percent);
            update_main_ui();
            save_control_settings();
            settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING);
        } else {
            settings_enter_view(SETTINGS_VIEW_OVERDRIVE_WARNING);
        }
        return;

    case SETTINGS_VIEW_FILTER:
        if (settings_filter_reset_confirm_open) {
            if (settings_index == 0) {
                settings_filter_reset_confirm_open = false;
                settings_index = 1;
                settings_refresh_view();
            } else {
                settings_start_filter_reset_anim();
            }
        } else if (settings_index == 0) {
            settings_edit_mode = true;
            settings_refresh_view();
        } else if (settings_index == 1) {
            settings_filter_reset_confirm_open = true;
            settings_index = 0;
            settings_refresh_view();
        } else {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        }
        return;

    case SETTINGS_VIEW_FILTER_RESET:
        if (settings_index == 0) {
            settings_enter_view(SETTINGS_VIEW_FILTER);
        } else {
            settings_start_filter_reset_anim();
        }
        return;

    case SETTINGS_VIEW_PURIFICATION_CONFIRM:
        if (settings_index == 0) {
            settings_enter_view(SETTINGS_VIEW_ROOT);
        } else {
            start_purification_mode();
        }
        return;

    case SETTINGS_VIEW_ABOUT:
        settings_enter_view(SETTINGS_VIEW_ROOT);
        return;

    default:
        settings_go_back();
        return;
    }
}

