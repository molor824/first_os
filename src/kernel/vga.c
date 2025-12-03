#include "vga.h"

void vga_set(size_t r, size_t c, uint16_t entry) {
	(&VGA_BUFFER_ADDR)[r * VGA_WIDTH + c] = entry;
}
