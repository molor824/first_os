.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO // Multiboot flag field
.set MAGIC, 0x1BADB002 // Magic number, finds bootloader
.set CHECKSUM, -(MAGIC + FLAGS) // Checksum
.set KERNEL_ADDR, 0xC0000000 // Kernel virtual base address
.set VGA_BUFFER_PHYS_ADDR, 0x000B8000
.set VGA_BUFFER_ADDR, 0xC03FF000

// Declare multiboot header that marks the program as kernel
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

// Define stack
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 0x100000 // 1 MB stack
stack_top:

// Preallocate pages used for paging.
.section .bss, "aw", @nobits
.align 4096
boot_page_directory:
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
.global _start
.type _start, @function
_start:
    // Physical address of boot_page_table_1
    movl $(boot_page_table_1 - KERNEL_ADDR), %edi
    // First address to map is address 0
    movl $0, %esi
    // map 1023 pages, 1024th will be VGA text buffer
    movl $1023, %ecx

    // Looping through pages and mapping physical addresses within kernel range as present and writable
1:
    cmpl $_kernel_start, %esi
    jl 2f
    cmpl $(_kernel_end - KERNEL_ADDR), %esi
    jge 3f
    // Mark physical address as present and writable
    // NOTE: this also maps .text and .rodata section as writable, mind security and mark them as non-writable
    movl %esi, %edx
    orl $0x003, %edx
    movl %edx, (%edi)

2:
    // Size of page is 4096 bytes
    addl $4096, %esi
    // Size of entries in boot_page_table_1 is 4 bytes
    addl $4, %edi
    // Loop to the next entry
    loop 1b

3:
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
    movl $(boot_page_table_1 - KERNEL_ADDR + 0x003), boot_page_directory - KERNEL_ADDR + 0
    movl $(boot_page_table_1 - KERNEL_ADDR + 0x003), boot_page_directory - KERNEL_ADDR + 768 * 4

    // Set cr3 to the address of the boot_page_directory
    movl $(boot_page_directory - KERNEL_ADDR), %ecx
    movl %ecx, %cr3

    // Enable paging and write-protect bit
    movl %cr0, %ecx
    orl $0x80010000, %ecx
    movl %ecx, %cr0

    // Jump to higher half with an absolute jump
    lea 4f, %ecx
    jmp *%ecx

// Virtual address starts
.section .text

4:
    // At this point paging is fully set and enabled
    // Unmap the identity mapping as its now unnecessary
    movl $0, boot_page_directory + 0

    // Reload crc3 to force a TLB flush so the changes take effect
    movl %cr3, %ecx
    movl %ecx, %cr3

    // setup stack
    movl $stack_top, %esp

    call _init
    call kernel_main
    call _fini

    // put in halt
    cli
.halt:
    hlt
    jmp .halt
