#include "string.h"

void* memcpy(void *dst, const void *src, size_t n) {
    size_t word_size = n / sizeof(size_t);
    for (size_t i = 0; i < word_size; i++) {
        ((size_t*)dst)[i] = ((const size_t*)src)[i];
    }
    size_t start = n - n % sizeof(size_t);
    for (size_t i = start; i < n; i++) {
        ((char*)dst)[i] = ((const char*)src)[i];
    }
    return dst;
}
void* memset(void *s, int c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((char*)s)[i] = (char)c;
    }
    return s;
}
size_t strlen(const char* s) {
    size_t l = 0;
    while (s[l])
        l++;
    return l;
}