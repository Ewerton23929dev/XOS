#pragma once
#include <stdint.h>

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct isr_context_t {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t error_code;
    uint32_t eip, cs, eflags;
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(uint8_t n, void (*handler)(void), uint8_t flags);