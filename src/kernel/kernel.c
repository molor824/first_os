#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "iobuf.h"
#include "file.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "kernel.h"

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

uint32_t page_directory_entries[1024];
uint32_t page_tables_entries[1024 * 1024];
size_t kernel_end_addr = (size_t)&KERNEL_ADDR;
boot_info_t *boot_info;

void *mmap(void *mem, size_t size) {
    size_t start_addr = (size_t)mem & -4096;
    size_t end_addr = (size_t)mem + size;
    size_t phys_addr = start_addr;
    size_t offset = kernel_end_addr - start_addr;
    while (phys_addr < end_addr) {
        if (kernel_end_addr % PDE_RANGE == 0) {
            page_directory_entries[kernel_end_addr / PDE_RANGE] = (size_t)&page_tables_entries[kernel_end_addr / 4096] | 3;
        }
        page_tables_entries[kernel_end_addr / 4096] = phys_addr | 3;
        phys_addr += 4096;
        kernel_end_addr += 4096;
    }
    REFRESH_CR3();
    return (void*)((size_t)mem + offset);
}
void bootinfo_init(void) {
    boot_info_t *boot_info_v = mmap(boot_info, sizeof(boot_info_t));
    while ((size_t)boot_info_v + boot_info_v->total_size > kernel_end_addr) {
        size_t allocated_size = kernel_end_addr - (size_t)boot_info_v;
        size_t remaining_size = boot_info_v->total_size - allocated_size;
        mmap((void*)boot_info + allocated_size, remaining_size);
    }

    boot_info = boot_info_v;
    char *tag_address = (char*)boot_info->tags;
    size_t tag_offset = 0;
    size_t tag_size = boot_info->total_size - sizeof(boot_info_t);

    while (tag_offset < tag_size) {
        boot_tag_t *tag = (boot_tag_t*)&tag_address[tag_offset];

        if (tag->type == 0) break;
        if (tag->type == FRAMEBUFFER_INFO) {
            framebuffer_info = (framebuffer_info_t*)tag->data;
        }

        tag_offset = (tag_offset + tag->size + 7) & -8;
    }
}
void main(void) {
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
    printf("Total kernel size in memory: %u bytes\n", (size_t)&_kernel_end - (size_t)&KERNEL_ADDR - (size_t)&_kernel_start);
    printf("Hello, World!\n");
    printf("Welcome to My first OS!\n");

    for (;;) {
        __asm__ volatile("hlt");
    }
    
    _fini();
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

    *(size_t*)((void*)&kernel_end_addr - (void*)&KERNEL_ADDR) = address;
    
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
        "mov $main, %ecx\n"
        "jmp *%ecx"
    );

    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
