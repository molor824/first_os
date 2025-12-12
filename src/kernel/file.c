#include "file.h"
#include "iobuf.h"
#include "string.h"
#include "stdio.h"

static int stdout_write(void *data, const void *buf, unsigned size) {
    buf_write(buf, size);
    return size;
}
static int stdout_flush(void *data) {
    buf_flush();
    return 0;
}

file_handle_t stdout_handle;
file_ptr_t file_ptrs[MAX_FILE_PTRS];

void file_init(void) {
    stdout_handle = (file_handle_t){
        .write = stdout_write,
        .flush = stdout_flush,
    };
    file_ptrs[STDOUT] = (file_ptr_t){
        .handle = &stdout_handle,
    };
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
int fread(int fd, void *buf, unsigned size) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // Invalid file descriptor or read not supported
    }
    int result = 0;
    file_ptr_t ptr = file_ptrs[fd];
    if (ptr.handle->read)
        result = ptr.handle->read(ptr.data, buf, size);
    return result;
}
int fwrite(int fd, const void *buf, unsigned size) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // Invalid file descriptor or write not supported
    }
    int result = 0;
    file_ptr_t ptr = file_ptrs[fd];
    if (ptr.handle->write)
        result = ptr.handle->write(ptr.data, buf, size);
    return result;
}
int favl(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // Invalid file descriptor or available not supported
    }
    int result = 0;
    file_ptr_t ptr = file_ptrs[fd];
    if (ptr.handle->available)
        result = ptr.handle->available(ptr.data);
    return result;
}
int fclose(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // File descriptor does not exist
    }
    int result = 0;
    if (file_ptrs[fd].handle->close)
        result = file_ptrs[fd].handle->close(file_ptrs[fd].data);
    file_ptrs[fd] = (file_ptr_t){};
    return result;
}
int fflush(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1;
    }
    file_ptr_t ptr = file_ptrs[fd];
    int result = 0;
    if (ptr.handle->flush)
        result = ptr.handle->flush(ptr.data);
    return result;
}
