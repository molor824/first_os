#ifndef __GDT_H
#define __GDT_H

#include <stdint.h>
#include <stddef.h>
#include "file.h"

#define GDT_CRT_PRIORITY FILE_CRT_PRIORITY - 1
#define KERNEL_CODE_SEGMENT 1
#define KERNEL_DATA_SEGMENT 2
#define USER_CODE_SEGMENT 3
#define USER_DATA_SEGMENT 4
#define TASK_STATE_SEGMENT 5

typedef struct {
    uint32_t base;
    uint32_t limit;
    uint8_t access_byte;
    uint8_t flags;
} gdt_params_t;

extern uint64_t gdt_entries[6];

#endif