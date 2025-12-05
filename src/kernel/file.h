#ifndef __FILE_H
#define __FILE_H

#define FILE_CRT_PRIORITY 10000

#define STDOUT 0
#define MAX_FILE_PTRS 0x10000

typedef int (*read_cb)(void *data, void *buf, unsigned size);
typedef int (*write_cb)(void *data, const void *buf, unsigned size);
typedef int (*available_cb)(void *data);
typedef int (*close_cb)(void *data);
typedef int (*flush_cb)(void *data);

// Handle struct containing methods for accessing file
typedef struct file_handle {
    read_cb read;
    write_cb write;
    available_cb available;
    close_cb close;
    flush_cb flush;
} file_handle_t;

// File ptr that points to data and handle
typedef struct file_ptr {
    void *data;
    file_handle_t *handle;
} file_ptr_t;

extern file_ptr_t file_ptrs[MAX_FILE_PTRS];

int freg(file_ptr_t handle);
int fread(int fd, void *buf, unsigned size);
int fwrite(int fd, const void *buf, unsigned size);
int favl(int fd);
int fclose(int fd);
int fflush(int fd);

#endif
