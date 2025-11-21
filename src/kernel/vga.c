#include "vga.h"

void vga_set(size_t r, size_t c, uint16_t entry) {
	((uint16_t*)VGA_MEMORY)[r * VGA_WIDTH + c] = entry;
}
