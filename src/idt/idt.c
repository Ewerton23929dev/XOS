#include "idt.h"

static struct idt_entry idt[256];
static struct idt_ptr idtr;

static void idt_set_gate_internal(
    uint8_t n,
    uint32_t handler,
    uint16_t selector,
    uint8_t flags
) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = selector;
    idt[n].zero        = 0;
    idt[n].flags       = flags;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_set_gate(uint8_t n, void (*handler)(void), uint8_t flags) {
    idt_set_gate_internal(n, (uint32_t)handler, 0x08, flags);
}

void invalid_opcode(struct isr_context_t* ctx)
{
    //ctx->eip // endereço da funçao
    while (1);
}
extern void invalid_opcode_handler();
void idt_init(void) {
    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint32_t)&idt;
    idt_set_gate(6,invalid_opcode_handler,0x08E);
    asm volatile("lidt %0" : : "m"(idtr));
}

#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(idt_init,0);