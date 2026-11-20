#include "string.h"
#include <stddef.h>
#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
unsigned str_pack32(const void *str) {
    const unsigned char *p = str;
    return (unsigned)p[0] |
        (unsigned)p[1] << 8 |
        (unsigned)p[2] << 16 |
        (unsigned)p[3] << 24;
}
void *str_unpack32(void *str, unsigned packed) {
    unsigned char *p = str;
    p[0] = packed;
    p[1] = packed >> 8;
    p[2] = packed >> 16;
    p[3] = packed >> 24;
    return str;
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
unsigned str_pack32(const char *str) {
    return (unsigned)str[0] << 24 | (unsigned)str[1] << 16 | (unsigned)str[2] << 8 | (unsigned)str[3];
}
char *str_unpack32(unsigned packed, char *str) {
    str[0] = packed >> 24;
    str[1] = packed >> 16;
    str[2] = packed >> 8;
    str[3] = packed;
    return str;
}
#else
        #error "Unknown endianness"
#endif

void* memcpy(void *restrict dst, const void *restrict src, size_t n) {
    uint8_t *pdst = dst;
    const uint8_t *psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdst[i] = psrc[i];
    }

    return dst;
}
void* memmove(void *dst, const void *src, size_t n) {
    uint8_t *pdst = dst;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dst) {
        for (size_t i = 0; i < n; i++) {
            pdst[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dst) {
        for (size_t i = n; i > 0; i--) {
            pdst[i - 1] = psrc[i - 1];
        }
    }

    return dst;
}
void* memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}
size_t strlen(const void* s) {
    size_t l = 0;
    while (*(const char*)(s++)) l++;
    return l;
}
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i])
            return -1;
        else if (p1[i] > p2[i])
            return 1;
    }

    return 0;
}
