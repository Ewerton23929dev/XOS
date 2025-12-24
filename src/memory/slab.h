#ifndef SLAB_H
#define SLAB_H
#include <stdint.h>

void MemorySlabInit();
void* SlabAlloc(uint32_t size);
void SlabFree(void* ptr);
#endif