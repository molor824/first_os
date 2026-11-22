#include "stdio.h"
#include "iobuf.h"
#include "printf.h"
#include "string.h"

#define MAX_BUFFER (sizeof(long long) * 8 + 2)

static int stdout_write(void *data, const void *buf, size_t size) {
    buf_write(buf, size);
    return size;
}
static int stdout_flush(void *data) {
    buf_flush();
    return 0;
}

file_handle_t stdout_handle;
file_ptr_t file_ptrs[MAX_FILE_PTRS];

void stdio_init(void) {
    stdout_handle = (file_handle_t){
        .write = stdout_write,
        .flush = stdout_flush,
    };
    file_ptrs[STDOUT] = (file_ptr_t){
        .handle = &stdout_handle,
    };
    printf("STDOUT setup complete.\n");
}

int fd_reg(file_ptr_t ptr) {
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
int read(int fd, void *buf, size_t size) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // Invalid file descriptor or read not supported
    }
    int result = -1;
    file_ptr_t ptr = file_ptrs[fd];
    if (ptr.handle->read)
        result = ptr.handle->read(ptr.data, buf, size);
    return result;
}
int write(int fd, const void *buf, size_t size) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // Invalid file descriptor or write not supported
    }
    int result = -1;
    file_ptr_t ptr = file_ptrs[fd];
    if (ptr.handle->write)
        result = ptr.handle->write(ptr.data, buf, size);
    return result;
}
int close(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1; // File descriptor does not exist
    }
    int result = 0;
    if (file_ptrs[fd].handle->close)
        result = file_ptrs[fd].handle->close(file_ptrs[fd].data);
    file_ptrs[fd] = (file_ptr_t){};
    return result;
}
int flush(int fd) {
    if (fd < 0 || fd >= MAX_FILE_PTRS || file_ptrs[fd].handle == NULL) {
        return -1;
    }
    file_ptr_t ptr = file_ptrs[fd];
    int result = 0;
    if (ptr.handle->flush)
        result = ptr.handle->flush(ptr.data);
    return result;
}
int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int count = vfprintf(STDOUT, format, ap);
    va_end(ap);
    return count;
}
int vprintf(const char *format, va_list ap) {
    return vfprintf(STDOUT, format, ap);
}

int fprintf(int fd, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int count = vfprintf(fd, format, ap);
    va_end(ap);
    return count;
}

#define _WRITE_CHECK(result, fd, buf, size) if ((result = fwrite(fd, buf, size)) < size) \
                                                return result < 0 ? result : count + result

static void vfprintf_out(char ch, void *data) {
    int fd = *(int*)data;
    write(fd, &ch, 1);
}

int vfprintf(int fd, const char *format, va_list ap) {
    return vfctprintf(vfprintf_out, &fd, format, ap);
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int count = vsprintf(str, format, ap);
    va_end(ap);
    return count;
}

int vsprintf_write(void *data, const void *buf, size_t size) {
    struct {
        char *str;
        unsigned pos;
    } *d = data;

    if (d->str)
        memcpy(&d->str[d->pos], buf, size);
    d->pos += size;
    return size;
}

// If null, it returns the count without writing
int vsprintf(char *str, const char *format, va_list ap) {
    struct {
        char *str;
        unsigned pos;
    } data = {
        .str = str,
        .pos = 0
    };
    // Create temporary file handle
    file_handle_t handle = {
        .write = vsprintf_write,
    };
    int fd = fd_reg((file_ptr_t){.data = &data, .handle = &handle});
    int count = vfprintf(fd, format, ap);
    close(fd);
    return count;
}

void putchar_(char c) {
    buf_putc(c);
}
