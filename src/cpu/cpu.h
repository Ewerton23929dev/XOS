#ifndef CPU_H
#define CPU_H
#include <stdint.h>

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} CpuIdRegs_t;

void CpuHalt(void);           // Para a CPU até próxima interrupção
void CpuReboot(void);         // Reinicia a máquina
void CpuIdle(void);           // Espera interrupção (HLT)
uint32_t CpuReadEflags(void); // Lê registrador de flags
void CpuCli(void);            // Desabilita interrupções
void CpuSti(void);            // Habilita interrupções
void CpuId(uint32_t leaf, CpuIdRegs_t* regs); // executa CPUID
void CpuPrintVendor(void);
#endif