#ifndef __MEM_H
#define __MEM_H

#include "kernel.h"
#include <stdint.h>

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) memmap_entries_t;
typedef struct {
    boot_tag_t tag;
    uint32_t entry_size;
    uint32_t entry_version;
    memmap_entries_t entries[];
} __attribute__((packed)) memmap_info_t;

#endif
