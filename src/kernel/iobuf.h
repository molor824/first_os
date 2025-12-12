#ifndef __IO_BUF_H
#define __IO_BUF_H

#include <stdint.h>
#include <stddef.h>
#include "framebuffer.h"

#define IO_BUF_SIZE 0x10000

extern char buffer[IO_BUF_SIZE];
extern size_t buf_size;
extern size_t buf_position;
extern size_t buf_cursor;
extern color_t buf_foreground, buf_background;

void buf_putc(char c);
void buf_write(const void *buf, size_t count);
void buf_puts(const char *s);
void buf_flush(void);

#endif