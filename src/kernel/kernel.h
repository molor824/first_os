#pragma once

#include "limine.h"
#include <stdint.h>
#include <stddef.h>
#include <stdnoreturn.h>

#define REFRESH_CR3() asm volatile( \
    "mov %%cr3, %%eax\n" \
    "mov %%eax, %%cr3" \
    : : : "eax" "memory" \
)

noreturn void hcf();
