.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO // Multiboot flag field
.set MAGIC, 0x1BADB002 // Magic number, finds bootloader
.set CHECKSUM, -(MAGIC + FLAGS) // Checksum
.set KERNEL_ADDR, 0xC0000000 // Kernel virtual base address
.set VGA_BUFFER_PHYS_ADDR, 0x000B8000
.set VGA_BUFFER_ADDR, 0xC03FF000
.set VGA_WIDTH, 80
.set VGA_HEIGHT, 25
.set ERROR_COLOR, 4<<4

// Declare multiboot header that marks the program as kernel
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

error_kernel_mmap_fail:
.string "ERROR: KERNEL MMAP FAIL, REMAINING KERNEL LACKS PAGES TO BE LOADED ONTO"

// Define stack
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 0x100000 // 1 MB stack
stack_top:

.section .bss, "aw", @nobits
// Preallocate pages used for paging.
.align 4096
page_directory:
.skip 4096
boot_page_table_1:
.skip 4096

// Further pages may be needed if kernel is large.

// NOTE FOR SELF: "a" = allocatable, "w" = writable
// @nobits = no space in file, just in memory
// if confused between .multiboot.* section and regular section, see the comment at linker.ld
// In short: .multiboot.* - physical addressed section
//           .* - virtual addressed section

// starting point
.section .multiboot.text, "a"

// (string str: %eax)
panic:
    mov $0, %edi // vga row
    mov $0, %esi // vga col

    // while (*str != 0)
.panic_loop_start:
    cmpb $0, (%eax)
    je abort

    // if (*str == '\n') skip to newline
    cmpb $0xA, (%eax)
    je .panic_newline

    // VGA_BUFFER[row*WIDTH+col]=(*str)|(color<<8)
    mov %edi, %ebx
    imul $VGA_WIDTH, %ebx
    add %esi, %ebx

    xor %cx, %cx
    movb (%eax), %cl
    or $(ERROR_COLOR<<8), %cx
    mov %cx, VGA_BUFFER_PHYS_ADDR(,%ebx,2)

    // col++
    inc %esi
    // if (col >= VGA_WIDTH) col = 0; row++;
    cmp $VGA_WIDTH, %esi
    jl .panic_skip_newline

.panic_newline:
    mov $0, %esi
    inc %edi

.panic_skip_newline:
    // str++;
    inc %eax
    jmp .panic_loop_start

.global abort
.type abort, @function
abort:
    cli
.halt:
    hlt
    jmp .halt

.global _start
.type _start, @function
_start:
    // Setup stack (physical addressed stack)
    mov $(stack_top - KERNEL_ADDR), %esp
    // Physical address of boot_page_table_1
    mov $(boot_page_table_1 - KERNEL_ADDR), %edi
    // First address to map is address 0
    mov $0, %esi
    // NOTE: Kernel should be mapped on first 1023 pages, as the last 1024th page is reserved for VGA buffer!
    mov $1023, %ecx

    // Looping through pages and mapping physical addresses within kernel range as present and writable
.while_start:
    cmp $0, %ecx  // if (ecx <= 0) end loop early
    jle .while_end
    cmp $_kernel_start, %esi // if (esi < physical_kernel_start) skip to increment
    jl .while_inc
    cmp $(_kernel_end - KERNEL_ADDR), %esi // if (esi >= physical_kernel_end) end loop
    jge .while_end

    // Mark physical address as present and writable
    // NOTE: this also maps .text and .rodata section as writable, mind security and mark them as non-writable
    mov %esi, %edx
    or $0x003, %edx
    mov %edx, (%edi)

.while_inc:
    // Size of page is 4096 bytes
    add $4096, %esi
    // Size of entries in boot_page_table_1 is 4 bytes
    add $4, %edi
    // Decrement ecx
    dec %ecx
    // Loop to the next entry
    jmp .while_start

.while_end:
    // Check if all part of kernel is mapped
    // otherwise print error to vga buffer and abort

    // if (esi < physical_kernel_end) panic and abort
    cmp $(_kernel_end - KERNEL_ADDR), %esi
    jge .skip_kernel_check
    lea error_kernel_mmap_fail, %eax
    call panic
.skip_kernel_check:
    // Map VGA memory buffer to 0xC03FF000 as "present" and "writable"
    movl $(VGA_BUFFER_PHYS_ADDR | 0x003), boot_page_table_1 - KERNEL_ADDR + 1023 * 4

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
    movl $(boot_page_table_1 - KERNEL_ADDR + 0x003), page_directory - KERNEL_ADDR + 768 * 4

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
