#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include "stdio.h"
#include "framebuffer.h"
#include "limine.h"
#include "kernel.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

noreturn void hcf() {
    for (;;) {
        asm("hlt");
    }
}

void kernel_main() {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    stdio_init();
    fb_init();

    printf("Hello, world!\n");
    fflush(STDOUT);

    for (uint8_t i = 0;; i++) {
        printf("%u\n", i);
        fflush(STDOUT);
    }

    hcf();
}
