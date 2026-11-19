#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mem.h"
#include "string.h"
#include "stdio.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "kernel.h"
#include "kbd.h"
#include "inst.h"
#include "irq_event.h"
#include "framebuffer.h"

#if defined(__linux__)
#error "The compiler is not cross-compiler"
#endif

#if !defined(__i386__)
#error "Must be compiled with i686-elf compiler"
#endif

// NOTE to self: This is fine, because each PDE points to 1024 PTE, which is exactly 4096 in length
// So no matter what this is aligned!
uint32_t page_directory_entries[1024]; __attribute__((aligned(4096)))
uint32_t page_tables_entries[1024 * 1024]; __attribute__((aligned(4096)))
size_t vmem_end_addr = (size_t)&KERNEL_ADDR;

boot_info_t *boot_info;

void bootinfo_init(void) {
    boot_info_t *boot_info_v = physmem_map(boot_info, sizeof(boot_info_t));
    while ((size_t)boot_info_v + boot_info_v->total_size > vmem_end_addr) {
        size_t allocated_size = vmem_end_addr - (size_t)boot_info_v;
        size_t remaining_size = boot_info_v->total_size - allocated_size;
        physmem_map((void*)boot_info + allocated_size, remaining_size);
    }

    boot_info = boot_info_v;
    void *tag_address = (void*)boot_info->tags;
    size_t tag_offset = 0;
    size_t tag_size = boot_info->total_size - sizeof(boot_info_t);

    while (tag_offset < tag_size) {
        boot_tag_t *tag = (boot_tag_t*)(tag_address + tag_offset);

        if (tag->type == 0) break;
        if (tag->type == FRAMEBUFFER_INFO) {
            framebuffer_info = (framebuffer_info_t*)tag;
        } else if (tag->type == MEMORY_MAP_INFO) {
            memmap_info = (memmap_info_t*)tag;
        }

        tag_offset = (tag_offset + tag->size + 7) & -8;
    }
}
void virtual_main(void) {
    // Unmap identity
    page_directory_entries[0] = 0;

    REFRESH_CR3();

    bootinfo_init();
    fb_init();
    file_init();
    gdt_init();
    idt_init();
    PIC_init();

    _init();

    printf("Kernel: %p\n", &KERNEL_ADDR);
    printf("Kernel physical address: %p, kernel physical end address: %p\n", &_kernel_start, &_kernel_end - &KERNEL_ADDR);
    printf("Total kernel size in memory: %u bytes\n", (size_t)&_kernel_end - (size_t)&KERNEL_ADDR - (size_t)&_kernel_start);

    printf("Full ASCII test\n");
    for (int i = ' '; i < 128; i++) {
        printf("%c", i);
        if (i % 8 == 7) printf("\n");
    }

    printf("Hello, World!\n");
    printf("Welcome to My first OS!\n");
    fflush(STDOUT);

    sti();

    for (;;) {
        irq_event_t event;
        while (irq_event_pop(&event)) {
            switch (event.type) {
                case IRQ_EVENT_TIM:
                    fflush(STDOUT);
                    break;
                case IRQ_EVENT_KBD:
                    kbd_push(event.scancode);
                    char c = kbd_charcode();
                    if (c) printf("%c", c);
                    break;
            }
        }
        hlt();
    }

    for (;;) {
        cli();
        hlt();
    }
}

__attribute__((section(".multiboot.text")))
void kernel_main(void) {
    __asm__ volatile("movl %%ebx, %0" : "=m"(*(void**)((void*)&boot_info - (void*)&KERNEL_ADDR)));
    // Setup memory paging
    // Iterating kernel_start to kernel_end, enable page tables entries, and corresponding page directory entries
    size_t address = (size_t)&KERNEL_ADDR;
    size_t phys_address = 0;

    while (address < (size_t)&_kernel_end) {
        if (address % PDE_RANGE == 0) {
            *(uint32_t*)((void*)&page_directory_entries[address / PDE_RANGE] - (void*)&KERNEL_ADDR) =
                (uint32_t)((void*)&page_tables_entries[address / 4096] - (void*)&KERNEL_ADDR) | 3;
        }
        if (phys_address >= ((size_t)&_kernel_start & -4096)) {
            *(uint32_t*)((void*)&page_tables_entries[address / 4096] - (void*)&KERNEL_ADDR) = phys_address | 3;
        }
        address += 4096;
        phys_address += 4096;
    }

    *(uint32_t*)((void*)&vmem_end_addr - (void*)&KERNEL_ADDR) = address;

    // Identity mapping
    *(uint32_t*)((void*)&page_directory_entries[0] - (void*)&KERNEL_ADDR) =
        ((size_t)&page_tables_entries[(size_t)&KERNEL_ADDR / 4096] - (size_t)&KERNEL_ADDR) | 3;

    // Store page directory entry address to cr3
    __asm__ volatile("mov %0, %%cr3" : : "r"((size_t)&page_directory_entries[0] - (size_t)&KERNEL_ADDR));

    // Enable paging and write-protect bit
    __asm__ volatile(
        "mov %cr0, %ecx\n"
        "or $0x80010000, %ecx\n"
        "mov %ecx, %cr0\n"
        "mov $stack_top, %esp\n"
        "mov $virtual_main, %ecx\n"
        "jmp *%ecx"
    );

    for (;;) {
        cli();
        hlt();
    }
}
