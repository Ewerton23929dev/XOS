#include <stdint.h>
#include <idt/idt.h>
#include <mmu/mmu.h>
#include <mmu/mmu_phys.h>
#include <vga/vga.h>
#include <io.h>

static inline void load_cr3(uint32_t val) {
    asm volatile("mov %0, %%cr3" :: "r"(val));
}
static inline void enable_paging() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

static inline uint32_t read_cr2() {
    uint32_t val;
    asm volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}
void disable_paging() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000;  // limpa bit PG
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void page_fault(struct isr_context_t* ctx) {
    uint32_t error = ctx->error_code;
    uint32_t addr = read_cr2();
    PanicPush("ADDR:","Erro to",addr);
    PanicPush("ERRO:","Erro code", error);
    panic("PAGE FAULT!");
}

#define ID_MAP_MB 7 // minimo 4MB e maximo 16MB
uint32_t* page_directory = NULL;
extern void isr_page_fault();
void MmuInit()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
    asm volatile("cli");
    page_directory = MmuAllocPage(); // usa GZM
    if (!page_directory) panic("MMU: Sem Memoria para PD");
    for (int i = 0; i < 1024; i++)
        page_directory[i] = 0;

    for (int pd = 0; pd < ID_MAP_MB / 4; pd++) {
        uint32_t* pt = MmuAllocPage();
        if (!pt) panic("MMU: Falha para Page Table");
        for (int i = 0; i < 1024; i++) {
            pt[i] = 0;
        }
        for (int i = 0; i < 1024; i++) {
            uint32_t phys = (pd * 0x400000) + (i * 0x1000);
            pt[i] = phys | HW_P | HW_W;
        }
        page_directory[pd] =
            ((uint32_t)pt) | HW_P | HW_W;
    }

    // NULL page inválida
    uint32_t* pt0 = (uint32_t*)(page_directory[0] & 0xFFFFF000);
    pt0[0] = 0;

    idt_set_gate(14, isr_page_fault, 0x8E);
    load_cr3((uint32_t)page_directory);
    enable_paging();
}
#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(MmuInit,40);
void MapRegion(uint32_t virt_addr,uint32_t phys_addr,PageFlags flags)
{
    uint32_t mmu_flags = 0;
    if (flags & PAGE_PRESENT) mmu_flags |= HW_P;
    if (flags & PAGE_WRITE) mmu_flags |= HW_W;
    if (flags & PAGE_USER) mmu_flags |= HW_U;
    if (flags & PAGE_WRITE_THROUGH) mmu_flags |= HW_WT;
    if (flags & PAGE_GLOBAL) mmu_flags |= HW_G;
    if (flags & PAGE_CACHE_DISABLE) mmu_flags |= HW_CD;
    uint32_t pd_index = (virt_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    if (!(page_directory[pd_index] & HW_P)) {
        uint32_t* new_pt = MmuAllocPage();
        page_directory[pd_index] = (uint32_t)new_pt | HW_P | HW_W;
    }
    uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    pt[pt_index] = (phys_addr & 0xFFFFF000) | mmu_flags;
    asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

void UnmapRegion(uint32_t virt_addr)
{
    uint32_t pd_index = (virt_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
    if (!(page_directory[pd_index] & HW_P))
        return;
    uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    
    if (pt[pt_index] & HW_P) {
        // Implementar MmuFreePage()
        pt[pt_index] = 0;
        asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory"); // invalida TLB
    }
}

#include <memory/gzm.h>
static size_t update_zones_count = 1;

void MapZones() {
    size_t zones_count = GzmGetCountZones();
    if (zones_count == update_zones_count) return;
    for (size_t i = update_zones_count; i < zones_count; i++) {
        struct GzmZone_t zone;
        if (GzmGetZone(i, &zone) != 0) continue;

        // Alinha início e fim da zona
        uint32_t start = align((uint32_t)zone.memory_phy, PAGE_SIZE);
        uint32_t end   = align((uint32_t)zone.memory_phy + zone.size, PAGE_SIZE);

        for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
            MapRegion(addr, addr, PAGE_PRESENT | PAGE_WRITE);
        }
    }
    update_zones_count = zones_count;
}