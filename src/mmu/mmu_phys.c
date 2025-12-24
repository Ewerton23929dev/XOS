#include <stdint.h>
#include <io.h>
#include <memory/gzm.h>
#include <mmu/mmu_phys.h>

#define MMU_ZONE_ID 0

static uint32_t offset = 0;
static struct GzmZone_t* mmu_zone = NULL;

// identity map
void MmuPhysInit(void)
{
    if (mmu_zone) return;
    uint32_t kernel_zone_size = 64 * 1024;
    mmu_zone = GzmCreateZone(MMU_ZONE_ID,kernel_zone_size,PAGE_SIZE);
    if (mmu_zone) {
        mmu_zone->flags |= GZM_ZONE_FIXED;
    }
}

void* MmuAllocPage(void)
{
    if (!mmu_zone) return NULL;
    if (offset + PAGE_SIZE > mmu_zone->size) return NULL;
    offset = align(offset,PAGE_SIZE);
    void* page = (uint8_t*)mmu_zone->memory_phy + offset;
    offset += PAGE_SIZE;
    return page;
}
