#include <memory/alloc.h>
#include <locks/spinlock.h>
#include <timer/timer.h>
#include <vga/vga.h>
#include <vga/cursor.h>
#include <io.h>
#include <stdint.h>

static struct VgaLine_t lines_display[VGA_H] = {0}; // linhas para carregar
static size_t lines_display_init = 0; // estado de iniciado
static size_t lines_count_render = 0; // neead para rederizar algo
static SpinLock_t vga_lock = SPINLOCK_INIT; // locker
static struct Vga_t* vga_display = (struct Vga_t*)0xB8000; // dados reais

struct PanicEntry_t {
    const char* label;
    const char* name;
    uint32_t value;
};
#define PANIC_MAX_ENTRIES 32
static struct PanicEntry_t panic_stack[PANIC_MAX_ENTRIES];
static size_t panic_top = 0;

/* Nao depende da API do VGA! */
static void print_char_to_vga(int row, int col, char c, uint8_t color)
{
    if (row < 0 || row >= VGA_H || col < 0 || col >= VGA_W) return;
    vga_display[row * VGA_W + col].c = c;
    vga_display[row * VGA_W + col].color = color;
}
static void print_string_to_vga(int row, int col, const char* str)
{
    while (*str && col < VGA_W) {
        print_char_to_vga(row, col, *str, 0x4F);
        str++;
        col++;
    }
}
static void print_hex_to_vga(int row, int col, uint32_t value)
{
    const char* hex_chars = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        uint8_t nibble = (value >> ((7 - i) * 4)) & 0xF;
        print_char_to_vga(row, col + i, hex_chars[nibble], 0x4F);
    }
}
void PanicPush(const char* label, const char* name, uint32_t value)
{
    if (panic_top < PANIC_MAX_ENTRIES) {
        panic_stack[panic_top++] = (struct PanicEntry_t){label, name, value};
    }
}
__attribute__((noreturn))
void panic(const char* msg)
{
    asm volatile("cli"); // desativa interrupções
    for (int i = 0; i < VGA_W * VGA_H; i++) {
        vga_display[i].c = ' ';
        vga_display[i].color = 0x4F;
    }
    const char* header = "KERNEL PANIC";
    int header_len = 0;
    while (header[header_len]) header_len++;
    int header_col = (VGA_W - header_len) / 2;
    for (int i = 0; i < header_len; i++) {
        vga_display[i + header_col].c = header[i];
        vga_display[i + header_col].color = 0x4F;
    }
    int msg_row = 2; // linha 0 = header, linha 1 = espaço
    for (int i = 0; msg[i] && i < VGA_W * (VGA_H - 3); i++) {
        int row = msg_row + i / VGA_W;
        int col = i % VGA_W;
        vga_display[row * VGA_W + col].c = msg[i];
        vga_display[row * VGA_W + col].color = 0x4F;
    }
    int table_row = msg_row + 4; // deixa espaço
    for (int i = 0; i < panic_top && table_row + i < VGA_H; i++) {
        print_string_to_vga(table_row + i, 0, panic_stack[i].label);
        print_string_to_vga(table_row + i, 12, panic_stack[i].name);
        print_hex_to_vga(table_row + i, 24, panic_stack[i].value);
    }
    for (;;) {
        asm volatile("hlt");
    }
}





/* <------------| SISTEMA DE API |--------------> */





