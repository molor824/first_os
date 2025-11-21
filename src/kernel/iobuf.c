#include <stdbool.h>
#include "vga.h"
#include "iobuf.h"

uint16_t buffer[IO_BUF_SIZE];
size_t buf_position;
size_t buf_size;
size_t buf_cursor;
uint8_t buf_color = VGA_COLOR_ENTRY(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

size_t buf_index(size_t i) {
    return (buf_position + i) % IO_BUF_SIZE;
}
uint16_t *buf_get(size_t i) {
    return &buffer[buf_index(i)];
}
char *buf_get_char(size_t i) {
    return (char*)&buffer[buf_index(i)];
}
void buf_push(uint16_t x) {
    *buf_get(buf_size) = x;
    if (buf_size < IO_BUF_SIZE) buf_size++;
    else buf_position++;
}
void buf_insert(size_t i, uint16_t x) {
    for (size_t j = buf_size; j > i; j--) {
        *buf_get(j) = *buf_get(j - 1);
    }
    *buf_get(i) = x;
    if (buf_size < IO_BUF_SIZE) buf_size++;
    else buf_position++;
}
void buf_remove(size_t i) {
    if (buf_size == 0) return;
    for (size_t j = i; j < buf_size - 1; j++) {
        *buf_get(j) = *buf_get(j + 1);
    }
    buf_size--;
}
void buf_putc(char c) {
    switch (c) {
        case '\r': // set cursor back to newline, in this case
            while (buf_cursor > 0 && *buf_get_char(buf_cursor - 1) != '\n') {
                buf_cursor--;
            }
            break;
        case '\b': // backspace, pops off character before cursor
            buf_remove(buf_cursor - 1);
            break;
        default:
            if (buf_cursor < buf_size) {
                *buf_get(buf_cursor) = VGA_ENTRY(c, buf_color);
                buf_cursor++;
            } else {
                buf_push(VGA_ENTRY(c, buf_color));
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
    // backtrace the cursor until the characters are out of bounds
    // or the cursor is out of bounds
    size_t cursor = buf_size;
    for (size_t row = 0; row < VGA_HEIGHT && cursor > 0; row++) {
        for (size_t col = 0; col < VGA_WIDTH && cursor > 0; col++) {
            cursor--;
            char ch = *buf_get_char(cursor);
            if (ch == '\n') break;
        }
    }

    if (*buf_get_char(cursor) == '\n') cursor++;
    
    // starting from cursor, iterate until row reaches past
    for (size_t row = 0; row < VGA_HEIGHT; row++) {
        size_t col = 0;
        for (; col < VGA_WIDTH && cursor < buf_size; col++) {
            uint16_t entry = *buf_get(cursor);
            char ch = (char)entry;
            cursor++;
            if (ch == '\n') break;
            else vga_set(row, col, entry);
        }
        for (; col < VGA_WIDTH; col++) {
            vga_set(row, col, 0);
        }
    }
}