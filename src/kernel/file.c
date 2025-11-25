#include "file.h"
#include "iobuf.h"
#include "string.h"

int stdout_write(void *data, const void *buf, unsigned size) {
    buf_write(buf, size);
    return size;
}

file_handle_t file_handles[MAX_FILE_HANDLES] = {
    [0] = {
        .write = stdout_write,
    }
};
file_handle_t *stdout_handle = &file_handles[STDOUT];

int file_register(file_handle_t handle) {
    file_handle_t null_handle = {0};
    if (memcmp(&handle, &null_handle, sizeof(file_handle_t)) == 0) {
        return -1; // Invalid handle
    }
    for (int i = 0; i < 256; i++) {
        if (memcmp(&file_handles[i], &null_handle, sizeof(file_handle_t)) == 0) {
            file_handles[i] = handle;
            return i; // Return the file descriptor
        }
    }
    return -1; // No available slot
}
int file_unregister(int fd) {
    if (fd < 0 || fd >= 256) {
        return -1; // Invalid file descriptor
    }
    file_handles[fd] = (file_handle_t){0}; // Clear the entry
    return 0; // Success
}
int file_read(int fd, void *buf, unsigned size) {
    if (fd < 0 || fd >= 256 || file_handles[fd].read == NULL) {
        return -1; // Invalid file descriptor or read not supported
    }
    return file_handles[fd].read(file_handles[fd].data, buf, size);
}
int file_write(int fd, const void *buf, unsigned size) {
    if (fd < 0 || fd >= 256 || file_handles[fd].write == NULL) {
        return -1; // Invalid file descriptor or write not supported
    }
    return file_handles[fd].write(file_handles[fd].data, buf, size);
}
int file_available(int fd) {
    if (fd < 0 || fd >= 256 || file_handles[fd].available == NULL) {
        return -1; // Invalid file descriptor or available not supported
    }
    return file_handles[fd].available(file_handles[fd].data);
}
int file_close(int fd) {
    if (fd < 0 || fd >= 256 || file_handles[fd].close == NULL) {
        return -1; // Invalid file descriptor or close not supported
    }
    return file_handles[fd].close(file_handles[fd].data);
}
