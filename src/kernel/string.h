#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

void* memcpy(void *dst, const void *src, size_t n);
void* memset(void *s, int c, size_t n);
size_t strlen(const void *s);
int memcmp(const void *s1, const void *s2, size_t n);
unsigned str_pack32(const void *str);
void *str_unpack32(void *str, unsigned p);

#endif
