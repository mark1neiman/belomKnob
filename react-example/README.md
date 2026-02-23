# BELOM A2 UI Concept

This project implements a UI concept for the BELOM A2 industrial dust collector. It is designed for a 38mm round display and features a physical encoder-driven interface.

## Tech Stack

- **Framework**: React 18
- **Styling**: Tailwind CSS v4
- **Icons**: Lucide React
- **Build Tool**: Vite

## Fonts

The project uses the following fonts:

1.  **Inter** (Google Fonts)
    *   **Usage**: Main UI text, labels, headers.
    *   **Weights**: 300, 400, 500, 600, 700.
    *   **Sizes (px)**: 10, 12, 14, 16, 18, 20, 24, 30.
    *   **Source**: Imported in `/src/styles/fonts.css` via Google Fonts.
    *   **Fallback**: System UI fonts (`system-ui`, `-apple-system`, etc.).

2.  **Monospace / Numeric** (System Default)
    *   **Usage**: Numeric values requiring fixed width alignment (Time, Date, Sensor Readings).
    *   **Sizes (px)**: 18, 24, 36, 48, 60.
    *   **Implementation**: `font-mono` utility class in Tailwind (defaults to `ui-monospace`, `SFMono-Regular`, `Menlo`, `Monaco`, `Consolas`, etc.).

## Icons

All icons are sourced from the **Lucide React** library (`lucide-react@0.487.0`).

**Key Icons Used:**
- `Gauge`: Sensitivity settings
- `Palette`: Accent color selection
- `Globe`: Language selection
- `Sun` / `Moon`: Brightness and Theme toggles
- `Clock`: Time settings
- `RotateCw`: Screen rotation
- `Lock`: Lock mode
- `Zap`: Overdrive mode
- `Filter`: Filter status
- `Wind`: Purification mode confirmation
- `Info`: About screen
- `ChevronLeft` / `ChevronRight`: Navigation indicators
- `Check`: Selection confirmation
- `RefreshCw`: Reset actions

## Interaction Model

The UI is designed to be controlled **exclusively** via a rotary encoder with a push button.

- **Rotate Left/Right**: Navigate lists or adjust values.
- **Press**: Select item or confirm value.
- **Long Press** (Concept): Go back or Reset (implemented as "Back" menu items in this prototype).

### Simulation in Browser
Since this is a web prototype:
- **Keyboard Left/Right Arrows**: Simulate Encoder Rotation.
- **Mouse Click**: Simulates Encoder Press (on specific elements) or Touch interaction (for debugging).

## LVGL Implementation Guide

For the embedded implementation on the actual hardware (e.g., STM32, ESP32), we recommend using **LVGL v8 or v9**.

### 1. Display Driver
- **Resolution**: 240x240, 320x320, or similar (Round).
- **Radius**: Apply a mask or software masking to respect the round shape.
  ```c
  // LVGL Round Display Masking
  lv_obj_t * screen = lv_scr_act();
  lv_obj_set_style_radius(screen, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_clip_corner(screen, true, 0);
  ```

### 2. Input Device (Indev)
- Map the physical encoder to `LV_INDEV_TYPE_ENCODER`.
- Use an `lv_group_t` to manage focus.
  ```c
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_ENCODER;
  indev_drv.read_cb = encoder_read;
  lv_indev_t * encoder_indev = lv_indev_drv_register(&indev_drv);

  lv_group_t * g = lv_group_create();
  lv_indev_set_group(encoder_indev, g);
  ```

### 3. UI Components Mapping

| React Component | LVGL Equivalent Strategy |
| :--- | :--- |
| **WheelList** (3D Menu) | **`lv_roller`** (simple) or **`lv_obj` list with `lv_event_cb`** (advanced). For the "fish-eye" 3D effect, use the `LV_EVENT_SCROLL` event to scale and set opacity of children based on their Y-position relative to the center. |
| **Range Slider** | **`lv_slider`** or **`lv_arc`**. For a round screen, `lv_arc` often looks better for values like Brightness or Sensitivity. |
| **Toggle** | **`lv_switch`** or a custom button with checked state. |
| **Icons** | Convert Lucide SVGs to **C array bitmaps** using the [LVGL Image Converter](https://lvgl.io/tools/imageconverter) or use a font symbol set (e.g., FontAwesome, or a custom generated font from Lucide icons). |
| **Themes** | Use `lv_theme_t`. Define a Dark and Light theme struct and switch between them using `lv_disp_set_theme`. |

### 4. Code Structure Recommendation (C/C++)

```c
// ui_events.h
void ui_event_navigate(lv_event_t * e);
void ui_event_value_change(lv_event_t * e);

// screens/settings_screen.c
void ui_create_settings_screen(void) {
    lv_obj_t * list = lv_obj_create(lv_scr_act());
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    
    // Add items
    create_menu_item(list, ICON_GAUGE, "Sensitivity", "5 clicks");
    create_menu_item(list, ICON_PALETTE, "Accent Color", "Red");
    
    // Attach group for encoder
    lv_group_add_obj(g, list);
}

// Custom 3D Scroll Effect
static void scroll_event_cb(lv_event_t * e) {
    lv_obj_t * cont = lv_event_get_target(e);
    // Iterate children, calculate distance from center, apply zoom/opacity
}
```
