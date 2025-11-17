#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "The compiler is not cross-compiler"
#endif

#if !defined(__i386__)
#error "Must be compiled with i686-elf compiler"
#endif

typedef enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
} vga_color_t;

#define VGA_COLOR_ENTRY(fg, bg) ((uint8_t)(fg) | (uint8_t)(bg) << 4)
#define VGA_ENTRY(ch, col) ((uint16_t)(ch) | (uint16_t)(col) << 8)

size_t strlen(const char* str) {
    size_t l = 0;
    while (str[l])
        l++;
    return l;
}

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

size_t vga_crow = 0, vga_ccol = 0;
uint8_t vga_color = VGA_COLOR_ENTRY(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
uint16_t* vga_buf = (uint16_t*)VGA_MEMORY;

void* memcpy(void *dst, const void *src, size_t size) {
    for (size_t i = 0; i < size; i++) {
        ((char*)dst)[i] = ((const char*)src)[i];
    }
    return dst;
}
void* memset(void *s, int c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((uint8_t *)s)[i] = (uint8_t)c;
    }
    return s;
}
void vga_init(void) {
    for (size_t r = 0; r < VGA_HEIGHT; r++) {
        for (size_t c = 0; c < VGA_WIDTH; c++) {
            size_t index = c + r * VGA_WIDTH;
            vga_buf[index] = VGA_ENTRY('\0', vga_color);
        }
    }
}
void vga_shift_row(size_t shift) {
    if (shift == 0) return;
    for (size_t i = 0; i < VGA_HEIGHT; i++) {
        size_t dst_row = i * VGA_WIDTH;
        size_t src_row = (i + shift) * VGA_WIDTH;
        if (src_row >= VGA_WIDTH * VGA_HEIGHT) {
            memset(&vga_buf[dst_row], 0, VGA_WIDTH * sizeof(uint16_t));
        } else {
            memcpy(&vga_buf[dst_row], &vga_buf[src_row], VGA_WIDTH * sizeof(uint16_t));
        }
    }
}
void vga_cursor_right(void) {
    vga_ccol++;
    if (vga_ccol >= VGA_WIDTH) {
        vga_ccol = 0;
        vga_crow++;
        if (vga_crow >= VGA_HEIGHT) {
            vga_crow = VGA_HEIGHT - 1;
            vga_shift_row(1);
        }
    }
}
void vga_cursor_down(void) {
    vga_crow++;
    if (vga_crow >= VGA_HEIGHT) {
        vga_crow = VGA_HEIGHT - 1;
        vga_shift_row(1);
    }
}
void vga_putc(char ch) {
    switch (ch) {
        case '\n':
            vga_ccol = 0;
            vga_cursor_down();
            break;
        case '\r':
            vga_ccol = 0;
            break;
        default:
            vga_buf[vga_crow * VGA_WIDTH + vga_ccol] = VGA_ENTRY(ch, vga_color);
            vga_cursor_right();
            break;
    }
}
void vga_puts(const char* str) {
    for (size_t i = 0; str[i]; i++) {
        vga_putc(str[i]);
    }
}
size_t u32toa(uint32_t a, char *dst) {
    uint32_t a1 = a;
    size_t count = 1;
    while (a1 >= 10) {
        count++;
        a1 /= 10;
    }
    if (!dst)
        return count;

    for (size_t i = count; i > 0; i--) {
        dst[i - 1] = (char)(a % 10) + '0';
        a /= 10;
    }

    return count;
}

void kernel_main(void) {
    vga_init();
    vga_puts("Hello, World!\n");
    vga_puts("Welcome to My first OS!\n");

    for (size_t i = 0; ; i++) {
        vga_putc('\r');
        char buffer[0x100];
        size_t count = u32toa(i, buffer);
        buffer[count] = '\0';
        vga_puts(buffer);
    }
}
