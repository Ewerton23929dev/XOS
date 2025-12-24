#ifndef VGA_HEADER
#define VGA_HEADER
#include <io.h>

#define VGA_BLACK 0x0
#define VGA_BLUE 0x1
#define VGA_GREEN 0x2
#define VGA_CIAN 0x3
#define VGA_RED 0x4
#define VGA_MAG 0x5
#define VGA_BROW 0x6
#define VGA_WHITE 0x7

#define VGA_H 25
#define VGA_W 80

struct Vga_t {
    char c;
    uint8_t color;
} __attribute__((packed));
typedef enum {
    VGA_LINE_NONE  = 0,
    VGA_LINE_DIRTY = 1 << 0,
    VGA_LINE_BUFFER = 2 << 0,
    VGA_LINE_WRITE = 3 << 0
} VgaLineFlags_t;
struct VgaLine_t {
    uint8_t valid;
    uint32_t y_origin;
    struct Vga_t* start_line;
    struct Vga_t* end_line;
    VgaLineFlags_t flags; 
};

void VgaInit();
void VGAGetPhyLine(size_t y, struct VgaLine_t* line);
void VGAGetDisplayLine(size_t y, struct VgaLine_t* line);
void VGAExternBufferCreateLine(size_t y,struct Vga_t* buffer,struct VgaLine_t* line);
void VGABufferCopyToDisplayLines(size_t y,struct VgaLine_t* line);
void VGACopyToDisplayLines(size_t y, struct VgaLine_t* line);
void VGALineWriteCaracter(size_t x, struct VgaLine_t* line,char c,uint8_t color);
void VGALineWritePixel(size_t x, struct VgaLine_t* line, uint8_t color); // is fun
void VGAPushLine(struct VgaLine_t* line,struct VgaLine_t* line_last);
void VGAPopLine(struct VgaLine_t* line,struct VgaLine_t* line_fist);
size_t VGAGetWritePose(struct VgaLine_t* line);
__attribute__((noreturn)) void panic(const char* msg);
void PanicPush(const char* label, const char* name, uint32_t value);
#endif