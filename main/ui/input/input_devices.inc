void LVGL_knob_event(void *event)
{
}

void LVGL_button_event(void *event)
{
}

/* Use hardware PCNT quadrature decoding for robust high-speed rotary handling. */
#define KNOB_PCNT_HIGH_LIMIT 32767
#define KNOB_PCNT_LOW_LIMIT -32768
#define KNOB_PCNT_GLITCH_NS 10000
#define KNOB_PCNT_COUNTS_PER_STEP 2
#define KNOB_PCNT_POLL_MS 2
#define KNOB_PCNT_MAX_BURST_STEPS 24
#define KNOB_PCNT_CLEAR_THRESHOLD 24000
#define KNOB_PCNT_REVERSE 0

static pcnt_unit_handle_t knob_pcnt_unit = NULL;
static pcnt_channel_handle_t knob_pcnt_chan_a = NULL;
static pcnt_channel_handle_t knob_pcnt_chan_b = NULL;
static TaskHandle_t knob_pcnt_task_handle = NULL;

static void knob_pcnt_task(void *arg)
{
    pcnt_unit_handle_t unit = (pcnt_unit_handle_t)arg;
    int last_count = 0;
    int residual_counts = 0;
    (void)pcnt_unit_get_count(unit, &last_count);

    while (1) {
        int count = last_count;
        if (pcnt_unit_get_count(unit, &count) == ESP_OK) {
            int delta = count - last_count;
            last_count = count;
            if (delta != 0) {
#if KNOB_PCNT_REVERSE
                delta = -delta;
#endif
                delta += residual_counts;
                int logical_steps = delta / KNOB_PCNT_COUNTS_PER_STEP;
                residual_counts = delta - (logical_steps * KNOB_PCNT_COUNTS_PER_STEP);

                if (logical_steps != 0) {
                    int dir = (logical_steps > 0) ? 1 : -1;
                    int steps = (logical_steps > 0) ? logical_steps : -logical_steps;
                    if (steps > KNOB_PCNT_MAX_BURST_STEPS) {
                        steps = KNOB_PCNT_MAX_BURST_STEPS;
                    }
                    int aggregated = dir * steps;
                    handle_knob_move(aggregated);
                    LVGL_knob_event((void *)(long)aggregated);
                }

                if (count >= KNOB_PCNT_CLEAR_THRESHOLD || count <= -KNOB_PCNT_CLEAR_THRESHOLD) {
                    if (pcnt_unit_clear_count(unit) == ESP_OK) {
                        last_count = 0;
                        residual_counts = 0;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(KNOB_PCNT_POLL_MS));
    }
}

void knob_init(uint32_t encoder_a, uint32_t encoder_b)
{
    gpio_config_t knob_gpio_cfg = {
        .pin_bit_mask = (1ULL << encoder_a) | (1ULL << encoder_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&knob_gpio_cfg));

    pcnt_unit_config_t unit_config = {
        .high_limit = KNOB_PCNT_HIGH_LIMIT,
        .low_limit = KNOB_PCNT_LOW_LIMIT,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &knob_pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = KNOB_PCNT_GLITCH_NS,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(knob_pcnt_unit, &filter_config));

    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = (int)encoder_a,
        .level_gpio_num = (int)encoder_b,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(knob_pcnt_unit, &chan_a_config, &knob_pcnt_chan_a));

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = (int)encoder_b,
        .level_gpio_num = (int)encoder_a,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(knob_pcnt_unit, &chan_b_config, &knob_pcnt_chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        knob_pcnt_chan_a,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        knob_pcnt_chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        knob_pcnt_chan_b,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        knob_pcnt_chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(knob_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(knob_pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(knob_pcnt_unit));

    BaseType_t task_ok = xTaskCreate(knob_pcnt_task, "knob_pcnt", 3072, knob_pcnt_unit, 10, &knob_pcnt_task_handle);
    ESP_ERROR_CHECK((task_ok == pdPASS) ? ESP_OK : ESP_FAIL);

    ESP_LOGI(TAG, "PCNT knob initialized on GPIO A=%lu, B=%lu", (unsigned long)encoder_a, (unsigned long)encoder_b);
}

/* Button helpers */
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C6
#define BOOT_BUTTON_NUM 9
#else
#define BOOT_BUTTON_NUM 0
#endif
#define BUTTON_ACTIVE_LEVEL 0
const char *button_event_table[] = {
    "BUTTON_PRESS_DOWN",
    "BUTTON_PRESS_UP",
    "BUTTON_PRESS_REPEAT",
    "BUTTON_PRESS_REPEAT_DONE",
    "BUTTON_SINGLE_CLICK",
    "BUTTON_DOUBLE_CLICK",
    "BUTTON_MULTIPLE_CLICK",
    "BUTTON_LONG_PRESS_START",
    "BUTTON_LONG_PRESS_HOLD",
    "BUTTON_LONG_PRESS_UP",
    "BUTTON_PRESS_END",
};

static void button_event_cb(void *arg, void *data)
{
    button_event_t event = (button_event_t)data;
    ESP_LOGI(TAG, "Button event %s", button_event_table[event]);
    if (event == BUTTON_SINGLE_CLICK) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in button single click");
            return;
        }
        handle_single_click();
        lvgl_port_unlock();
    } else if (event == BUTTON_DOUBLE_CLICK) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in button double click");
            return;
        }
        handle_double_click();
        lvgl_port_unlock();
    } else if (event == BUTTON_LONG_PRESS_START) {
        if (!lvgl_port_lock(UI_LVGL_LOCK_TIMEOUT_MS)) {
            ESP_LOGW(TAG, "LVGL lock timeout in button long press");
            return;
        }
        handle_long_press();
        lvgl_port_unlock();
    }
    LVGL_button_event(data);
}
void button_init(uint32_t button_num)
{
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = button_num,
            .active_level = BUTTON_ACTIVE_LEVEL,
#if CONFIG_GPIO_BUTTON_SUPPORT_POWER_SAVE
            .enable_power_save = true,
#endif
        },
    };
    button_handle_t btn = iot_button_create(&btn_cfg);
    assert(btn);
    esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, button_event_cb, (void *)BUTTON_SINGLE_CLICK);
    err |= iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, button_event_cb, (void *)BUTTON_DOUBLE_CLICK);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_START, button_event_cb, (void *)BUTTON_LONG_PRESS_START);

#if CONFIG_ENTER_LIGHT_SLEEP_MODE_MANUALLY
    /*!< For enter Power Save */
    button_power_save_config_t config = {
        .enter_power_save_cb = button_enter_power_save,
    };
    err |= iot_button_register_power_save_cb(&config);
#endif

    ESP_ERROR_CHECK(err);
}

