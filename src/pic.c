#include <stdint.h>
#include <io.h>
#include <pic.h>

void PicRemap(uint8_t offset1, uint8_t offset2)
{
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    outb(PIC1_DATA, a1 & ~0x01);
    outb(PIC2_DATA, a2);
}

void PicMaskIrq(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inb(port);
    value |= 1 << (irq % 8);
    outb(port, value);
}

void PicUnmaskIrq(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t value = inb(port);
    value &= ~(1 << (irq % 8));
    outb(port, value);
}

void PicSendEoi(uint8_t irq)
{
    if(irq >= 8)
        outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void PicSetFreq(uint32_t freq)
{
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / freq);

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}