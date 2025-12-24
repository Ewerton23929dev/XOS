#ifndef MMU_H
#define MMU_H
#include <stdint.h>

typedef enum {
    PAGE_PRESENT        = 1 << 0,
    PAGE_WRITE          = 1 << 1,
    PAGE_USER           = 1 << 2,
    PAGE_WRITE_THROUGH  = 1 << 3,
    PAGE_CACHE_DISABLE  = 1 << 4,
    PAGE_GLOBAL         = 1 << 8,
} PageFlags;

#define HW_P  0x001
#define HW_W  0x002
#define HW_U  0x004
#define HW_WT 0x008
#define HW_CD 0x010
#define HW_G  0x100
#define PAGE_KERNEL_RO (PAGE_PRESENT)
#define PAGE_KERNEL_RW (PAGE_PRESENT | PAGE_WRITE)
#define PAGE_USER_RW   (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)

void MmuInit();
void disable_paging();
void MapRegion(uint32_t virt_addr,uint32_t phys_addr,PageFlags flags);
void UnmapRegion(uint32_t virt_addr);
void MapZones();
#endif