void VGAGetPhyLine(size_t y, struct VgaLine_t* line)
{
    if (y >= VGA_H || !line) return;
    line->start_line = &vga_display[y*VGA_W];
    line->end_line = line->start_line + (VGA_W-1);
    line->y_origin = y;
    line->flags = VGA_LINE_NONE;
}
void VGAGetDisplayLine(size_t y, struct VgaLine_t* line)
{
    if (y >= VGA_H || !line) return;
    *line = lines_display[y];
    if (!line->start_line) {
        line->start_line = &vga_display[y * VGA_W];
        line->end_line = line->start_line + (VGA_W - 1);
    }
    line->flags = VGA_LINE_NONE;
}
void VGAExternBufferCreateLine(size_t y,struct Vga_t* buffer,struct VgaLine_t* line)
{
    line->start_line = &buffer[y*VGA_W];
    line->end_line = line->start_line + (VGA_W-1);
    line->y_origin = y;
    line->flags |= VGA_LINE_BUFFER; // indica que e fora do lines_display.
    return;
}
void VGABufferCopyToDisplayLines(size_t y,struct VgaLine_t* line)
{
    if (y >= VGA_H || !line || !(line->flags & VGA_LINE_BUFFER)) return;
    SpinLock(&vga_lock);
    struct VgaLine_t* line_display = &lines_display[y];
    line_display->start_line = line->start_line;
    line_display->end_line = line->end_line;
    line_display->y_origin = line->y_origin;
    line_display->flags |= VGA_LINE_DIRTY; // manda atualizar
    line_display->flags |= VGA_LINE_BUFFER;
    line_display->valid = 1;
    lines_count_render++;
    SpinUnlock(&vga_lock);
}
static void VGABufferCopyToDisplayLinesNoLock(size_t y,struct VgaLine_t* line)
{
    if (y >= VGA_H || !line || !(line->flags & VGA_LINE_BUFFER)) return;
    struct VgaLine_t* line_display = &lines_display[y];
    line_display->start_line = line->start_line;
    line_display->end_line = line->end_line;
    line_display->y_origin = line->y_origin;
    line_display->flags |= VGA_LINE_DIRTY; // manda atualizar
    line_display->flags |= VGA_LINE_BUFFER;
    line_display->valid = 1;
    lines_count_render++;
}
void VGACopyToDisplayLines(size_t y, struct VgaLine_t* line)
{
    if (y >= VGA_H || !line || (line->flags & VGA_LINE_BUFFER)) return;
    SpinLock(&vga_lock);
    struct VgaLine_t* dst_line = &lines_display[y];
    struct Vga_t* dst = dst_line->start_line;
    struct Vga_t* src = line->start_line;
    int count = VGA_W;
    __asm__ __volatile__ (
        "1:\n\t"
        "movw (%[s]), %%ax\n\t"
        "movw %%ax, (%[d])\n\t"
        "add $2, %[s]\n\t"
        "add $2, %[d]\n\t"
        "dec %[c]\n\t"
        "jnz 1b\n\t"
        : [d] "+r"(dst), [s] "+r"(src), [c] "+r"(count)
        :
        : "ax", "memory"
    );
    dst_line->flags |= VGA_LINE_DIRTY;
    dst_line->valid = 1;
    lines_count_render++;
    SpinUnlock(&vga_lock);
}
static void VGACopyToDisplayLinesNoLock(size_t y, struct VgaLine_t* line)
{
    if (y >= VGA_H || !line || (line->flags & VGA_LINE_BUFFER)) return;
    struct VgaLine_t* dst_line = &lines_display[y];
    struct Vga_t* dst = dst_line->start_line;
    struct Vga_t* src = line->start_line;
    int count = VGA_W;
    __asm__ __volatile__ (
        "1:\n\t"
        "movw (%[s]), %%ax\n\t"
        "movw %%ax, (%[d])\n\t"
        "add $2, %[s]\n\t"
        "add $2, %[d]\n\t"
        "dec %[c]\n\t"
        "jnz 1b\n\t"
        : [d] "+r"(dst), [s] "+r"(src), [c] "+r"(count)
        :
        : "ax", "memory"
    );
    dst_line->flags |= VGA_LINE_DIRTY;
    dst_line->valid = 1;
    lines_count_render++;
}
void VGALineWriteCaracter(size_t x, struct VgaLine_t* line,char c,uint8_t color)
{
    if (x >= VGA_W || !line) return;
    SpinLock(&vga_lock);
    struct Vga_t* addr = &line->start_line[x];
    uint16_t val = (color << 8) | c;
    __asm__ __volatile__ (
        "movw %[v], (%[a])"
        :
        : [v] "r"(val), [a] "r"(addr)
        : "memory"
    );
    line->flags |= VGA_LINE_WRITE; // indica que foi escrito.
    lines_count_render++;
    SpinUnlock(&vga_lock);
}
void VGAClearLine(struct VgaLine_t* line) {
    if (!line) return;
    SpinLock(&vga_lock);
    uint32_t* ptr = (uint32_t*)line->start_line;
    uint32_t clear = 0x00200020; // ' ' + color 0x00 duas vezes
    int count = VGA_W / 2;
    for (int i = 0; i < count; i++) {
        ptr[i] = clear;
    }
    if (VGA_W & 1) {
        line->start_line[VGA_W-1].c = ' ';
        line->start_line[VGA_W-1].color = 0x00;
    }
    line->flags |= VGA_LINE_WRITE;
    lines_count_render++;
    SpinUnlock(&vga_lock);
}

