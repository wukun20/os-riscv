#include "types.h"

void *memset(void *dst, int c, uint n) {
    char *cdst;
    
    cdst = dst;
    for(int i = 0; i < n; i++) {
        cdst[i] = c;
    }

    return dst;
}

int memcmp(const void *v1, const void *v2, uint n) {
    uchar *s1, *s2;

    s1 = (uchar *)v1;
    s2 = (uchar *)v2;
    while(n-- > 0) {
        if(*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++, s2++;
    }

    return 0;
}

void *memmove(void *dst, const void *src, uint n) {
    char *d;
    const char *s;
    if(n == 0) {
        return dst;
    }
    d = dst;
    s = src;
    // 内存重叠需要从后向前复制
    if(s < d && s + n < d) {
        d += n;
        s += n;
        while(n-- > 0) {
            *--d = *--s;
        }
    } else {
        while(n-- > 0) {
            *d++ = *s++;
        }
    }
    
    return dst;
}

// 部分库中为追求性能不检查重叠
void *memcpy(void *dst, const void *src, uint n) {
  return memmove(dst, src, n);
}

int strncmp(const char *p, const char *q, uint n) {
    while(n > 0 && *p && *p == *q) {
        n--, p++, q++;
    }
    if(n == 0) {
        return 0;
    }
    return *(uchar *)p - *(uchar *)q;
}

char *strncpy(char *s, const char *t, uint n) {
    char *os;

    os = s;
    while(n-- > 0 && t) {
        *s++ = *t++;
    }
    while(n-- > 0) {
        *s++ = 0;
    }

    return os;
}

char *safestrncpy(char *s, const char *t, uint n) {
    char *os;

    os = s;
    while(n-- > 0 && t) {
        *s++ = *t++;
    }
    *s++ = 0;

    return os;
}

int strlen(const char *s) {
    int n;
    
    for(n = 0; s[n]; n++) {
    }

    return n;
}

