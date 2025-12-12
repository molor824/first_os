#include <stdbool.h>
#include "iobuf.h"
#include "default_font.h"

char buffer[IO_BUF_SIZE];
size_t buf_position = 0;
size_t buf_cursor = 0;
size_t buf_size = 0;
color_t buf_foreground = {255, 255, 255};
color_t buf_background = {0, 0, 0};

static size_t buf_index(size_t i) {
    return (buf_position + i) % IO_BUF_SIZE;
}
static char *buf_get(size_t i) {
    return &buffer[buf_index(i)];
}
static void buf_push(char x) {
    *buf_get(buf_size) = x;
    if (buf_size < IO_BUF_SIZE) buf_size++;
    else buf_position++;
}
static void buf_remove(size_t i) {
    if (buf_size == 0) return;
    for (size_t j = i; j < buf_size - 1; j++) {
        *buf_get(j) = *buf_get(j + 1);
    }
    buf_size--;
}
void buf_putc(char c) {
    switch (c) {
        case '\r': // set cursor back to newline, in this case
            while (buf_cursor > 0 && *buf_get(buf_cursor - 1) != '\n') {
                buf_cursor--;
            }
            break;
        case '\b': // backspace, pops off character before cursor
            buf_remove(buf_cursor - 1);
            break;
        default:
            if (buf_cursor < buf_size) {
                *buf_get(buf_cursor) = c;
                buf_cursor++;
            } else {
                buf_push(c);
                buf_cursor = buf_size;
            }
    }
}
void buf_write(const void *buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buf_putc(((char*)buf)[i]);
    }
}
void buf_puts(const char *s) {
    for (size_t i = 0; s[i]; i++) {
        buf_putc(s[i]);
    }
}
void buf_flush(void) {
    fb_clear();

    size_t fb_width = framebuffer_info->width;
    size_t fb_height = framebuffer_info->height;
    size_t text_rows = fb_height / FONT_LINE_HEIGHT;
    size_t text_cols = fb_width / FONT_ADVANCE;

    size_t rows = 0;
    size_t columns = 0;
    size_t start_cursor = buf_size;
    for (; start_cursor > 0; start_cursor--) {
        char ch = *buf_get(start_cursor - 1);
        if (ch == '\n') {
            rows++;
            columns = 0;
        } else if (ch == '\r') {
            columns = 0;
        } else {
            columns++;
            if (columns >= text_cols) {
                columns = 0;
                rows++;
            }
        }
        if (rows >= text_rows) {
            break;
        }
    }

    point_t pen = {.y = FONT_ASCENDER};
    for (size_t cursor = start_cursor; cursor < buf_size; cursor++) {
        char ch = *buf_get(cursor);
        if (ch == '\n') {
            pen.y += FONT_LINE_HEIGHT;
            pen.x = 0;
        } else if (ch == '\r') {
            pen.x = 0;
        } else {
            fb_write_char(pen, ch, buf_foreground, buf_background);
            pen.x += FONT_ADVANCE;
            if (pen.x >= fb_width) {
                pen.x = 0;
                pen.y += FONT_LINE_HEIGHT;
            }
        }
        if (pen.y >= fb_height) {
            break;
        }
    }
}