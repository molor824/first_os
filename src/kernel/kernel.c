#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "iobuf.h"
#include "file.h"
#include "vga.h"

#if defined(__linux__)
#error "The compiler is not cross-compiler"
#endif

#if !defined(__i386__)
#error "Must be compiled with i686-elf compiler"
#endif

extern char _kernel_start;
extern char _kernel_end;
extern char KERNEL_ADDR;

void kernel_main(void) {
    printf("Kernel: %p\n", &KERNEL_ADDR);
    printf("VGA Buffer: %p\n", &VGA_BUFFER_ADDR);
    printf("Total kernel size in memory: %u bytes\n", &_kernel_end - &_kernel_start - (long)&KERNEL_ADDR);
    printf("Hello, World!\n");
    printf("Welcome to My first OS!\n");
    fflush(STDOUT);
}
