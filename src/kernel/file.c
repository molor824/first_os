#include "file.h"
#include "iobuf.h"
#include "string.h"
#include "stdio.h"

int stdout_write(void *data, const void *buf, unsigned size) {
    buf_write(buf, size);
    return size;
}

file_handle_t stdout_handle;
file_ptr_t file_ptrs[MAX_FILE_HANDLES];

__attribute__((constructor(FILE_CRT_PRIORITY))) void file_crt(void) {
    stdout_handle.write = stdout_write;
    file_ptrs[STDOUT].handle = &stdout_handle;
    printf("STDOUT setup complete.\n");
}

int file_register(file_ptr_t ptr) {
    if (ptr.handle == NULL) {
        return -1; // Invalid handle
    }
    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        if (file_ptrs[i].handle == NULL) {
            file_ptrs[i] = ptr;
            return i; // Return the file descriptor
        }
    }
    return -1; // No available slot
}
int file_unregister(int fd) {
    if (fd < 0 || fd >= MAX_FILE_HANDLES) {
        return -1; // Invalid file descriptor
    }
    file_ptrs[fd] = (file_ptr_t){0}; // Clear the entry
    return 0; // Success
}
int file_read(int fd, void *buf, unsigned size) {
    if (fd < 0 || fd >= MAX_FILE_HANDLES || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->read == NULL) {
        return -1; // Invalid file descriptor or read not supported
    }
    return file_ptrs[fd].handle->read(file_ptrs[fd].data, buf, size);
}
int file_write(int fd, const void *buf, unsigned size) {
    if (fd < 0 || fd >= MAX_FILE_HANDLES || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->write == NULL) {
        return -1; // Invalid file descriptor or write not supported
    }
    return file_ptrs[fd].handle->write(file_ptrs[fd].data, buf, size);
}
int file_available(int fd) {
    if (fd < 0 || fd >= MAX_FILE_HANDLES || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->available == NULL) {
        return -1; // Invalid file descriptor or available not supported
    }
    return file_ptrs[fd].handle->available(file_ptrs[fd].data);
}
int file_close(int fd) {
    if (fd < 0 || fd >= MAX_FILE_HANDLES || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->close == NULL) {
        return -1; // Invalid file descriptor or close not supported
    }
    return file_ptrs[fd].handle->close(file_ptrs[fd].data);
}
