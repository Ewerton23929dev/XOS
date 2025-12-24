#ifndef ALLOC_H
#define ALLOC_H
#include <stdint.h>

void KmallocinitBoot();
void* kmalloc(uint32_t size);
#endif