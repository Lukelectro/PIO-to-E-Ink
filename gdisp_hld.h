#ifndef GDISP_HLD_H
#define GDIPS_HLD_H
#include "pico/stdlib.h"
#include "mcufont/fonts.h"
#include "mcufont/mcufont.h"

void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state);
uint8_t char_callback(int16_t x0, int16_t y0, mf_char character, void *state);
void text_to_eink(int16_t x, int16_t y, char* text);

#endif