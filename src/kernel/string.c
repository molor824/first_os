#include "string.h"
#include <stddef.h>

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

void* memcpy(void *dst, const void *src, size_t n) {
    void *dst_start = dst, *dst_end = dst + n;
    while (dst < dst_end) {
        if ((size_t)dst % _Alignof(unsigned) == 0 && (size_t)src % _Alignof(unsigned) == 0) {
            goto aligned_loop;
        }
        *(char*)dst = *(char*)src;
        dst++;
        src++;
    }
    return dst_start;
aligned_loop:
    while (dst < dst_end) {
        if ((size_t)(dst_end - dst) < sizeof(unsigned)) goto end_loop;
        *(unsigned*)dst = *(unsigned*)src;
        dst += sizeof(unsigned);
        src += sizeof(unsigned);
    }
    return dst_start;
end_loop:
    while (dst < dst_end) {
        *(char*)dst = *(char*)src;
        dst++;
        src++;
    }
    return dst_start;
}
void* memset(void *s, int c, size_t n) {
    char ch = c;
    unsigned packed = ch | ch << 8 | ch << 16 | ch << 24;
    void *start = s;
    void *end = s + n;
    while (s < end) {
        if ((size_t)s % _Alignof(unsigned) == 0) goto packed_loop;
        *(char*)s = ch;
        s++;
    }
    return start;
packed_loop:
    while (s < end) {
        if ((size_t)(end - s) < sizeof(unsigned)) goto end_loop;
        *(unsigned*)s = packed;
        s += _Alignof(unsigned);
    }
    return start;
end_loop:
    while (s < end) {
        *(char*)s = ch;
        s++;
    }
    return start;
}
size_t strlen(const void* s) {
    size_t l = 0;
    while (*(const char*)(s++)) l++;
    return l;
}
int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    const unsigned char *end = p1 + n;
    while (p1 < end) {
        int diff = *p1 - *p2;
        if (diff) return diff;
        p1++;
        p2++;
    }
    return 0;
}
