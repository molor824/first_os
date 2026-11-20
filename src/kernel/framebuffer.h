#pragma once

#include "kernel.h"
#include "limine.h"
#include <stdint.h>

typedef struct {
    uint32_t x, y;
} point_t;

typedef struct {
    uint8_t r, g, b;
} color_t;

extern struct limine_framebuffer framebuffer;

void fb_clear(void);
uint32_t fb_color_value(color_t color);
void fb_fill_rect(point_t origin, point_t end, color_t fill_color);
int fb_write_pixel(point_t point, color_t color);
void fb_write_char(point_t origin, unsigned char ch, color_t fg, color_t bg);
void fb_init(void);
