#ifndef _IO_BUF_H
#define _IO_BUF_H

#include <stddef.h>

#define IO_BUF_SIZE 0x10000

extern uint16_t buffer[IO_BUF_SIZE];
extern size_t buf_position;
extern size_t buf_size;
extern size_t buf_cursor;
extern uint8_t buf_color;

void buf_putc(char c);
void buf_write(const void *buf, size_t count);
void buf_puts(const char *s);
void buf_flush(void);

#endif