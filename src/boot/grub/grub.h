#ifndef GRUB_HEADER_H
#define GRUB_HEADER_H
#include <stdint.h>
#include "multiboot1.h"

struct memory_region {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
};

#define MAX_REGIONS 400

struct memory_region* GrubGetMemoryRegions(uint32_t magic, void* info_addr, uint32_t* out_count);
#endif