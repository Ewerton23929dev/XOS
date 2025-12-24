#pragma once
#include <stdint.h>

#define MULTIBOOT1_BOOTLOADER_MAGIC 0x2BADB002

/* Flags do multiboot_info */
#define MULTIBOOT_INFO_MEM_MAP (1 << 6)

/* Estrutura principal passada em EBX */
struct multiboot_info {
    uint32_t flags;

    uint32_t mem_lower;
    uint32_t mem_upper;

    uint32_t boot_device;

    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    uint32_t syms[4]; // a.out ou ELF, ignora

    uint32_t mmap_length;
    uint32_t mmap_addr;

    /* resto não interessa agora */
} __attribute__((packed));

/* Entrada do memory map */
struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));
