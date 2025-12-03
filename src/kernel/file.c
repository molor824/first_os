#include "file.h"
#include "iobuf.h"
#include "string.h"
#include "stdio.h"

int stdout_write(void *data, const void *buf, unsigned size) {
    buf_write(buf, size);
    return size;
}
int stdout_flush(void *data) {
    buf_flush();
    return 0;
}

file_handle_t stdout_handle;
file_ptr_t file_ptrs[MAX_FILE_PTRS];

__attribute__((constructor(FILE_CRT_PRIORITY))) void file_crt(void) {
    stdout_handle.write = stdout_write;
    stdout_handle.flush = stdout_flush;
    file_ptrs[STDOUT].handle = &stdout_handle;
    printf("STDOUT setup complete.\n");
}

int freg(file_ptr_t ptr) {
    if (ptr.handle == NULL) {
        return -1; // Invalid handle
    }
    for (int i = 0; i < MAX_FILE_PTRS; i++) {
        if (file_ptrs[i].handle == NULL) {
            file_ptrs[i] = ptr;
            return i; // Return the file descriptor
        }
    }
    return -1; // No available slot
}
int funreg(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS) {
        return -1; // Invalid file descriptor
    }
    file_ptrs[fd] = (file_ptr_t){0}; // Clear the entry
    return 0; // Success
}
int fread(int fd, void *buf, unsigned size) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->read == NULL) {
        return -1; // Invalid file descriptor or read not supported
    }
    return file_ptrs[fd].handle->read(file_ptrs[fd].data, buf, size);
}
int fwrite(int fd, const void *buf, unsigned size) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->write == NULL) {
        return -1; // Invalid file descriptor or write not supported
    }
    return file_ptrs[fd].handle->write(file_ptrs[fd].data, buf, size);
}
int favl(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->available == NULL) {
        return -1; // Invalid file descriptor or available not supported
    }
    return file_ptrs[fd].handle->available(file_ptrs[fd].data);
}
int fclose(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->close == NULL) {
        return -1; // Invalid file descriptor or close not supported
    }
    return file_ptrs[fd].handle->close(file_ptrs[fd].data);
}
int fflush(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL || file_ptrs[fd].handle->flush == NULL) {
        return -1;
    }
    return file_ptrs[fd].handle->flush(file_ptrs[fd].data);
}
