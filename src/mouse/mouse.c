#include <io.h>
#include <pic.h>
#include <locks/spinlock.h>
#include <idt/idt.h>

#define MOUSE_SIZE_BUF 250

static uint8_t mouse_buffer[MOUSE_SIZE_BUF];
static SpinLock_t mouse_lock = SPINLOCK_INIT;
static uint8_t head = 0;
static uint8_t tail = 0;

void mouse_callback()
{
    SpinLockIrq(&mouse_lock);
    uint8_t data = inb(0x60);
    uint8_t next = (head + 1) % MOUSE_SIZE_BUF;
    if (next != tail) {
        mouse_buffer[head] = data;
        head = next;
    }
    PicSendEoi(12);
    SpinUnlockIrq(&mouse_lock);
}

extern void mouse_handler();
void MouseInit()
{
    idt_set_gate(0x2C,mouse_handler,0x8E);
    PicUnmaskIrq(12);
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
}