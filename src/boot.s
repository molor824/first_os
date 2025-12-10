.set MAGIC, 0xE85250D6 // Magic number, finds bootloader
.set ARCH, 0
.set WIDTH, 800
.set HEIGHT, 600

.set FRAMEBUFFER_INFO, 8

// Paging
.set PDE_INDEX, 0x300
.set PDE_COUNT, 1
.set KERNEL_ADDR, PDE_INDEX*0x400000 // Kernel virtual base address

.global KERNEL_ADDR

.section multiboot2
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

.align 8

.section .multiboot.data, "aw"

// Define stack
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 0x100000 // 1 MB stack
stack_top:

.section .bss, "aw", @nobits
// Preallocate pages used for paging.
.align 4
framebuffer_info:
framebuffer_addr:
.skip 4
framebuffer_pitch:
.skip 4
framebuffer_width:
.skip 4
framebuffer_height:
.skip 4
framebuffer_bpp:
.skip 1
framebuffer_type:
.skip 1
framebuffer_reserved:
.skip 1
framebuffer_red_field_pos:
.skip 1
framebuffer_red_mask_size:
.skip 1
framebuffer_green_field_pos:
.skip 1
framebuffer_green_mask_size:
.skip 1
framebuffer_blue_field_pos:
.skip 1
framebuffer_blue_mask_size:
.skip 1

.align 4096
page_directory:
.skip 4096
boot_page_table_1:
.skip 4096*PDE_COUNT

// Further pages may be needed if kernel is large.

// NOTE FOR SELF: "a" = allocatable, "w" = writable
// @nobits = no space in file, just in memory
// if confused between .multiboot.* section and regular section, see the comment at linker.ld
// In short: .multiboot.* - physical addressed section
//           .* - virtual addressed section

// starting point
.section .multiboot.text, "a"

.global abort
.type abort, @function
abort:
    cli
0:
    hlt
    jmp 0b

.global _start
.type _start, @function
_start:
    // Setup stack (physical addressed stack)
    // No need to pop stack off since stack is reset after paging
    mov $(stack_top - KERNEL_ADDR), %esp

    // Go through multiboot2 tags till framebuffer_info is received
    mov %ebx, %edi
    mov (%edi), %eax // Total size
    add %edi, %eax   // End pointer
    add $8, %ebx // Start iterating 

0:
    cmp %eax, %ebx // while (ebx < eax)
    jge 0f

    // read type and check
    mov (%ebx), %ecx
    // if type is 0, it's a terminating tag
    cmp $0, %ecx
    je 0f
    // if (ecx == FRAMEBUFFER_INFO)
    cmp $FRAMEBUFFER_INFO, %ecx
    jne 1f
    // Framebuffer_type must be 1, otherwise abort
    // if (cl != 1) abort
    mov 29(%ebx), %cl
    // Store framebuffer to a struct
    mov 8(%ebx), %ecx
    mov %ecx, (framebuffer_addr - KERNEL_ADDR)
    mov 16(%ebx), %ecx
    mov %ecx, (framebuffer_pitch - KERNEL_ADDR)
    mov 20(%ebx), %ecx
    mov %ecx, (framebuffer_width - KERNEL_ADDR)
    mov 24(%ebx), %ecx
    mov %ecx, (framebuffer_height - KERNEL_ADDR)
    mov 28(%ebx), %ecx
    mov %ecx, (framebuffer_bpp - KERNEL_ADDR)
    mov 30(%ebx), %ecx
    mov %ecx, (framebuffer_red_mask_size - KERNEL_ADDR)
    mov 34(%ebx), %cl
    mov %cl, (framebuffer_blue_mask_size - KERNEL_ADDR)
    1:

    // Read size and add
    mov 4(%ebx), %ecx
    add %ecx, %ebx
    // Align with 8-byte padding via this formula ebx = (ebx + 7) & ~7
    add $7, %ebx
    and $-8, %ebx // ~7 = -8
    jmp 0b
