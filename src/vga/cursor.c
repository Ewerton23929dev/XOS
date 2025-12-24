#include <io.h>
#include <timer/timer.h>
#include <stdint.h>
#include <vga/vga.h>
#include <locks/spinlock.h>

static struct CursorVgaSate_t {
    size_t y;
    size_t x;
    uint8_t show; // esta amostra
} cursor_vga_state = {0};
static SpinLock_t cursor_vga_lock = SPINLOCK_INIT;

void VGACursorSetPose(size_t y, size_t x)
{
    if (y >= VGA_H || x >= VGA_W) return;
    struct VgaLine_t line;
    VGAGetDisplayLine(y,&line);
    SpinLock(&cursor_vga_lock);
    cursor_vga_state.x = x;
    cursor_vga_state.y = y;
    cursor_vga_state.show = 1;
    struct Vga_t* next = &line.start_line[x];
    next->c = ' ';
    next->color = 0x0F;
    line.flags |= VGA_LINE_DIRTY;
    SpinUnlock(&cursor_vga_lock);
}
void VGACursorVisible(uint8_t flag)
{
    SpinLock(&cursor_vga_lock);
    outb(0x3D4, 0x0A);
    uint8_t val = 14; // start scanline da linha fina
    if (!flag) val |= 0x20; // bit 5 = 1 -> cursor off
    outb(0x3D5, val);
    cursor_vga_state.show = flag;
    SpinUnlock(&cursor_vga_lock);
}

void VGACApplyCursorHW(void)
{
    uint16_t pos = cursor_vga_state.y * VGA_W + cursor_vga_state.x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);
    outb(0x3D5, pos >> 8);
}

// APPLY 

static void VGACTick()
{
    SpinLock(&cursor_vga_lock);
    if (!cursor_vga_state.show) {
        // Cursor OFF
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
    } 
    if (cursor_vga_state.show) VGACApplyCursorHW();
    SpinUnlock(&cursor_vga_lock);
}

void VGACursorInit()
{
    asm volatile("cli");
    outb(0x3D4, 0x0A);   // Cursor Start
    outb(0x3D5, 14);     // linha inicial (cursor ligado)
    outb(0x3D4, 0x0B);   // Cursor End
    outb(0x3D5, 15);     // linha final
    cursor_vga_state.show = 1;
    VGACApplyCursorHW();
    struct TimerCtx_t cursor;
    cursor.func = VGACTick;
    cursor.period = 7;
    cursor.remaining = 5;
    RegistreQuest(&cursor);
    asm volatile("sti");
}