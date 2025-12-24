#include <stdint.h>
#include <io.h>

void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
uint32_t align_up(uint32_t x)
{
    return (x + 0xFFF) & 0xFFFFF000;
}
void outl(uint16_t port, uint32_t val) 
{
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}
void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}
uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t align(uint32_t addr, uint32_t align)
{
    if (align == 0) return addr;
    return (addr + align - 1) & ~(align - 1);
}
void* memcpy(void* dst, const void* src, size_t n)
{
    uintptr_t d = (uintptr_t)dst;
    uintptr_t s = (uintptr_t)src;
    while ((d & 3) && n) {
        *(uint8_t*)d++ = *(uint8_t*)s++;
        n--;
    }
    while (n >= 4) {
        *(uint32_t*)d = *(uint32_t*)s;
        d += 4;
        s += 4;
        n -= 4;
    }
    while (n--)
        *(uint8_t*)d++ = *(uint8_t*)s++;
    return dst;
}
void *memset(void *ptr, int c, size_t n)
{
    uint8_t *p = (uint8_t*) ptr;
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return ptr;
}
static char *strtok_save = NULL;
char *strtok(char *str, const char *delim)
{
    if (str) strtok_save = str;
    if (!strtok_save) return NULL;
    char *start = strtok_save;
    while (*start) {
        int is_delim = 0;
        for (const char *d = delim; *d; d++) {
            if (*start == *d) { is_delim = 1; break; }
        }
        if (!is_delim) break;
        start++;
    }
    if (*start == '\0') {
        strtok_save = NULL;
        return NULL;
    }
    char *end = start;
    while (*end) {
        int is_delim = 0;
        for (const char *d = delim; *d; d++) {
            if (*end == *d) { is_delim = 1; break; }
        }
        if (is_delim) break;
        end++;
    }
    if (*end) {
        *end = '\0';
        strtok_save = end + 1;
    } else {
        strtok_save = NULL;
    }

    return start;
}
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        if (*s1 != *s2) return (int)(uint8_t)(*s1) - (int)(uint8_t)(*s2);
        s1++;
        s2++;
    }
    return (int)(uint8_t)(*s1) - (int)(uint8_t)(*s2);
}
size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
int strncmp(const char* a, const char* b, int max)
{
    for (int i = 0; i < max; i++) {
        unsigned char ca = a[i];
        unsigned char cb = b[i];

        if (ca != cb)
            return ca - cb;

        if (ca == '\0')
            return 0;
    }
    return 0;
}