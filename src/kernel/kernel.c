#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "file.h"
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

// _kernel_start is in physical addressing
extern char _kernel_start;
// _kernel_end is in virtual addressing
extern char _kernel_end;

extern char PDE_INDEX;
extern char KERNEL_ADDR;

extern void _init(void);
extern void _fini(void);

// TODO: each page table entries should be allocated on its own 4096 alignment!
// So somehow fucking figure out a way to not pre-allocate all of them and dynamically allocate more
// fuck me...
uint32_t page_directory_entries[1024]; __attribute__((aligned(4096)))
uint32_t page_tables_entries[1024 * 1024]; __attribute__((aligned(4096)))

size_t vmem_end_addr = (size_t)&KERNEL_ADDR;
boot_info_t *boot_info, *phys_boot_info;

void *vmem_map(void *phys_mem, size_t size) {
    size_t start_addr = (size_t)phys_mem & -4096;
    size_t end_addr = (size_t)phys_mem + size;
    size_t phys_addr = start_addr;
    size_t offset = vmem_end_addr - start_addr;
    while (phys_addr < end_addr) {
        if (vmem_end_addr % PDE_RANGE == 0) {
            // NOTE: PDE receives physical address of PTE, not virtual
            size_t idx = vmem_end_addr / PDE_RANGE;
            page_directory_entries[idx] = ((size_t)&page_tables_entries[idx * 1024] - (size_t)&KERNEL_ADDR) | 3;
        }
        page_tables_entries[vmem_end_addr / 4096] = phys_addr | 3;
        phys_addr += 4096;
        vmem_end_addr += 4096;
    }
    REFRESH_CR3();
    return (void*)((size_t)phys_mem + offset);
}
void bootinfo_init(void) {
    boot_info_t *boot_info_v = vmem_map(boot_info, sizeof(boot_info_t));
    while ((size_t)boot_info_v + boot_info_v->total_size > vmem_end_addr) {
        size_t allocated_size = vmem_end_addr - (size_t)boot_info_v;
        size_t remaining_size = boot_info_v->total_size - allocated_size;
        vmem_map((void*)boot_info + allocated_size, remaining_size);
    }

    boot_info = boot_info_v;
    void *tag_address = (void*)boot_info->tags;
    size_t tag_offset = 0;
    size_t tag_size = boot_info->total_size - sizeof(boot_info_t);

    while (tag_offset < tag_size) {
        boot_tag_t *tag = (boot_tag_t*)&tag_address[tag_offset];

        if (tag->type == 0) break;
        if (tag->type == FRAMEBUFFER_INFO) {
            framebuffer_info = (framebuffer_info_t*)tag;
        } else if (tag->type == MEMORY_MAP_INFO) {

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
    printf("boot_info physical address: %p, boot_info total size: %u\n", phys_boot_info, boot_info->total_size);
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
    __asm__ volatile("movl %%ebx, %0" : "=m"(*(void**)((size_t)&boot_info - (size_t)&KERNEL_ADDR)));
    // Setup memory paging
    // Iterating kernel_start to kernel_end, enable page tables entries, and corresponding page directory entries
    size_t address = (size_t)&KERNEL_ADDR;
    size_t phys_address = 0;

    while (address < (size_t)&_kernel_end) {
        if (address % PDE_RANGE == 0) {
            *(size_t*)((void*)&page_directory_entries[address / PDE_RANGE] - (void*)&KERNEL_ADDR) =
                ((size_t)&page_tables_entries[address / 4096] - (size_t)&KERNEL_ADDR) | 3;
        }
        if (phys_address >= ((size_t)&_kernel_start & -4096)) {
            *(size_t*)((void*)&page_tables_entries[address / 4096] - (void*)&KERNEL_ADDR) = phys_address | 3;
        }
        address += 4096;
        phys_address += 4096;
    }

    *(size_t*)((void*)&vmem_end_addr - (void*)&KERNEL_ADDR) = address;

    // Identity mapping
    *(size_t*)((void*)&page_directory_entries[0] - (void*)&KERNEL_ADDR) = ((size_t)&page_tables_entries[(size_t)&KERNEL_ADDR / 4096] - (size_t)&KERNEL_ADDR) | 3;

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
