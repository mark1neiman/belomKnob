static void uart_control_print_help(void)
{
    printf("\r\nUART control commands:\r\n");
    printf("  help          - show this help\r\n");
    printf("  + / up        - increase fan speed / move selection up\r\n");
    printf("  - / down      - decrease fan speed / move selection down\r\n");
    printf("  speed <0-110> - set fan speed in percent (110 only in overdrive)\r\n");
    printf("  filter <good|warning|critical> - set filter status color\r\n");
    printf("  click         - single click action\r\n");
    printf("  double        - double click action\r\n");
    printf("  hold          - long press action\r\n");
    printf("  lock          - lock main screen\r\n");
    printf("  unlock        - unlock main screen\r\n");
    printf("  purify <on|off> - start/stop purification mode\r\n");
    printf("  sleep / wake  - enter/exit eco sleep mode\r\n");
}

static void uart_control_set_speed(int speed)
{
    set_speed_percent(speed);
    apply_fan_pwm(fan_speed_percent);
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "LVGL lock timeout in uart set_speed");
        return;
    }
    update_main_ui();
    lvgl_port_unlock();
    ESP_LOGI(TAG, "Fan speed set to %d%%", fan_speed_percent);
}

static void uart_control_set_filter(filter_status_t new_status)
{
    filter_status = new_status;
    clamp_speed_to_limits();
    apply_fan_pwm(fan_speed_percent);
    if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
        ESP_LOGW(TAG, "LVGL lock timeout in uart set_filter");
        return;
    }
    update_main_ui();
    lvgl_port_unlock();
    ESP_LOGI(TAG, "Filter status set to: %s", get_filter_status_text());
}

static void uart_control_exec(char *cmd)
{
    while (*cmd == ' ' || *cmd == '\t') {
        cmd++;
    }
    if (*cmd == '\0') {
        return;
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
        uart_control_print_help();
        return;
    }
    if (strcmp(cmd, "+") == 0 || strcmp(cmd, "up") == 0) {
        handle_knob_move(1);
        return;
    }
    if (strcmp(cmd, "-") == 0 || strcmp(cmd, "down") == 0) {
        handle_knob_move(-1);
        return;
    }
    if (strncmp(cmd, "speed ", 6) == 0) {
        uart_control_set_speed(atoi(cmd + 6));
        return;
    }
    if (strncmp(cmd, "filter ", 7) == 0) {
        const char *value = cmd + 7;
        if (strcmp(value, "good") == 0) {
            uart_control_set_filter(FILTER_STATUS_GOOD);
        } else if (strcmp(value, "warning") == 0) {
            uart_control_set_filter(FILTER_STATUS_WARNING);
        } else if (strcmp(value, "critical") == 0) {
            uart_control_set_filter(FILTER_STATUS_CRITICAL);
        } else {
            ESP_LOGW(TAG, "Unknown filter status: %s", value);
        }
        return;
    }
    if (strcmp(cmd, "click") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart click");
            return;
        }
        handle_single_click();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "double") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart double");
            return;
        }
        handle_double_click();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "hold") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart hold");
            return;
        }
        handle_long_press();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "lock") == 0) {
        ui_locked = true;
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart lock");
            return;
        }
        set_lock_overlay(true);
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "unlock") == 0) {
        ui_locked = false;
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart unlock");
            return;
        }
        set_lock_overlay(false);
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "sleep") == 0) {
        enter_sleep_mode();
        return;
    }
    if (strcmp(cmd, "wake") == 0) {
        wake_from_sleep();
        return;
    }
    if (strcmp(cmd, "purify on") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart purify on");
            return;
        }
        start_purification_mode();
        lvgl_port_unlock();
        return;
    }
    if (strcmp(cmd, "purify off") == 0) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in uart purify off");
            return;
        }
        stop_purification_mode(true);
        lvgl_port_unlock();
        return;
    }

    ESP_LOGW(TAG, "Unknown UART command: %s", cmd);
}

static void uart_control_task(void *arg)
{
    (void)arg;
    char line[64];

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    uart_control_print_help();

    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }
        line[strcspn(line, "\r\n")] = '\0';
        uart_control_exec(line);
    }
}

