/**
 * @file ui_logic_pseudo.c
 * @brief Pseudo-code for BELOM A2 UI Logic (LVGL Reference)
 * 
 * This file outlines the core state machine and event handling for the UI.
 * It is not intended to compile, but to guide the C/C++ implementation.
 */

#include "styles_colors.h"

// --- State Definitions ---

typedef enum {
    UI_STATE_BOOT,
    UI_STATE_MAIN_SCREEN,
    UI_STATE_SETTINGS_ROOT,
    UI_STATE_SETTINGS_SUBMENU
} ui_state_t;

typedef enum {
    FILTER_STATUS_GOOD,
    FILTER_STATUS_WARNING,
    FILTER_STATUS_CRITICAL
} filter_status_t;

typedef struct {
    uint8_t sensitivity;      // 15-100 (steps per 100%)
    uint8_t brightness;       // 10-100%
    uint16_t accent_color;    // LV_COLOR_HEX format
    uint8_t rotation;         // 0 or 180 degrees
    bool lock_mode_enabled;   // true = allows locking
    bool allow_overdrive;     // true = allow >100% speed
    uint16_t filter_lifetime; // e.g. 100 hours
    uint16_t filter_used;     // e.g. 12 hours
} settings_t;

typedef struct {
    ui_state_t current_state;
    ui_state_t prev_state;
    uint8_t current_speed;    // 0-110
    bool is_locked;
    filter_status_t filter_status;
    settings_t settings;
    // Navigation state
    uint8_t selected_menu_index;
    uint8_t selected_submenu_index;
} app_context_t;

app_context_t app;

// --- Helper Functions ---

void update_filter_status() {
    float usage_ratio = (float)app.settings.filter_used / (float)app.settings.filter_lifetime;
    if (usage_ratio >= 0.9f) {
        app.filter_status = FILTER_STATUS_CRITICAL;
    } else if (usage_ratio >= 0.75f) {
        app.filter_status = FILTER_STATUS_WARNING;
    } else {
        app.filter_status = FILTER_STATUS_GOOD;
    }
}

uint8_t get_max_speed() {
    // Overdrive Logic: Only if enabled AND critical filter
    if (app.settings.allow_overdrive && app.filter_status == FILTER_STATUS_CRITICAL) {
        return 110;
    }
    return 100;
}

// --- Event Handlers (Encoder) ---

/**
 * @brief Called when the physical encoder rotates
 * @param steps Positive for CW, Negative for CCW
 */
void on_encoder_rotate(int16_t steps) {
    if (app.current_state == UI_STATE_MAIN_SCREEN) {
        if (app.is_locked) return; // Ignore input if locked

        // Calculate step size based on sensitivity
        // React: step = range / sensitivity
        // range is 100 (or 110)
        // sensitivity is e.g. 15 clicks for full range
        // step = 100 / 15 = ~6.6 per click
        
        float step_val = (float)get_max_speed() / (float)app.settings.sensitivity;
        int16_t speed_change = (int16_t)(steps * step_val);
        
        int16_t new_speed = app.current_speed + speed_change;
        if (new_speed < 0) new_speed = 0;
        if (new_speed > get_max_speed()) new_speed = get_max_speed();
        
        app.current_speed = (uint8_t)new_speed;
        
        // Update Ring Display (LVGL function call)
        // update_gauge_ui(app.current_speed);
        
        // Show/Hide Overdrive Text
        if (app.current_speed > 100) {
            // show_overdrive_warning(true); // Red text
        } else {
            // show_overdrive_warning(false);
        }

    } else if (app.current_state == UI_STATE_SETTINGS_ROOT) {
        // Scroll through menu items
        // clamp(app.selected_menu_index + steps, 0, MAX_MENU_ITEMS - 1);
        // update_menu_ui();
        
    } else if (app.current_state == UI_STATE_SETTINGS_SUBMENU) {
        // Handle submenu navigation (e.g. Filter Reset: Back vs Reset)
        // clamp(app.selected_submenu_index + steps, 0, MAX_SUBMENU_ITEMS - 1);
        // update_submenu_ui();
    }
}

/**
 * @brief Called on short press of the encoder button
 */
void on_encoder_click_short() {
    if (app.current_state == UI_STATE_MAIN_SCREEN) {
        // Toggle Lock
        if (app.settings.lock_mode_enabled) {
            app.is_locked = !app.is_locked;
            // update_lock_icon(app.is_locked);
        }
        
    } else if (app.current_state == UI_STATE_SETTINGS_ROOT) {
        // Enter Submenu
        // enter_submenu(app.selected_menu_index);
        
    } else if (app.current_state == UI_STATE_SETTINGS_SUBMENU) {
        // Confirm Action
        // e.g. if (current_submenu == FILTER_RESET)
        // if (selected_index == RESET_BTN) {
        //     app.settings.filter_used = 0;
        //     update_filter_status();
        //     go_back();
        // } else {
        //     go_back();
        // }
    }
}

/**
 * @brief Called on long press (> 1s) of the encoder button
 */
void on_encoder_click_long() {
    if (app.current_state == UI_STATE_MAIN_SCREEN) {
        // Enter Settings
        app.current_state = UI_STATE_SETTINGS_ROOT;
        // show_settings_ui();
        
    } else if (app.current_state == UI_STATE_SETTINGS_ROOT) {
        // Exit Settings
        app.current_state = UI_STATE_MAIN_SCREEN;
        // show_main_ui();
        
    } else if (app.current_state == UI_STATE_SETTINGS_SUBMENU) {
        // Go Back to Root Menu
        app.current_state = UI_STATE_SETTINGS_ROOT;
        // show_settings_ui();
    }
}

// --- Display Logic Notes ---

/*
 * Colors:
 * - Critical: ALWAYS #EF4444 (Red)
 * - Warning: ALWAYS #EAB308 (Amber)
 * - Good: ALWAYS #10B981 (Green)
 * - Overdrive Text: ALWAYS #EF4444 (Red)
 * 
 * - Accent: Defined by app.settings.accent_color (used for main UI elements)
 */
