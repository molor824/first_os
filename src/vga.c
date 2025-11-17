#include <vga.h>

uint16_t* vga_get(size_t r, size_t c) {
	return ((uint16_t*)VGA_MEMORY) + r * VGA_WIDTH + c;
}