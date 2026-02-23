# BELOM A2 Dust Collector UI - LVGL Port Reference

## Overview

This document serves as a reference for porting the BELOM A2 UI concept to LVGL (Light and Versatile Graphics Library). The current implementation is a React-based prototype, but the logic and state management are designed to be translatable to an embedded C environment.

## System Specifications

- **Display**: Circular 38mm (466x466px).
- **Input**: Single Rotary Encoder with Push Button.
  - Rotation (CW/CCW): Navigation / Value Adjustment.
  - Short Press: Selection / Toggle / Confirm.
  - Long Press: Back / Exit Menu.

## State Machine

The system has two main modes:

1. **Main Screen (Gauge View)**: Shows speed, filter status, and lock status.
2. **Settings Menu**: Configuration interface.

### Global State (`app_state_t`)

```c
typedef struct {
    uint8_t speed;           // 0-100 (110 in overdrive)
    filter_status_t filter;  // GOOD, WARNING, CRITICAL
    bool locked;             // Lock screen interaction
    settings_t settings;     // User preferences
} app_state_t;
```

## Logic & Behavior

### 0. Boot / Welcome Screen

- **Trigger**: System Power On.
- **Duration**: ~2.5 seconds.
- **Animation**:
  - Center: Displays Logo (Light/Dark variant based on saved theme) and "BELOM A2" text.
  - Progress Bar: Linear bar fills from 0% to 100%.
  - Segmented Ring: Segments fill sequentially from 0% to 100% in sync with progress.
- **Transition**: Automatically switches to **Main Screen** upon completion.

### 1. Main Screen Logic

- **Encoder Rotate CCW**: Adjusts fan speed.
  - Step size determined by `settings.sensitivity`.
  - Max speed is **100%**.
  - **Exception**: If `filter == CRITICAL` AND `settings.allowOverdrive == true`, max speed is **110%**.
- **Encoder Short Press**: Toggles `locked` state (if `settings.lockMode` is enabled).
- **Encoder Long Press**: Enter **Settings Menu**.
- **Display**:
  - Segmented Ring: Shows speed.
  - Center: Speed value or Icon.
  - Filter Indicator:
    - **Green** (#10B981): Good.
    - **Yellow** (#EAB308): Warning.
    - **Red** (#EF4444): Critical. (Colors are fixed, do not change with theme).
  - Overdrive Warning:
    - Text "OVERDRIVE MODE" appears in **Red** (#EF4444) if speed > 100%.

### 2. Settings Menu Logic

- **Structure**: Wheel/Roller List style.
- **Navigation**:
  - Rotate: Scroll list or adjust value.
  - Short Press: Enter submenu or confirm selection.
  - Long Press: Go Back (Submenu -> Root -> Main Screen).
- **Items**:
  - **Sensitivity**: 15-100 clicks for full range.
  - **Accent Color**: Selection of UI colors.
  - **Language**: EN, RU, DE, FR, EE, FI, LV
  - **Brightness**: 10-100%.
  - **Theme**: Dark / Light.
  - **Rotation**: 0° / 180° (Flip screen).
  - **Lock Mode**: Enable/Disable auto-lock.
  - **Overdrive**: Enable/Disable 110% boost capability.
  - **Filter**:
    - **Lifetime**: Set max hours (50-100h).
    - **Reset Usage**:
      - Submenu: [BACK] [RESET].
      - Must select RESET to clear usage.
      - BACK returns to Filter menu without action.
    - **Back**: Exit to Root Menu.

## Implementation Details for LVGL

### Styles

- **Primary Color**: User configurable (`settings.accentColor`).
- **Critical/Error Color**: Fixed Red (`#EF4444`).
- **Warning Color**: Fixed Amber (`#EAB308`).
- **Success Color**: Fixed Emerald (`#10B981`).

### Components

- **Arc/Ring**: Use `lv_arc` or draw multiple `lv_line` / `lv_img` rotated for the segmented effect.
- **Menu**: `lv_roller` or a custom list with 3D transformation (simulated in React with CSS `perspective`). In LVGL, a standard list or roller is preferred for performance.

### Files in this Directory

- `ui_logic_pseudo.c`: Pseudocode for the main state machine and event handling.
- `styles_colors.h`: Color definitions.