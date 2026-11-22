#pragma once

#include <stdint.h>

void mem_init();
void *hhdm_vaddr(void *paddr);
void *hhdm_paddr(void *vaddr);

void *phys_page_alloc();
void phys_page_free(void *page);
