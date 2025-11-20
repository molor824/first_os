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

const char hexchars[] = "0123456789ABCDEF";

int u32toa(uint32_t a, char *dst, unsigned int radix) {
    if (radix < 2 || radix > 16)
        return -1;

    uint32_t a1 = a;
    int count = 1;
    while (a1 >= radix) {
        count++;
        a1 /= radix;
    }
    if (!dst)
        return count;

    for (size_t i = count; i > 0; i--) {
        dst[i - 1] = hexchars[a % radix];
        a /= radix;
    }

    return count;
}
int i32toa(int32_t a, char *dst, unsigned int radix) {
    if (radix < 2 || radix > 16)
        return -1;

    if (a < 0) {
        if (dst) {
            dst[0] = '-';
            return u32toa(-a, &dst[1], radix) + 1;
        } else {
            return u32toa(-a, dst, radix) + 1;
        }
    }
    return u32toa(a, dst, radix);
}

__attribute__((constructor)) void address_print(void) {
    buf_puts("Function stack: ");
    int esp;
    __asm__ __volatile__("movl %%esp, %0"
                         : "=r"(esp));
    char buffer[0x20];
    size_t count = u32toa((uint32_t)esp, buffer, 16);
    buf_write(buffer, count);
    buf_puts("\n");
    buf_flush();
}

void kernel_main(void) {
    buf_puts("Hello, World!\n");
    buf_puts("Welcome to My first OS!\n");
    buf_flush();

    for (size_t i = 0; ; i++) {
        char buffer[0x100];
        size_t count = u32toa(i, buffer, 2);
        buf_putc('\r');
        buf_write(buffer, count);
        buf_flush();
    }
}
