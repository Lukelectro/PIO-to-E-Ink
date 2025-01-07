#ifndef GDISP_HLD_H
#define GDIPS_HLD_H
#include "pico/stdlib.h"

enum rotation{ROT_0=0x0000,ROT_90=0x0100,ROT_180=0x0200,ROT_270=0x0300};

void text_to_eink(int16_t x, int16_t y, char* text, enum rotation dir);
void eink_set_font(char*); // set font to use, can be any one of MF_INCLUDED_FONTS.
void clearscreen();
void clear_buffer();

#endif