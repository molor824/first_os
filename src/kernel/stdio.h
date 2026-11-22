#pragma once

#include <stdarg.h>
#include <stddef.h>

#define STDOUT 0
#define MAX_FILE_PTRS 0x10000

typedef int (*read_cb)(void *data, void *buf, size_t size);
typedef int (*write_cb)(void *data, const void *buf, size_t size);
typedef int (*close_cb)(void *data);
typedef int (*flush_cb)(void *data);

// Handle struct containing methods for accessing file
typedef struct file_handle {
    read_cb read;
    write_cb write;
    close_cb close;
    flush_cb flush;
} file_handle_t;

// File ptr that points to data and handle
typedef struct file_ptr {
    void *data;
    file_handle_t *handle;
} file_ptr_t;

int fd_reg(file_ptr_t ptr);
int read(int fd, void *buf, size_t size);
int write(int fd, const void *buf, size_t size);
int close(int fd);
int flush(int fd);

void stdio_init(void);

int printf(const char *format, ...);
int vprintf(const char *format, va_list ap);

int fprintf(int fd, const char *format, ...);
int vfprintf(int fd, const char *format, va_list ap);

int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
