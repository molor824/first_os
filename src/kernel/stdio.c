#include "stdio.h"
#include "itoa.h"
#include "iobuf.h"
#include "string.h"

#define MAX_BUFFER (sizeof(long long) * 8 + 2)

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

// Formats and writes to fd.
// It prints until fwrite returns zero or less
// More in detail format specifiers aren't implemented yet
// TODO: Implement floating point, and other format specifiers
int vfprintf(int fd, const char *format, va_list ap) {
    int count = 0;
    enum {
        PRINT_STATE_PRINT,
        PRINT_STATE_FORMAT
    } state = PRINT_STATE_PRINT;
    int print_start = 0;
    int result;

    for (int i = 0; format[i]; i++) {
        char ch = format[i];

        switch (state) {
            case PRINT_STATE_PRINT:
                if (ch == '%') {
                    int size = i - print_start;
                    _WRITE_CHECK(result, fd, &format[print_start], size);
                    count += size;
                    state = PRINT_STATE_FORMAT;
                    print_start = i;
                }
                break;
            case PRINT_STATE_FORMAT: ;
                char buffer[MAX_BUFFER] = {0};
                switch (ch) {
                    case 'c': ;
                        char c = (char)va_arg(ap, int);
                        _WRITE_CHECK(result, fd, &c, 1);
                        count++;
                        break;
                    case 's': ;
                        const char *s = va_arg(ap, const char *);
                        int len = strlen(s);
                        _WRITE_CHECK(result, fd, s, len);
                        count += len;
                        break;
                    // all integer printing
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X':
                    case 'p': ;
                        switch (ch) {
                            case 'd':
                            case 'i':
                                len = itoa(va_arg(ap, int), buffer, 10);
                                break;
                            case 'u':
                                len = utoa(va_arg(ap, unsigned), buffer, 10);
                                break;
                            case 'o':
                                len = utoa(va_arg(ap, unsigned), buffer, 8);
                                break;
                            case 'x':
                                len = utoa(va_arg(ap, unsigned), buffer, 16);
                                break;
                            case 'X':
                                len = utoa(va_arg(ap, unsigned), buffer, 16);
                                for (int j = 0; j < len; j++) {
                                    if (buffer[j] >= 'a' && buffer[j] <= 'f')
                                        buffer[j] -= 'a' - 'A';
                                }
                                break;
                            case 'p':
                                len = ultoa((unsigned long)va_arg(ap, void *), buffer, 16);
                                break;
                        }
                        _WRITE_CHECK(result, fd, buffer, len);
                        count += len;
                        break;
                    case '%':
                        _WRITE_CHECK(result, fd, "%", 1);
                        count++;
                        break;
                }
                state = PRINT_STATE_PRINT;
                print_start = i + 1;
                break;
        }
    }
    // Write remaining part
    int len = strlen(&format[print_start]);
    _WRITE_CHECK(result, fd, &format[print_start], len);
    count += len;
    return count;
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int count = vsprintf(str, format, ap);
    va_end(ap);
    return count;
}

int vsprintf_write(void *data, const void *buf, unsigned size) {
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
    int fd = freg((file_ptr_t){.data = &data, .handle = &handle});
    int count = vfprintf(fd, format, ap);
    fclose(fd);
    return count;
}
