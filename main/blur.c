#include "lvgl.h"

static inline int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline uint16_t bswap16_u16(uint16_t v)
{
    return (uint16_t)((v << 8) | (v >> 8));
}

static inline uint16_t unpack_native(uint16_t raw, bool swapped)
{
    return swapped ? bswap16_u16(raw) : raw;
}

static inline uint16_t pack_native(uint16_t rgb565, bool swapped)
{
    return swapped ? bswap16_u16(rgb565) : rgb565;
}

static inline uint16_t components_to_rgb565(uint16_t r5, uint16_t g6, uint16_t b5)
{
    return (uint16_t)(((r5 & 0x1F) << 11) | ((g6 & 0x3F) << 5) | (b5 & 0x1F));
}

/*
 * LVGL9-compatible stack/box blur for canvas draw buffer in RGB565.
 * Signature kept as lv_canvas_stackblur_rgba for call-site compatibility.
 */
void lv_canvas_stackblur_rgba(lv_obj_t *canvas, const lv_area_t *area, uint16_t radius)
{
    LV_ASSERT_OBJ(canvas, &lv_canvas_class);

    if (radius < 1) {
        return;
    }

    lv_draw_buf_t *draw_buf = lv_canvas_get_draw_buf(canvas);
    if (!draw_buf || !draw_buf->data) {
        return;
    }

    const bool swapped = (draw_buf->header.cf == LV_COLOR_FORMAT_RGB565_SWAPPED);
    if (draw_buf->header.cf != LV_COLOR_FORMAT_RGB565 && !swapped) {
        return;
    }

    const int32_t w = (int32_t)draw_buf->header.w;
    const int32_t h = (int32_t)draw_buf->header.h;
    if (w <= 0 || h <= 0) {
        return;
    }

    int32_t x1 = 0;
    int32_t y1 = 0;
    int32_t x2 = w - 1;
    int32_t y2 = h - 1;

    if (area) {
        x1 = clamp_i32(area->x1, 0, w - 1);
        y1 = clamp_i32(area->y1, 0, h - 1);
        x2 = clamp_i32(area->x2, 0, w - 1);
        y2 = clamp_i32(area->y2, 0, h - 1);
    }

    if (x1 > x2 || y1 > y2) {
        return;
    }

    const int32_t rw = x2 - x1 + 1;
    const int32_t rh = y2 - y1 + 1;
    const int32_t max_r = (rw > rh ? rw : rh) - 1;
    if ((int32_t)radius > max_r) {
        radius = (uint16_t)max_r;
        if (radius < 1) {
            return;
        }
    }

    const int32_t r = (int32_t)radius;
    const int32_t window = r * 2 + 1;

    uint16_t *tmp = (uint16_t *)lv_malloc((size_t)rw * (size_t)rh * sizeof(uint16_t));
    if (!tmp) {
        return;
    }

    uint16_t *pixels = (uint16_t *)draw_buf->data;
    const int32_t stride_px = (int32_t)(draw_buf->header.stride / sizeof(uint16_t));

    for (int32_t y = y1; y <= y2; y++) {
        uint16_t *row = pixels + y * stride_px;

        int32_t sum_r = 0;
        int32_t sum_g = 0;
        int32_t sum_b = 0;

        for (int32_t k = -r; k <= r; k++) {
            int32_t xx = clamp_i32(x1 + k, x1, x2);
            uint16_t p = unpack_native(row[xx], swapped);
            sum_r += (p >> 11) & 0x1F;
            sum_g += (p >> 5) & 0x3F;
            sum_b += p & 0x1F;
        }

        for (int32_t x = x1; x <= x2; x++) {
            uint16_t out_r = (uint16_t)((sum_r + window / 2) / window);
            uint16_t out_g = (uint16_t)((sum_g + window / 2) / window);
            uint16_t out_b = (uint16_t)((sum_b + window / 2) / window);
            tmp[(y - y1) * rw + (x - x1)] = components_to_rgb565(out_r, out_g, out_b);

            int32_t x_sub = clamp_i32(x - r, x1, x2);
            int32_t x_add = clamp_i32(x + r + 1, x1, x2);

            uint16_t p_sub = unpack_native(row[x_sub], swapped);
            uint16_t p_add = unpack_native(row[x_add], swapped);

            sum_r += ((p_add >> 11) & 0x1F) - ((p_sub >> 11) & 0x1F);
            sum_g += ((p_add >> 5) & 0x3F) - ((p_sub >> 5) & 0x3F);
            sum_b += (p_add & 0x1F) - (p_sub & 0x1F);
        }
    }

    for (int32_t x = 0; x < rw; x++) {
        int32_t sum_r = 0;
        int32_t sum_g = 0;
        int32_t sum_b = 0;

        for (int32_t k = -r; k <= r; k++) {
            int32_t yy = clamp_i32(k, 0, rh - 1);
            uint16_t p = tmp[yy * rw + x];
            sum_r += (p >> 11) & 0x1F;
            sum_g += (p >> 5) & 0x3F;
            sum_b += p & 0x1F;
        }

        for (int32_t y = 0; y < rh; y++) {
            uint16_t out_r = (uint16_t)((sum_r + window / 2) / window);
            uint16_t out_g = (uint16_t)((sum_g + window / 2) / window);
            uint16_t out_b = (uint16_t)((sum_b + window / 2) / window);

            uint16_t out_p = components_to_rgb565(out_r, out_g, out_b);
            pixels[(y1 + y) * stride_px + (x1 + x)] = pack_native(out_p, swapped);

            int32_t y_sub = clamp_i32(y - r, 0, rh - 1);
            int32_t y_add = clamp_i32(y + r + 1, 0, rh - 1);

            uint16_t p_sub = tmp[y_sub * rw + x];
            uint16_t p_add = tmp[y_add * rw + x];

            sum_r += ((p_add >> 11) & 0x1F) - ((p_sub >> 11) & 0x1F);
            sum_g += ((p_add >> 5) & 0x3F) - ((p_sub >> 5) & 0x3F);
            sum_b += (p_add & 0x1F) - (p_sub & 0x1F);
        }
    }

    lv_free(tmp);
    lv_obj_invalidate(canvas);
}
