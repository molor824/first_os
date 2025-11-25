#ifndef __ITOA_H
#define __ITOA_H

#define ULLTOA_MAX_SIZE (sizeof(unsigned long long) * 8 + 1)
#define ULTOA_MAX_SIZE (sizeof(unsigned long) * 8 + 1)
#define UTOA_MAX_SIZE (sizeof(unsigned) * 8 + 1)
#define LLTOA_MAX_SIZE (sizeof(long long) * 8 + 2)
#define LTOA_MAX_SIZE (sizeof(long) * 8 + 2)
#define ITOA_MAX_SIZE (sizeof(int) * 8 + 2)

int ulltoa(unsigned long long a, char *dst, unsigned radix);
int ultoa(unsigned long a, char *dst, unsigned radix);
int utoa(unsigned a, char *dst, unsigned radix);

int lltoa(long long a, char *dst, unsigned radix);
int ltoa(long a, char *dst, unsigned radix);
int itoa(int a, char *dst, unsigned radix);

#endif