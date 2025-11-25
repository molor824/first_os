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

__attribute__((constructor)) void address_print(void) {
    int esp;
    __asm__ __volatile__("movl %%esp, %0"
                         : "=r"(esp));
    printf("ini section esp: %p\n", esp);
    buf_flush();
}

void kernel_main(void) {
    printf("Hello, World!\n");
    printf("Welcome to My first OS!\n");
    buf_flush();

    for (size_t i = 0; ; i++) {
        printf("Counter: %d\n", i);
        buf_flush();
    }
}
