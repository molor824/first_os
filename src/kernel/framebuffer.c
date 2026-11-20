#include "framebuffer.h"
#include "default_font.h"
#include "limine.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

struct limine_framebuffer *framebuffer;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

static uint32_t fb_channel(uint8_t value, uint8_t mask_size, uint8_t mask_shift) {
    uint64_t max = ((uint64_t)1 << mask_size);
    return (uint32_t)(value * max / 255) << mask_shift;
}
void fb_init() {
    if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    for (size_t i = 0; i < framebuffer_request.response->framebuffer_count; i++) {
        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[i];

        if (fb->memory_model == LIMINE_FRAMEBUFFER_RGB) {
            framebuffer = fb;
            return;
        }
    }

    hcf();
}
void fb_pixel_map_color(uint8_t *pixel, color_t color) {
    uint32_t value = fb_channel(color.r, framebuffer->red_mask_size, framebuffer->red_mask_shift)
        | fb_channel(color.g, framebuffer->green_mask_size, framebuffer->green_mask_shift)
        | fb_channel(color.b, framebuffer->blue_mask_size, framebuffer->blue_mask_shift);
    memcpy(pixel, &value, framebuffer->bpp / 8);
}
void fb_clear() {
    memset(framebuffer->address, 0, framebuffer->pitch * framebuffer->height);
}
int fb_write_pixel(point_t point, color_t color) {
    if (point.x >= framebuffer->width) return -1;
    if (point.y >= framebuffer->height) return -1;
    uint32_t bytes_per_pixel = framebuffer->bpp / 8;
    uint32_t index = point.x * bytes_per_pixel + point.y * framebuffer->pitch;

    fb_pixel_map_color(&framebuffer->address[index], color);
    return 0;
}
void fb_write_char(point_t origin, unsigned char ch, color_t fg, color_t bg) {
    glyph_t glyph = glyphs[ch];
    int min_y = FONT_ASCENDER - glyph.bitmap_top;
    int min_x = glyph.bitmap_left;
    int max_y = min_y + glyph.rows;
    int max_x = min_x + glyph.width;

    for (int y = 0; y < FONT_LINE_HEIGHT; y++) {
        int py = origin.y + y;
        if (py < 0 || py >= framebuffer->height) continue;
        for (int x = 0; x < FONT_ADVANCE; x++) {
            int px = origin.x + x;
            if (px < 0 || px >= framebuffer->width) continue;

            color_t color = bg;
            if (x >= min_x && x < max_x && y >= min_y && y < max_y) {
                int glyph_x = x - min_x;
                int glyph_y = y - min_y;
                int buffer_index = glyph_x;
                if (glyph.pitch >= 0) buffer_index += glyph.pitch * glyph_y;
                else buffer_index += -glyph.pitch * (glyph.rows - glyph_y - 1);

                uint8_t value = glyph.buffer[buffer_index];
                color.r = ((int)fg.r * (int)value + (int)bg.r * (int)(255 - value)) / 255;
                color.g = ((int)fg.g * (int)value + (int)bg.g * (int)(255 - value)) / 255;
                color.b = ((int)fg.b * (int)value + (int)bg.b * (int)(255 - value)) / 255;
            }
            fb_write_pixel((point_t){px, py}, color);
        }
    }
}