void VGAClearLineNoLock(struct VgaLine_t* line) {
    if (!line) return;
    uint32_t* ptr = (uint32_t*)line->start_line;
    uint32_t clear = 0x00200020; // ' ' + color 0x00 duas vezes
    int count = VGA_W / 2;
    for (int i = 0; i < count; i++) {
        ptr[i] = clear;
    }
    if (VGA_W & 1) {
        line->start_line[VGA_W-1].c = ' ';
        line->start_line[VGA_W-1].color = 0x00;
    }
    line->flags |= VGA_LINE_WRITE;
    lines_count_render++;
}

void VGALineWritePixel(size_t x, struct VgaLine_t* line, uint8_t color)
{
    if (x >= VGA_W || !line) return;
    SpinLock(&vga_lock);
    struct Vga_t* addr_pixel = &line->start_line[x];
    addr_pixel->c = ' ';
    addr_pixel->color = color;
    line->flags |= VGA_LINE_WRITE;
    lines_count_render++;
    SpinUnlock(&vga_lock);
}
void VGAPushLine(struct VgaLine_t* line,struct VgaLine_t* line_last)
{
    if (!line) return
    SpinLock(&vga_lock);
    *line_last = lines_display[VGA_H-1];
    for (int y = VGA_H-1; y > 0; y--) {
        lines_display[y] = lines_display[y-1];
        lines_display[y].flags |= VGA_LINE_DIRTY;
    }
    lines_display[0] = *line;
    lines_display[0].flags |= VGA_LINE_DIRTY;
    lines_count_render = VGA_H;
    SpinUnlock(&vga_lock);
}
void VGAPopLine(struct VgaLine_t* line,struct VgaLine_t* line_fist)
{
    if (!line) return;
    SpinLock(&vga_lock);
    *line_fist = lines_display[0];
    for (int y = 0; y < VGA_H-1; y++) {
        lines_display[y] = lines_display[y+1];
        lines_display[y].flags |= VGA_LINE_DIRTY;
    }
    lines_display[VGA_H-1] = *line;
    lines_display[VGA_H-1].flags |= VGA_LINE_DIRTY;
    lines_count_render = VGA_H;
    SpinUnlock(&vga_lock);
}
size_t VGAGetWritePose(struct VgaLine_t* line)
{
    if (!line) return 0;
    struct Vga_t* line_data = line->start_line;
    for (int i = VGA_W-1; i >= 0; i--) {
        if (line_data[i].c != ' ') return i;
    }
    return 0;
}

void VGAFushDisplay()
{
    if (lines_count_render != 0) {
        for (int y = 0; y < lines_count_render && y < VGA_H; y++) {
            struct VgaLine_t* line = &lines_display[y];
            if (!line || line->flags == VGA_LINE_NONE) continue; // NONE sem autoridade, ou 0
            if (!(line->flags & VGA_LINE_WRITE)) continue; // pule linhas nao modificadas
            if (line->flags & VGA_LINE_BUFFER) VGABufferCopyToDisplayLinesNoLock(y,line);
            if (!(line->flags & VGA_LINE_BUFFER)) VGACopyToDisplayLinesNoLock(y,line);
            line->flags &= ~VGA_LINE_DIRTY; // perde poder de flush
            line->flags &= ~VGA_LINE_WRITE; // perde que foi escrito, flag util e otimizaçao
        }
    }
    lines_count_render = 0;
}

void VgaInit()
{
    for (int i = 0; i < VGA_H; i++) {
        struct VgaLine_t line_real;
        VGAGetPhyLine(i,&line_real); // pego a linha do vga real.
        VGAClearLineNoLock(&line_real);
        VGACopyToDisplayLinesNoLock(i,&line_real); // jogo no arry de render.
    }
    lines_display_init = 1;
    VGACursorInit();
    struct TimerCtx_t vga;
    vga.func = VGAFushDisplay;
    vga.period = 3;
    vga.remaining = 1;
    RegistreQuest(&vga);
    VGACursorInit();
}