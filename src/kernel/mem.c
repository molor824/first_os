#include "mem.h"

// memmap_info_t *memmap_info;

// void *physmem_map(void *phys_mem, size_t size) {
//     size_t start_addr = (size_t)phys_mem & -4096;
//     size_t end_addr = (size_t)phys_mem + size;
//     size_t phys_addr = start_addr;
//     size_t offset = vmem_end_addr - start_addr;
//     while (phys_addr < end_addr) {
//         if (vmem_end_addr % PDE_RANGE == 0) {
//             // NOTE: PDE receives physical address of PTE, not virtual
//             page_directory_entries[vmem_end_addr / PDE_RANGE] = ((size_t)&page_tables_entries[vmem_end_addr / 4096] - (size_t)&KERNEL_ADDR) | 3;
//         }
//         page_tables_entries[vmem_end_addr / 4096] = phys_addr | 3;
//         phys_addr += 4096;
//         vmem_end_addr += 4096;
//     }
//     REFRESH_CR3();
//     return (void*)((size_t)phys_mem + offset);
// }
