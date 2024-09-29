#ifndef GDISP_HLD_H
#define GDIPS_HLD_H
#include "pico/stdlib.h"

void text_to_eink(int16_t x, int16_t y, char* text, uint8_t dir);
void clearscreen();
void clear_buffer();

#endif