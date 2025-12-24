#include <cpu/cpu.h>
#include <console/console.h>
#include <io.h>

void CpuHalt(void) {
    asm volatile("hlt");
}

void CpuIdle(void) {
    asm volatile("sti; hlt"); // garante que IRQ ainda funciona
}

void CpuCli(void) {
    asm volatile("cli");
}

void CpuSti(void) {
    asm volatile("sti");
}

uint32_t CpuReadEflags(void) {
    uint32_t flags;
    asm volatile("pushf; pop %0" : "=r"(flags));
    return flags;
}

// Reboot via porta do teclado (8042 controller)
void CpuReboot(void) {
    CpuCli(); // desliga interrupções
    outb(0x64, 0xFE); // comando de reset
    while (1) asm volatile("hlt"); // fallback caso não reinicie
}

void CpuId(uint32_t leaf, CpuIdRegs_t* regs) {
    asm volatile(
        "cpuid"
        : "=a"(regs->eax), "=b"(regs->ebx), "=c"(regs->ecx), "=d"(regs->edx)
        : "a"(leaf)
    );
}

void CpuPrintVendor(void) {
    CpuIdRegs_t regs;
    CpuId(0, &regs); // leaf 0 = vendor string

    char vendor[13];
    *((uint32_t*)&vendor[0]) = regs.ebx;
    *((uint32_t*)&vendor[4]) = regs.edx;
    *((uint32_t*)&vendor[8]) = regs.ecx;
    vendor[12] = '\0';

    ConsolePrint("CPU Vendor: ");
    ConsolePrint(vendor);
    ConsolePrint("\n");
}

void CpuPowerOff()
{
    outw(0x604, 0x2000);
    for(;;) __asm__("hlt");
}