0:

.set KERNEL_PAGE_COUNT, PDE_COUNT*1024
    // Physical address of boot_page_table_1
    mov $(boot_page_table_1 - KERNEL_ADDR), %edi
    // First address to map is address 0
    mov $0, %esi

    // Looping through pages and mapping physical addresses within kernel range as present and writable
0:
    cmp $_kernel_start, %esi // if (esi < physical_kernel_start) skip to increment
    jl 1f
    cmp $(KERNEL_PAGE_COUNT*4096), %esi  // while (esi < 1023*4096)
    jge 0f
    cmp $(_kernel_end - KERNEL_ADDR), %esi // if (esi >= physical_kernel_end) end loop
    jge 0f

    // Mark physical address as present and writable
    // NOTE: this also maps .text and .rodata section as writable, mind security and mark them as non-writable
    mov %esi, %edx
    or $0x003, %edx
    mov %edx, (%edi)

1:
    // Size of page is 4096 bytes
    add $4096, %esi
    // Size of entries in boot_page_table_1 is 4 bytes
    add $4, %edi
    // Loop to the next entry
    jmp 0b
0:
    // Check if all part of kernel is mapped
    // otherwise print error to vga buffer and abort

    // if (esi < physical_kernel_end) panic and abort
    cmp $(_kernel_end - KERNEL_ADDR), %esi
    jge 0f
    lea error_kernel_mmap_fail, %eax
    call panic
0:
    // Map VGA memory buffer to 0xC03FF000 as "present" and "writable"
    // movl $(VGA_BUFFER_PHYS_ADDR | 0x003), boot_page_table_1 - KERNEL_ADDR + (PDE_COUNT*1024-1) * 4

    // The page table is used at both page directory entry 0 (virtually from 0x0
    // to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
    // 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
    // higher half). The kernel is identity mapped because enabling paging does
    // not change the next instruction, which continues to be physical. The CPU
    // would instead page fault if there was no identity mapping.

    // Map the page table to both virtual addresses 0x0 and 0xC0000000
    // NOTE: the kernel itself is in page 768 (because that's where 0xC0000000 is)
    // We need to map the physical address to page 768 along with write-protect and present bits
    // But also page 0 since that's where we are currently, and we need to jump to virtual address
    movl $(boot_page_table_1 - KERNEL_ADDR + 0x003), page_directory - KERNEL_ADDR + 0

    // for (eax = 0; eax < PDE_COUNT; eax++)
    mov $0, %eax
0:
    cmp $PDE_COUNT, %eax
    jge 0f

    // (page_directory - KERNEL_ADDR + PDE_INDEX * 4)[eax * 4] = $(boot_page_table_1 - KERNEL_ADDR + 0x003 + eax * 4096)
    mov %eax, %ebx
    imul $4096, %ebx
    add $(boot_page_table_1 - KERNEL_ADDR + 0x003), %ebx
    mov %ebx, (page_directory - KERNEL_ADDR + PDE_INDEX*4)(,%eax,4)
1:
    inc %eax
    jmp 0b
0:

    // Set cr3 to the address of the page_directory
    mov $(page_directory - KERNEL_ADDR), %ecx
    mov %ecx, %cr3

    // Enable paging and write-protect bit
    mov %cr0, %ecx
    or $0x80010000, %ecx
    mov %ecx, %cr0

    // Jump to higher half with an absolute jump
    lea _virt_start, %ecx
    jmp *%ecx

// Virtual address starts
.section .text

_virt_start:
    // At this point paging is fully set and enabled
    // Unmap the identity mapping as its now unnecessary
    movl $0, page_directory + 0

    // Reload crc3 to force a TLB flush so the changes take effect
    mov %cr3, %ecx
    mov %ecx, %cr3

    // setup stack
    mov $stack_top, %esp

    call _init
    call kernel_main
    call _fini

    // abort
    cli
0:
    hlt
    jmp 0b
