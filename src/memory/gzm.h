#ifndef GZM_H
#define GZM_H
#include <stdint.h>

#define GZM_DEFAULT (0 << 0)
#define GZM_ZONE_FIXED (1 << 0)
struct GzmZone_t {
    int id;
    uint32_t align;
    void* memory_phy;
    uint32_t size;
    uint8_t valid; // 0 ou 1
    uint8_t flags;
};

void GzmInit(void);
struct GzmZone_t* GzmCreateZone(int id, uint32_t size, uint32_t align_v);
int GzmGetZone(int id, struct GzmZone_t* out);
uint32_t GzmGetCountZones();
#endif