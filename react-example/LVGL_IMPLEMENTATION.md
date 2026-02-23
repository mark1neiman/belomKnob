# LVGL Implementation Concept

This document details how the React concepts in the BELOM A2 UI translate to LVGL (Light and Versatile Graphics Library) for embedded implementation.

## 1. Core Architecture

### Display & Input
- **Driver**: GC9A01 or similar (240x240 / 320x320 Round).
- **Input**: Rotary Encoder (Button + Rotate).
- **LVGL**: Version 8.x or 9.x recommended.

### File Structure
Ideally, separate UI logic from business logic.
```
/ui
  /assets (Converted images/fonts)
  /components
    ui_wheel_list.c/.h    (Custom widget for the scrolling list)
    ui_header.c/.h        (Top overlay)
  /screens
    ui_screen_settings.c/.h
    ui_screen_purification.c/.h
  ui.c/.h (Main entry)
```

## 2. The "Wheel List" (3D Menu)

The core navigation component in React uses CSS 3D transforms (`rotateX`, `scale`, `opacity`). In LVGL, we simulate this with 2D transformations in a scroll event callback.

### Strategy
1. Create a container (`lv_obj_create`) with `LV_FLEX_FLOW_COLUMN`.
2. Set `lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER)`.
3. Add an event callback `LV_EVENT_SCROLL` to the container.

### Code Concept (C)
```c
static void scroll_event_cb(lv_event_t * e) {
    lv_obj_t * cont = lv_event_get_target(e);
    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);
        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /* Get the percentage of distance from center (0..1) */
        /* Assuming visible area height is ~240 */
        int32_t dist_pct = (diff_y * 100) / 120; 
        if(dist_pct > 100) dist_pct = 100;

        /* Calculate scale and opacity */
        /* Scale: 100% at center, 60% at edges */
        int32_t scale = 256 - (dist_pct * 100) / 100; 
        /* Opacity: 255 at center, 50 at edges */
        lv_opa_t opa = 255 - (dist_pct * 2);

        lv_obj_set_style_transform_zoom(child, scale, 0);
        lv_obj_set_style_opa(child, opa, 0);
    }
}
```

## 3. The Date/Time Picker

In React, we use the same `WheelList` logic for selecting numbers. In LVGL, the built-in `lv_roller` widget is perfect for this, but styling needs adjustment to match the "clean" look.

### Strategy
- Use `lv_roller` for Day, Month, Year.
- Set `lv_roller_set_options` dynamically (e.g., changing days based on month).
- Use `lv_roller_set_visible_row_count(roller, 3)`.
- **Infinite Loop**: `lv_roller_set_mode(roller, LV_ROLLER_MODE_INFINITE)`.

### Code Concept (C)
```c
lv_obj_t * roller_day = lv_roller_create(parent);
lv_roller_set_options(roller_day, "1\n2\n3\n...\n31", LV_ROLLER_MODE_INFINITE);
lv_roller_set_visible_row_count(roller_day, 3);
// Style active item with accent color
lv_obj_set_style_text_color(roller_day, my_accent_color, LV_PART_SELECTED);
```

## 4. Icons & Fonts

### Fonts Strategy & Sizes
LVGL requires fonts to be rasterized (or compiled) at specific sizes. Unlike the web (CSS), you cannot arbitrarily scale a font without artifacts unless you use the FreeType vector engine (heavy for microcontrollers).

You must generate **separate font files** for each size used.

#### A. Required Font Sizes (Inter)
Used for UI Labels, Headers, and Text.
*   **10px**: Tiny labels (e.g., "FIRMWARE", "INTERFACE", "Used"). *(Consolidated 9px & 10px)*
*   **12px**: Small labels, Button text, Sub-values. *(Consolidated 11px & 12px)*
*   **14px**: Headers (e.g., "SETTINGS", "SENSITIVITY").
*   **16px**: Standard List Item Labels.
*   **18px**: Large Labels, Titles.
*   **20px**: Extra Large Labels (Theme names).
*   **24px**: Huge Labels (Month names).
*   **30px**: Display Titles (Model Name).

#### B. Required Number Fonts (Monospace / Tabular)
Used for large numeric values (Time, Date, Sensor Data).
*   **18px**: Version numbers.
*   **24px**: Year, Percentage units.
*   **36px**: Time (HH:MM), Day of Month.
*   **48px**: Filter Usage Hours.
*   **60px**: Sensitivity, Brightness, Lifetime.

*Optimization Tip: For the large number fonts (36px, 48px, 60px), generate a font containing **only** the characters `0123456789%.:` to save flash memory.*

### Implementation Steps

