esp_err_t app_touch_init(void)
{
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
    /* Initilize I2C */
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = EXAMPLE_PIN_NUM_TOUCH_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = EXAMPLE_PIN_NUM_TOUCH_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = EXAMPLE_TOUCH_I2C_CLK_HZ};
    ESP_RETURN_ON_ERROR(i2c_param_config(EXAMPLE_TOUCH_I2C_NUM, &i2c_conf), TAG, "I2C configuration failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(EXAMPLE_TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0), TAG, "I2C initialization failed");

    /* Initialize touch HW */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = EXAMPLE_PIN_NUM_TOUCH_RST, // Shared with LCD reset
        .int_gpio_num = EXAMPLE_PIN_NUM_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            // Screen orientation is usually adjusted here.
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST820_CONFIG();
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)EXAMPLE_TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle), TAG, "");
    return esp_lcd_touch_new_i2c_cst820(tp_io_handle, &tp_cfg, &touch_handle);
#else
    touch_handle = NULL;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

void rounder_event_cb(lv_event_t *e)
{
    lv_area_t *area = lv_event_get_invalidated_area(e);
    if (!area) {
        return;
    }

    /* Keep invalidation area inside display bounds first.
     * Using unsigned intermediates here can overflow when LVGL sends
     * temporary negative coordinates (e.g. shadows/transforms). */
    if (area->x1 < 0) {
        area->x1 = 0;
    }
    if (area->y1 < 0) {
        area->y1 = 0;
    }
    if (area->x1 >= EXAMPLE_LCD_H_RES) {
        area->x1 = EXAMPLE_LCD_H_RES - 1;
    }
    if (area->y1 >= EXAMPLE_LCD_V_RES) {
        area->y1 = EXAMPLE_LCD_V_RES - 1;
    }
    if (area->x2 >= EXAMPLE_LCD_H_RES) {
        area->x2 = EXAMPLE_LCD_H_RES - 1;
    }
    if (area->y2 >= EXAMPLE_LCD_V_RES) {
        area->y2 = EXAMPLE_LCD_V_RES - 1;
    }
    if (area->x2 < 0) {
        area->x2 = 0;
    }
    if (area->y2 < 0) {
        area->y2 = 0;
    }

    // Round start down to even pixel, end up to odd pixel.
    area->x1 &= ~1;
    area->y1 &= ~1;
    area->x2 |= 1;
    area->y2 |= 1;

    if (area->x2 >= EXAMPLE_LCD_H_RES) {
        area->x2 = EXAMPLE_LCD_H_RES - 1;
    }
    if (area->y2 >= EXAMPLE_LCD_V_RES) {
        area->y2 = EXAMPLE_LCD_V_RES - 1;
    }
    if (area->x2 < area->x1) {
        area->x2 = area->x1;
    }
    if (area->y2 < area->y1) {
        area->y2 = area->y1;
    }
}

esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 5,       /* LVGL task priority */
        .task_stack = 12288,      /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT, // EXAMPLE_LCD_DRAW_BUFF_HEIGHT
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,  // Screen orientation is usually adjusted here.
            .mirror_x = false, // true,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
#if UI_ENABLE_INVALIDATE_ROUNDER
    lv_display_add_event_cb(lvgl_disp, rounder_event_cb, LV_EVENT_INVALIDATE_AREA, NULL);
#endif
    lv_display_add_event_cb(lvgl_disp, fps_display_refr_event_cb, LV_EVENT_REFR_START, NULL);

    /* Add touch input (for selected screen) */
    if (touch_handle) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lvgl_disp,
            .handle = touch_handle,
        };
        lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);
    } else {
        ESP_LOGW(TAG, "Touch is disabled because initialization failed");
    }

    return ESP_OK;
}

