.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO // Multiboot flag field
.set MAGIC, 0x1BADB002 // Magic number, finds bootloader
.set CHECKSUM, -(MAGIC + FLAGS) // Checksum

// Declare multiboot header that marks the program as kernel
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

// Define stack
.section .bss
.align 16
stack_bottom:
.skip 16384 // 16KiB
stack_top:

// starting point
.section .text
.global _start
.type _start, @function
_start:
    // Bootloader loads us to 32-bit protected mode
    // Interrupts, paging are disabled

    // setup stack
    mov $stack_top, %esp

    call kernel_main

    // put in halt
    cli
.halt:
    hlt
    jmp .halt

.size _start, . - _start
