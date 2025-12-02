#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "iobuf.h"

#if defined(__linux__)
#error "The compiler is not cross-compiler"
#endif

#if !defined(__i386__)
#error "Must be compiled with i686-elf compiler"
#endif

extern char _kernel_start;
extern char _kernel_end;

void kernel_main(void) {
    printf("Total kernel size in memory: %u bytes\n", &_kernel_end - &_kernel_start - 0xC0000000);
    printf("Hello, World!\n");
    printf("Welcome to My first OS!\n");
    buf_flush();
}
