#ifndef PIC_H
#define PIC_H
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182
#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void PicRemap(uint8_t offset1, uint8_t offset2);
void PicMaskIrq(uint8_t irq);
void PicUnmaskIrq(uint8_t irq);
void PicSendEoi(uint8_t irq);
void PicSetFreq(uint32_t freq);
#endif