#include <stdint.h>
#include <memory/gzm.h>
#include <vga/vga.h>
#include <io.h>

#define HEAP_ID 1
#define HEAP_SIZE (1*1024*1024)
#define HEAP_ALIGN 16

static uint8_t* heap_base = NULL;
static uint8_t* heap_brk = NULL;
static uint8_t* heap_limit = NULL;

void KmallocinitBoot()
{
    struct GzmZone_t* z = GzmCreateZone(HEAP_ID,HEAP_SIZE,HEAP_ALIGN);
    if (!z) return;
    z->flags |= GZM_DEFAULT;
    heap_base = (uint8_t*)z->memory_phy;
    heap_brk = heap_base;
    heap_limit = heap_base + z->size;
}
#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(KmallocinitBoot,10);

void* kmalloc(uint32_t size)
{
    if (!heap_base) panic("kmalloc usado antes de init");
    size = align(size,HEAP_ALIGN);
    if (heap_brk + size > heap_limit) return NULL;
    void* ptr = heap_brk;
    heap_brk += size;
    return ptr;
}