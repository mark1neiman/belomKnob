/**
 * @file styles_colors.h
 * @brief Color definitions for BELOM A2 UI (LVGL Reference)
 */

#ifndef STYLES_COLORS_H
#define STYLES_COLORS_H

// --- Fixed Status Colors (Do not change with theme) ---

#define COLOR_CRITICAL   0xEF4444  // Red-500
#define COLOR_WARNING    0xEAB308  // Amber-500
#define COLOR_GOOD       0x10B981  // Emerald-500

// --- Overdrive Color ---
#define COLOR_OVERDRIVE_TEXT  0xEF4444 // Same as Critical

// --- Theme Colors (Dark Mode) ---
#define COLOR_BG_DARK    0x09090b  // Zinc-950
#define COLOR_TEXT_DARK  0xf4f4f5  // Zinc-100
#define COLOR_RING_DARK  0x52525b  // Zinc-600 (Inactive ring segments)

// --- Theme Colors (Light Mode) ---
#define COLOR_BG_LIGHT   0xf4f4f5  // Zinc-100
#define COLOR_TEXT_LIGHT 0x18181b  // Zinc-900
#define COLOR_RING_LIGHT 0xa1a1aa  // Zinc-400 (Inactive ring segments)

// --- Accent Colors (User Selectable) ---
#define ACCENT_RED       0xE10600  // Default BELOM Red
#define ACCENT_ORANGE    0xF97316
#define ACCENT_AMBER     0xEAB308
#define ACCENT_GREEN     0x10B981
#define ACCENT_CYAN      0x06B6D4
#define ACCENT_BLUE      0x3B82F6
#define ACCENT_PURPLE    0x8B5CF6
#define ACCENT_PINK      0xEC4899

#endif // STYLES_COLORS_H
