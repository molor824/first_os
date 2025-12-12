.set MAGIC, 0xE85250D6 // Magic number, finds bootloader
.set ARCH, 0
.set WIDTH, 800
.set HEIGHT, 600

.set FRAMEBUFFER_INFO, 8

// Paging
.set PDE_INDEX, 0x300
.set KERNEL_ADDR, PDE_INDEX*0x400000 // Kernel virtual base address

.global KERNEL_ADDR
.global PDE_INDEX
.global stack_top

.section .multiboot2
.align 8
multiboot2_header:
    .long MAGIC
    .long ARCH
    .long multiboot2_end - multiboot2_header
    .long -(MAGIC + ARCH + multiboot2_end - multiboot2_header)

.align 8
framebuffer_request:
    .long 5
    .long 20
    .long WIDTH
    .long HEIGHT
    .long 24 // BPP, guaranteed to return direct RGB indexing

.align 8
    .long 0
    .long 0x8
multiboot2_end:

// Define stack
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 0x100000 // 1 MB stack
stack_top:

.section .multiboot.text, "a"
.extern kernel_main

.global _start
.type _start, @function
_start:
    // Initialize stack
    mov $(stack_top - KERNEL_ADDR), %esp
    // Call kernel_main
    call kernel_main
