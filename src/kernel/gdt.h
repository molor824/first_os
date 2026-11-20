#pragma once

#include <stdint.h>
#include <stddef.h>

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

typedef struct {
    uint32_t _reserved0;
    uint64_t rsp[3];
    uint64_t _reserved1;
    uint64_t ist[7];
    uint64_t _reserved2;
    uint16_t _reserved3;
    uint16_t iopb;
} __attribute__((packed)) tss_t;

extern uint64_t gdt_entries[7];

void gdt_init(void);
uint64_t gdt_entry(gdt_params_t params);
void set_gdt(uint16_t limit, uint64_t base);
void reload_segments();

// Granularity flag, indicates the size the Limit value is scaled by. If clear (0), the Limit is in 1 Byte blocks (byte granularity). If set (1), the Limit is in 4 KiB blocks (page granularity).
#define FLAG_G (1 << 3)
// Size flag. If clear (0), the descriptor defines a 16-bit protected mode segment. If set (1) it defines a 32-bit protected mode segment. A GDT can have both 16-bit and 32-bit selectors at once.
#define FLAG_DB (1 << 2)
// Long-mode code flag. If set (1), the descriptor defines a 64-bit code segment. When set, DB should always be clear. For any other type of segment (other code types or any data segment), it should be clear (0).
#define FLAG_L (1 << 1)

// Present bit. Allows an entry to refer to a valid segment. Must be set (1) for any valid segment.
#define ACCESS_P (1 << 7)
// Descriptor privilege level field. Contains the CPU Privilege level of the segment. 0 = highest privilege (kernel), 3 = lowest privilege (user applications).
#define ACCESS_PRIV0 0
#define ACCESS_PRIV1 (1 << 5)
#define ACCESS_PRIV2 (1 << 6)
#define ACCESS_PRIV3 (ACCESS_PRIV1 | ACCESS_PRIV2)
// Descriptor type bit. If clear (0) the descriptor defines a system segment (eg. a Task State Segment). If set (1) it defines a code or data segment.
#define ACCESS_S (1 << 4)
// Executable bit. If clear (0) the descriptor defines a data segment. If set (1) it defines a code segment which can be executed from.
#define ACCESS_E (1 << 3)
// Direction bit/Conforming bit
// For data selectors: Direction bit. If clear (0) the segment grows up. If set (1) the segment grows down, ie. the Offset has to be greater than the Limit.
// For code selectors: Conforming bit.
// - If clear (0) code in this segment can only be executed from the ring set in DPL.
// - If set (1) code in this segment can be executed from an equal or lower privilege level. For example, code in ring 3 can far-jump to conforming code in a ring 2 segment. The DPL field represent the highest privilege level that is allowed to execute the segment. For example, code in ring 0 cannot far-jump to a conforming code segment where DPL is 2, while code in ring 2 and 3 can. Note that the privilege level remains the same, ie. a far-jump from ring 3 to a segment with a DPL of 2 remains in ring 3 after the jump.
#define ACCESS_DC (1 << 2)
// Readable bit/Writable bit
// For code segments: Readable bit. If clear (0), read access for this segment is not allowed. If set (1) read access is allowed. Write access is never allowed for code segments.
// For data segments: Writeable bit. If clear (0), write access for this segment is not allowed. If set (1) write access is allowed. Read access is always allowed for data segments.
#define ACCESS_RW (1 << 1)
// Accessed bit. The CPU will set it when the segment is accessed unless set to 1 in advance. This means that in case the GDT descriptor is stored in read only pages and this bit is set to 0, the CPU trying to set this bit will trigger a page fault. Best left set to 1 unless otherwise needed.
#define ACCESS_A (1)
