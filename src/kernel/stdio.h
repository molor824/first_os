#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>

int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);

int fprintf(int fd, const char *format, ...);
int vfprintf(int fd, const char *format, va_list ap);

int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);

#endif
