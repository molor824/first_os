#ifndef __FRAMEBUFFER_H
#define __FRAMEBUFFER_H

#include <stdint.h>

typedef struct {
    uint8_t *address;
    uint32_t address_high;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t type;
    uint8_t reserved;
    uint8_t padding;
    uint8_t red_field_position;
    uint8_t red_mask_size;
    uint8_t green_field_position;
    uint8_t green_mask_size;
    uint8_t blue_field_position;
    uint8_t blue_mask_size;
} __attribute__((packed)) framebuffer_info_t;

typedef struct {
    uint32_t x, y;
} point_t;

typedef struct {
    uint8_t r, g, b;
} color_t;

extern framebuffer_info_t *framebuffer_info;

extern void fb_clear(void);
extern void fb_pixel_map_color(uint8_t *pixel, color_t color);
extern int fb_write_pixel(point_t point, color_t color);
extern void fb_write_char(point_t origin, char ch, color_t fg, color_t bg);

#endif