#ifndef GRUB_HEADER_H
#define GRUB_HEADER_H
#include <stdint.h>
#include "multiboot2.h"

#define GRUB_MAX_REGIONS 128
typedef enum {
    GRUB_MEM_NONE = 0, // ignore
    GRUB_MEM_USED = 1,
    GRUB_MEM_FREE = 2
} GrubRegionFlag_t;
struct GrubMemoryRegion_t {
    uint64_t base;
    uint64_t size;
    GrubRegionFlag_t flag;
    uint32_t grub_type; // original type.
};
struct GrubHeaderMemory_t {
    uint64_t total_ram;
    uint64_t free_ram;
    uint32_t region_count;
    uint32_t region_max;
    struct GrubMemoryRegion_t regions[GRUB_MAX_REGIONS];
};

int GrubParserMb2(void* mbi_addr);
struct GrubHeaderMemory_t* GrubGetMemoryHeader();
#endif