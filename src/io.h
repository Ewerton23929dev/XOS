#ifndef IO_H
#define IO_H
#include <stdint.h>

#define NULL (void*)0x0
typedef unsigned int size_t;

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t align_up(uint32_t x);
uint32_t align(uint32_t addr, uint32_t align);
void* memcpy(void* dst, const void* src, size_t n);
char *strtok(char *str, const char *delim);
void *memset(void *ptr, int c, size_t n);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char* str);
int strncmp(const char* a, const char* b, int max);
#endif