#include "framebuffer.h"
#include "default_font.h"
#include "string.h"
#include "kernel.h"

framebuffer_info_t *framebuffer_info;

void fb_init(void) {
    framebuffer_info->address = vmem_map(framebuffer_info->address, framebuffer_info->height * framebuffer_info->pitch);
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
    int min_y = FONT_ASCENDER - glyph.bitmap_top;
    int min_x = glyph.bitmap_left;
    int max_y = min_y + glyph.rows;
    int max_x = min_x + glyph.width;
    
    for (int y = 0; y < FONT_LINE_HEIGHT; y++) {
        int py = origin.y + y;
        if (py < 0 || py >= framebuffer_info->height) continue;
        for (int x = 0; x < FONT_ADVANCE; x++) {
            int px = origin.x + x;
            if (px < 0 || px >= framebuffer_info->height) continue;

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
