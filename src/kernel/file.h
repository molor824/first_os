#ifndef __FILE_H
#define __FILE_H

#define FILE_CRT_PRIORITY 10000

#define STDOUT 0
#define MAX_FILE_HANDLES 0x10000

typedef int (*read_cb)(void *data, void *buf, unsigned size);
typedef int (*write_cb)(void *data, const void *buf, unsigned size);
typedef int (*available_cb)(void *data);
typedef int (*close_cb)(void *data);

// Handle struct containing methods for accessing file
typedef struct file_handle {
    read_cb read;
    write_cb write;
    available_cb available;
    close_cb close;
} file_handle_t;

// File ptr that points to data and handle
typedef struct file_ptr {
    void *data;
    file_handle_t *handle;
} file_ptr_t;

// TODO: Extend kernel to 2 page directory, and make file_map support atleast 1MiB worth of file handlers
extern file_ptr_t file_ptrs[MAX_FILE_HANDLES];

int file_register(file_ptr_t handle);
// NOTE: file_unregister does not close the file, it only removes it from the file map
int file_unregister(int fd);
int file_read(int fd, void *buf, unsigned size);
int file_write(int fd, const void *buf, unsigned size);
int file_available(int fd);
int file_close(int fd);

#endif
