#ifndef CURSOR_VGA_H
#define CURSOR_VGA_H
#include <io.h>
#include <stdint.h>

void VGACursorSetPose(size_t y, size_t x);
void VGACursorVisible(uint8_t flag);
void VGACApplyCursorHW(void);
void VGACursorInit();
#endif