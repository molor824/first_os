#include "itoa.h"

const char hexchars[] = "0123456789abcdef";

int ulltoa(unsigned long long a, char *dst, unsigned base) {
    if (base < 2 || base > 16)
        return -1;

    unsigned long long a1 = a;
    int count = 1;
    while (a1 >= base) {
        count++;
        a1 /= base;
    }
    if (!dst)
        return count;

    for (int i = count; i > 0; i--) {
        dst[i - 1] = hexchars[a % base];
        a /= base;
    }

    return count;
}
int ultoa(unsigned long a, char *dst, unsigned base) {
    return ulltoa((unsigned long long)a, dst, base);
}
int utoa(unsigned a, char *dst, unsigned base) {
    return ulltoa((unsigned long long)a, dst, base);
}
int lltoa(long long a, char *dst, unsigned base) {
    if (base < 2 || base > 16)
        return -1;

    if (a < 0) {
        if (dst) {
            dst[0] = '-';
            return ulltoa((unsigned long long)(-a), &dst[1], base) + 1;
        } else {
            return ulltoa((unsigned long long)(-a), dst, base) + 1;
        }
    }
    return ulltoa((unsigned long long)a, dst, base);
}
int ltoa(long a, char *dst, unsigned base) {
    return lltoa((long long)a, dst, base);
}
int itoa(int a, char *dst, unsigned base) {
    return lltoa((long long)a, dst, base);
}