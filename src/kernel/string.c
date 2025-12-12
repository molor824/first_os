#include "string.h"

void* memcpy(void *dst, const void *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
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
int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}