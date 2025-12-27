#include <io.h>
#include <memory/gzm.h>
#include <mmu/mmu.h>
#include <mmu/mmu_phys.h>
#include <vga/vga.h>
#include <stdint.h>

#define SLAB_ID 2
#define SLAB_SIZE (12*1024) // 12kb
#define SLAB_ALIGN 8
#define SLAB_MAX 900

struct SlabRegion_t {
    void* base;
    uint32_t size;
    int used;
};

static struct GzmZone_t* slab_zone = NULL;
static struct SlabRegion_t slab_arry[SLAB_MAX]; // motivo: Eu crio e deleto muito, isso não esgota.
static uint32_t slab_alloc_count = 0;
static uint8_t* slab_offset = NULL;

void MemorySlabInit()
{
    slab_zone = GzmCreateZone(SLAB_ID,SLAB_SIZE,SLAB_ALIGN);
    slab_offset = slab_zone->memory_phy;
}
#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(MemorySlabInit,30);
void* SlabAlloc(uint32_t size) {
    if (!slab_zone) panic("Slab not init");
    for (uint32_t i = 0; i < slab_alloc_count; i++) {
        if (!slab_arry[i].used && slab_arry[i].size >= size) {
            slab_arry[i].used = 1;
            return slab_arry[i].base;
        }
    }
    if (slab_alloc_count >= SLAB_MAX) return NULL;

    uintptr_t aligned = ((uintptr_t)slab_offset + (SLAB_ALIGN-1)) & ~(SLAB_ALIGN-1);
    if (aligned + size > (uintptr_t)slab_zone->memory_phy + slab_zone->size)
        return NULL;

    // mapear páginas do aligned até aligned+size
    uint32_t start = align(aligned, PAGE_SIZE);
    uint32_t end   = align(aligned + size, PAGE_SIZE);
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE)
        MapRegion(addr, addr, PAGE_PRESENT | PAGE_WRITE);

    struct SlabRegion_t* r = &slab_arry[slab_alloc_count];
    r->base = (void*)aligned;
    r->size = size;
    r->used = 1;
    slab_offset = (uint8_t*)(aligned + size);
    slab_alloc_count++;
    return r->base;
}


void SlabFree(void* ptr)
{
    if (!slab_zone) panic("Slab not init");
    for (uint32_t i = 0; i < slab_alloc_count; i++) {
        if (slab_arry[i].base == ptr) {
            slab_arry[i].used = 0;
            return;
        }
    }
    return; // nao achou
}
