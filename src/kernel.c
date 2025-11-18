#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <iobuf.h>

#if defined(__linux__)
#error "The compiler is not cross-compiler"
#endif

#if !defined(__i386__)
#error "Must be compiled with i686-elf compiler"
#endif

size_t u32toa(uint32_t a, char *dst) {
    uint32_t a1 = a;
    size_t count = 1;
    while (a1 >= 10) {
        count++;
        a1 /= 10;
    }
    if (!dst)
        return count;

    for (size_t i = count; i > 0; i--) {
        dst[i - 1] = (char)(a % 10) + '0';
        a /= 10;
    }

    return count;
}

void kernel_main(void) {
    buf_puts("Hello, World!\n");
    buf_puts("Welcome to My first OS!\n");
    buf_flush();

    for (size_t i = 0; ; i++) {
        char buffer[0x100];
        size_t count = u32toa(i, buffer);
        buf_putc('\r');
        buf_write(buffer, count);
        buf_flush();
    }
}