1.  **Download Font**: Get `Inter-Regular.ttf` and `Inter-Bold.ttf`.
2.  **Converter**: Go to [LVGL Online Font Converter](https://lvgl.io/tools/fontconverter).
3.  **Batch Generate**:
    *   Upload `Inter-Bold.ttf`.
    *   Size: `60`. Range: `0-9`. Name: `lv_font_inter_bold_60`.
    *   Size: `48`. Range: `0-9`. Name: `lv_font_inter_bold_48`.
    *   ...repeat for all sizes.
4.  **Declare in C**:
    ```c
    LV_FONT_DECLARE(lv_font_inter_bold_60);
    LV_FONT_DECLARE(lv_font_inter_regular_16);
    ```
5.  **Apply Style**:
    ```c
    static lv_style_t style_huge_num;
    lv_style_init(&style_huge_num);
    lv_style_set_text_font(&style_huge_num, &lv_font_inter_bold_60);
    ```

### Icons
- **React**: Lucide React (SVG based).
- **LVGL**: 
  - **Option A (Symbol Font)**: Create a custom font file containing only the icons you need using fontello.com or similar, then convert with LVGL Font Converter. Map icons to characters (e.g., `\uF001`).
  - **Option B (Image)**: Convert SVGs to PNGs, then use **LVGL Image Converter** to create C arrays. This is heavier on flash memory but pixel-perfect.
  - **Recommendation**: Symbol Font is better for scalability and recoloring (theme support).

## 5. Theming (Dark/Light)

### Strategy
- Use LVGL's style system.
- Create a `style_dark` and `style_light`.
- When toggling theme, iterate through the object tree or use `lv_disp_set_theme` if implementing a full custom theme structure.

```c
static lv_style_t style_base;
static lv_style_t style_accent;

void init_styles() {
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, lv_color_hex(0x18181b)); // Zinc-900
    lv_style_set_text_color(&style_base, lv_color_white());

    lv_style_init(&style_accent);
    lv_style_set_text_color(&style_accent, lv_color_hex(0xE10600)); // Default Red
}
```

## 6. Hardware Resource Estimation (ESP32)

Based on your specifications (**ESP32, 8MB PSRAM, 16MB Flash, 466x466 AMOLED Display**), the hardware is sufficient but requires careful bus configuration due to the high pixel count.

### Memory Analysis

1.  **Flash (16MB)**:
    *   **Firmware Code**: ~1-2 MB.
    *   **Fonts**: Even with full Cyrillic/Latin sets for all sizes (10px-60px), this will likely take **<2 MB** uncompressed.
    *   **Icons**: As font symbols or bitmaps, negligible (<500 KB).
    *   **Result**: Massive headroom (10MB+ free). No aggressive compression needed.

2.  **RAM (8MB PSRAM)**:
    *   **Frame Buffers**: For a 466x466 screen at 16-bit color (RGB565):
        *   One full buffer = `466 * 466 * 2 bytes` = **~425 KB**.
        *   Two buffers (Double Buffering) = **~850 KB**.
    *   **LVGL Heap**: You have **7+ MB** remaining for caching, widgets, and dynamic objects.
    *   **Result**: 8MB is luxury. You can easily enable **double buffering** in PSRAM.

### Performance & Interface
*   **Interface**: **QSPI (Quad SPI)**.
    *   **Bandwidth**: Excellent. With QSPI, you can push 4 data bits per clock cycle. At 40-80MHz, this provides sufficient throughput for the 466x466 resolution.
    *   **Frame Rate**: You can expect **30-60 FPS** easily, even with full-screen transitions.
*   **Buffer Strategy**:
    *   Allocate **2 full frame buffers** in PSRAM (`2 * 466 * 466 * 2 bytes` ≈ 850 KB).
    *   Use **DMA** to transfer data to the display in the background while the CPU renders the next frame.
*   **LVGL Config**: Enable `LV_USE_GPU_ESP32` or `LV_USE_DMA2D` (if available in your IDF) to further accelerate blending.

## 7. Data Persistence & State Management

Since this is an industrial device, it must retain settings after power loss and keep accurate time.

### A. Non-Volatile Storage (NVS)
Settings such as `Language`, `Sensitivity`, `Brightness`, and `Filter Usage` must be saved to the ESP32's **NVS** (Non-Volatile Storage) partition.

*   **Technology**: NVS Library (part of ESP-IDF).
*   **Strategy**: Save settings only when they change (e.g., when leaving the Settings menu) to avoid wearing out the Flash memory.
*   **Structure**:
    ```c
    typedef struct {
        uint8_t brightness;      // 0-100
        uint8_t sensitivity;     // 10-20
        uint32_t filter_runtime; // in minutes
        uint8_t language_id;     // 0=EN, 1=UA, etc.
    } app_settings_t;
    ```

### B. Sleep Mode (Screen Protection)
To prevent OLED burn-in and save power, implement an inactivity timer.
*   **Logic**: If no input (Rotate/Click) for **30 minutes**, turn off the display (send black frame or disable display controller).
*   **Wake Up**: A single button press wakes the screen. This press should **only** wake the screen and not trigger an action.
*   **Purification**: The fan control logic runs independently of the display state.

## 8. Encoder Logic (State Machine)

Since the hardware has limited input (Rotate + Click), implement a state machine for the UI.

```c
typedef enum {
    STATE_MENU_NAV,
    STATE_EDIT_VALUE,
    STATE_CONFIRM
} ui_state_t;

static void encoder_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_KEY) {
        uint32_t key = lv_indev_get_key(lv_indev_get_act());
        if(key == LV_KEY_ENTER) {
            // Switch state: Enter Edit Mode or Confirm
            if(current_state == STATE_MENU_NAV) {
                 // Enter edit mode for selected item
            }
        }
    }
}
```
