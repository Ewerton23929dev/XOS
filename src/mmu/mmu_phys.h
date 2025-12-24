#ifndef MMU_PHYS_H
#define MMU_PHYS_H
#include <stdint.h>

#define PAGE_SIZE 0x1000

void* MmuAllocPage(void);
void MmuPhysInit(void);
#endif