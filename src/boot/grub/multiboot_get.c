#include <boot/grub/multiboot2.h>
#include <vga/vga.h>
#include <boot/grub/grub.h>
#include <stdint.h>
#include <io.h>

static struct GrubHeaderMemory_t grub_memory_header = {0};
static int parser_run = 0; // flag para indicar ser fez algum parser

void GrubParserMmap(struct multiboot_tag_mmap* mmap_tag)
{
    grub_memory_header.region_count = 0;
    grub_memory_header.total_ram = 0;
    grub_memory_header.free_ram = 0;
    grub_memory_header.region_max = GRUB_MAX_REGIONS; // inicia

    uint8_t* entry_ptr = (uint8_t*)mmap_tag + sizeof(struct multiboot_tag_mmap);
    uint8_t* end = (uint8_t*)mmap_tag + mmap_tag->size;

    while (entry_ptr < end) {
        if (grub_memory_header.region_count >= GRUB_MAX_REGIONS) {
            break;
        }
        struct GrubMemoryRegion_t* region_new = &grub_memory_header.regions[grub_memory_header.region_count];
        struct multiboot_mmap_entry* e = (struct multiboot_mmap_entry*)entry_ptr;
        entry_ptr += mmap_tag->entry_size;
        region_new->base = e->addr;
        region_new->size = e->len;
        region_new->grub_type = e->type;
        grub_memory_header.total_ram += e->len;
        if (e->type == MULTIBOOT_MEMORY_AVAILABLE) {
            region_new->flag = GRUB_MEM_FREE;
            grub_memory_header.free_ram += e->len;
        } else {
            region_new->flag = GRUB_MEM_USED;
        }
        grub_memory_header.region_count++;
    }
}
struct GrubHeaderMemory_t* GrubGetMemoryHeader()
{
    if (!parser_run) return NULL;
    return &grub_memory_header;
}

int GrubParserMb2(void* mbi_addr)
{
    uint8_t* ptr = (uint8_t*)mbi_addr;
    uint32_t total_size = *(uint32_t*)ptr;
    ptr += 8;
    while (ptr < (uint8_t*)mbi_addr + total_size) {
        struct multiboot_tag* tag = (struct multiboot_tag*)ptr;
        if (tag->type == MULTIBOOT_TAG_TYPE_END) break;
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            GrubParserMmap((struct multiboot_tag_mmap*)tag);
        }
        ptr += (tag->size + 7) & ~7;
    }
    parser_run = 1;
    return 0;
}