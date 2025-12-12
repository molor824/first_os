#include "framebuffer.h"
#include "default_font.h"
#include "string.h"
#include "kernel.h"

framebuffer_info_t *framebuffer_info;

void fb_init(void) {
    framebuffer_info->address = mmap(framebuffer_info->address, framebuffer_info->height * framebuffer_info->pitch);
}
void fb_pixel_map_color(uint8_t *pixel, color_t color) {
    uint32_t bytes_per_pixel = framebuffer_info->bpp / 8;
    memset(pixel, 0, bytes_per_pixel);

    color.r = color.r >> (8 - framebuffer_info->red_mask_size);
    color.g = color.g >> (8 - framebuffer_info->red_mask_size);
    color.b = color.b >> (8 - framebuffer_info->red_mask_size);

    pixel[framebuffer_info->red_field_position / 8] |= color.r << (framebuffer_info->red_field_position % 8);
    pixel[framebuffer_info->red_field_position / 8 + 1] |= color.r >> (8 - framebuffer_info->red_field_position % 8);

    pixel[framebuffer_info->green_field_position / 8] |= color.g << (framebuffer_info->green_field_position % 8);
    pixel[framebuffer_info->green_field_position / 8 + 1] |= color.g >> (8 - framebuffer_info->green_field_position % 8);

    pixel[framebuffer_info->blue_field_position / 8] |= color.b << (framebuffer_info->blue_field_position % 8);
    pixel[framebuffer_info->blue_field_position / 8 + 1] |= color.b >> (8 - framebuffer_info->blue_field_position % 8);
}
void fb_clear() {
    memset(framebuffer_info->address, 0, framebuffer_info->pitch * framebuffer_info->height);
}
int fb_write_pixel(point_t point, color_t color) {
    if (point.x >= framebuffer_info->width) return -1;
    if (point.y >= framebuffer_info->height) return -1;
    uint32_t bytes_per_pixel = framebuffer_info->bpp / 8;
    uint32_t index = point.x * bytes_per_pixel + point.y * framebuffer_info->pitch;

    fb_pixel_map_color(&framebuffer_info->address[index], color);
    return 0;
}
void fb_write_char(point_t origin, char ch, color_t fg, color_t bg) {
    glyph_t glyph = glyphs[(int)ch];
    int draw_x = origin.x + glyph.bitmap_left;
    int draw_y = origin.y - glyph.bitmap_top;

    for (int y = 0; y < glyph.rows; y++) {
        int py = draw_y + y;
        if (py < 0 || py >= framebuffer_info->height) continue;
        for (int x = 0; x < glyph.width; x++) {
            int px = draw_x + x;
            if (px < 0 || px >= framebuffer_info->width) continue;
            int buffer_index = x;
            if (glyph.pitch >= 0) buffer_index += glyph.pitch * y;
            else buffer_index += -glyph.pitch * (glyph.rows - y - 1);

            uint8_t value = glyph.buffer[buffer_index];
            color_t color = {
                .r = fg.r * value / 255 + bg.r * (255 - value) / 255,
                .g = fg.g * value / 255 + bg.g * (255 - value) / 255,
                .b = fg.b * value / 255 + bg.b * (255 - value) / 255
            };
            
            fb_write_pixel((point_t){px, py}, color);
        }
    }
}
