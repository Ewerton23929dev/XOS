#include <idt/idt.h>
#include <pic.h>
#include <mmu/mmu.h>
#include <memory/alloc.h>
#include <timer/timer.h>
#include <keyboard/keyboard.h>
#include <mmu/mmu_phys.h>
#include <io.h>
#include <boot/grub/grub.h>
#include <stddef.h>
#include <memory/gzm.h>
#include <vga/vga.h>
#include <program/program.h>
#include <memory/slab.h>
#include <console/console.h>

void kernel_main(void* info_addr)
{
    asm volatile("cli");
    PicRemap(0x20,0x28); // NAO remover
    PicSetFreq(100); // NAO remover
    idt_init();
    GzmInit();
    KmallocinitBoot();
    MmuPhysInit();
    MemorySlabInit();
    MmuInit();
    TimerInit();
    KeyboardInit();
    VgaInit();
    ConsoleInit();
    ProgramInit();
    MapZones();
    SlabAlloc(1*1024);
    while (1) {
        TimerPoll();
        TimerRunTasks();
        asm volatile("sti; hlt"); // descanso, pausa
    }
}