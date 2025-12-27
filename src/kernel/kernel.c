#include <pic.h>
#include <io.h>
#include <time.h>
#include <memory/gzm.h>
#include <timer/timer.h>
#include <mmu/mmu.h>
#include <vga/vga.h>
#include <boot/grub/grub.h>
#include <boot/bootstrap/bootstrap.h>

void kernel_main(void* info_addr)
{
    uint32_t reserved   = *(uint32_t*)(info_addr + 4);
    if (reserved != 0) panic("MBI INVALIDO!");
    GrubParserMb2(info_addr);
    asm volatile("cli");
    PicRemap(0x20,0x28); // NAO remover
    PicSetFreq(100); // NAO remover
    GlobalInitEarly();
    MapZones();
    while (1) {
        TimerPoll();
        TimerRunTasks();
        asm volatile("sti; hlt"); // descanso, pausa
    }
}