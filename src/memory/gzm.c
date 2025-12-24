/*
*  GZM(Gerenciador Zonas Mapeadas)
*   Um Mapeador de memoria fisica em early boot
*   Bassicamente um organizador de memoria fisica para outras camadas,
*   como HEAP, SLAB e outros.
*   Ele apenas decide onde uma zona de memoria, existe fisicamente na memoria.
*
*       ISSO NAO E UM ALLOCADOR!! cuidado, aqui tem um dragão!
*
* SpinLock sao pelo fato de nao ser runtime safe por arquitetura.
*/

#include <stdint.h>
#include <memory/gzm.h>
#include <locks/spinlock.h>
#include <io.h>
#include <vga/vga.h>

extern uint32_t kernel_end;
extern uint32_t kernel_start;
extern uint8_t gzm_start[];
extern uint8_t gzm_end[];

struct GzmHeader_t {
    uint32_t zone_count;
    uint32_t max_zones;
};

#define KERNEL_MAX_MEMORY (128*1024*1024) // 128MB, limite não real
#define GZM_FAST_IDS_MAX 1024
#define MAX_ZONE 900 // apenas por existir em early boot.
#define GZM_BASE ((uint8_t*)&gzm_start)
static struct GzmHeader_t* gzm_header = NULL;
static struct GzmZone_t* zone_arry = NULL;
static SpinLock_t gzm_lock = SPINLOCK_INIT;
static uint8_t gzm_flag_init = 0;
static uint8_t* gzm_last_offset = NULL; // aponta para o fim da última zona
static struct GzmZone_t* gzm_fast_table[GZM_FAST_IDS_MAX];

static struct GzmZone_t* __NoLockGzmGetZone(int id)
{
    if (!gzm_flag_init) return NULL;
    if (id >= 0 && id < GZM_FAST_IDS_MAX) {
        return gzm_fast_table[id];
    }
    for (int i = 0; i < gzm_header->max_zones; i++) {
        struct GzmZone_t* select_zone = &zone_arry[i];
        if (!select_zone->valid) continue;
        if (select_zone->id != id) continue;
        return select_zone;
    }
    return NULL;
}
uint32_t GzmGetCountZones()
{
    return gzm_header->zone_count;
}
void GzmInit() 
{
    gzm_flag_init = 1;
    gzm_header = (struct GzmHeader_t*)GZM_BASE;
    zone_arry = (struct GzmZone_t*)(GZM_BASE + sizeof(struct GzmHeader_t));

    gzm_header->zone_count = 0;
    gzm_header->max_zones = MAX_ZONE;
    for (int i = 0; i < gzm_header->max_zones; i++) {
        zone_arry[i].valid = 0;
    }
    gzm_last_offset = (uint8_t*)align((uintptr_t)&gzm_end, 8);
}

struct GzmZone_t* GzmCreateZone(int id, uint32_t size, uint32_t align_v) {
    SpinLock(&gzm_lock);
    if (!gzm_flag_init) goto fail;
    struct GzmZone_t* check_exist = __NoLockGzmGetZone(id);
    if (check_exist) {
        return check_exist;
    }
    struct GzmZone_t* free_zone = NULL;
    for(int i = 0; i < gzm_header->max_zones; i++) {
        if(!zone_arry[i].valid) {
            free_zone = &zone_arry[i];
            break;
        }
    }
    if(!free_zone) {
        SpinUnlock(&gzm_lock);
        return NULL;
    }
    if (!free_zone) goto fail;
    uint8_t* prev_offset = (uint8_t*)align((uintptr_t)gzm_last_offset, align_v);
    if ((uintptr_t)prev_offset + size > (uintptr_t)(kernel_start + KERNEL_MAX_MEMORY)) goto fail;
    // inicializa zona
    free_zone->align = align_v;
    free_zone->id = id;
    free_zone->flags = 0;
    free_zone->size = size;
    prev_offset = (uint8_t*)align((uintptr_t)prev_offset,align_v);
    free_zone->memory_phy = prev_offset;
    free_zone->valid = 1;
    gzm_last_offset = prev_offset + size;
    if (id >= 0 && id < GZM_FAST_IDS_MAX) {
        gzm_fast_table[id] = free_zone;
    }
    gzm_header->zone_count++;
    SpinUnlock(&gzm_lock);
    return free_zone;
    fail:
    SpinUnlock(&gzm_lock);
    return NULL;
}

int GzmGetZone(int id, struct GzmZone_t* out)
{
    if (!gzm_flag_init) return -1;
    SpinLock(&gzm_lock);
    if (id >= 0 && id < GZM_FAST_IDS_MAX) {
        if (gzm_fast_table[id]->valid != 1) {
            SpinUnlock(&gzm_lock);
            return -1;
        }
        *out = *gzm_fast_table[id];
    }
    for (int i = 0; i < gzm_header->max_zones; i++) {
        struct GzmZone_t* select_zone = &zone_arry[i];
        if (!select_zone->valid) continue;
        if (select_zone->id != id) continue;
        *out = *select_zone;
        SpinUnlock(&gzm_lock);
        return 0;
    }
    SpinUnlock(&gzm_lock);
    return -1;
}