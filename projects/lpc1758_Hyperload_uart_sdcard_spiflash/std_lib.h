#ifndef STD_LIB_H_
#define STD_LIB_H_

void memcpy (void* dst, const void* src, int cnt)
{
    char *d = (char*)dst;
    const char *s = (const char*)src;

    while (cnt--)
        *d++ = *s++;
}

void memset (void* dst, int val, int cnt) {
    char *d = (char*)dst;

    while (cnt--)
        *d++ = (char)val;
}

#endif /* STD_LIB_H_ */
