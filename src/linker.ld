ENTRY(_start)

/* Explain for future self:
.multiboot.* sections are in physical address!
however the rest are in virtual address, hence `. += 0xC0000000`
NOTE: this offset does NOT mean that the section will be stored accordingly in hardware level
it only makes labels in the assembly within that section behave as if they were stored that way
CPU will access the addresses with the offset 0xC0000000, which is the whole point of implementing paging,
to move the kernel to high half address without affecting RAM
so anything within .multiboot.* section is in physical address
while everything else is in virtual address!!!
*/

SECTIONS
{
	. = 0x00100000;
	/* The kernel will live at 3GB + 1MB in the virtual address space
	which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections */

	_kernel_start = .;
	.multiboot.data : {
		*(.multiboot.data)
	}

	.multiboot.text : {
		*(.multiboot.text)
	}

	. += 0xC0000000;
	/* Virtual address */
	.text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	/* symbol that indicates the end address of kernel */
	_kernel_end = .;
}
