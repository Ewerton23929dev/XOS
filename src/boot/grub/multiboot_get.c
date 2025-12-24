#include <boot/grub/multiboot1.h>
#include <vga/vga.h>
#include "grub.h"
#include <stdint.h>
#include <io.h>

static struct memory_region regions[MAX_REGIONS];
#define MULTIBOOT1_BOOTLOADER_MAGIC 0x2BADB002

struct memory_region* GrubGetMemoryRegions(uint32_t magic, void* info_addr, uint32_t* out_count)
{

    struct multiboot_info* mbi = (struct multiboot_info*)info_addr;

    uint32_t regions_count = 0;

    if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        // Itera sobre MMAP do GRUB
        uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;
        struct multiboot_mmap_entry* entry =
            (struct multiboot_mmap_entry*)mbi->mmap_addr;

        while ((uintptr_t)entry < mmap_end && regions_count < MAX_REGIONS) {
            regions[regions_count].addr = entry->addr;
            regions[regions_count].len  = entry->len;
            regions[regions_count].type = entry->type;
            regions_count++;

            entry = (struct multiboot_mmap_entry*)((char*)entry + entry->size + sizeof(entry->size));
        }
    } else {
        // Fallback se MMAP não existe
        regions[0].addr = 0x100000; // depois do BIOS/bootloader
        regions[0].len  = mbi->mem_upper * 1024;
        regions[0].type = 1; // RAM utilizável
        regions_count = 1;
    }

    if (out_count)
        *out_count = regions_count;

    return regions;
}