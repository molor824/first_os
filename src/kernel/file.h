#ifndef __FILE_H
#define __FILE_H

#define STDOUT 0
#define MAX_FILE_HANDLES 64

typedef int (*read_cb)(void *data, void *buf, unsigned size);
typedef int (*write_cb)(void *data, const void *buf, unsigned size);
typedef int (*available_cb)(void *data);
typedef int (*close_cb)(void *data);

// Interface struct containing methods for file operations
typedef struct file_handle {
    read_cb read;
    write_cb write;
    available_cb available;
    close_cb close;
    void *data;
} file_handle_t;

// TODO: Extend kernel to 2 page directory, and make file_map support atleast 1MiB worth of file handlers
extern file_handle_t file_handles[MAX_FILE_HANDLES];
extern file_handle_t *stdout_handle;

int file_register(file_handle_t handle);
// NOTE: file_unregister does not close the file, it only removes it from the file map
int file_unregister(int fd);
int file_read(int fd, void *buf, unsigned size);
int file_write(int fd, const void *buf, unsigned size);
int file_available(int fd);
int file_close(int fd);

#endif
