#ifndef GDISP_HLD_H
#define GDIPS_HLD_H
#include "pico/stdlib.h"

enum rotation{ROT_0,ROT_90,ROT_180,ROT_270};

void text_to_eink(int16_t x, int16_t y, char* text, enum rotation dir);
void eink_set_font(char*); // set font to use, can be any one of MF_INCLUDED_FONTS.
void clearscreen();
void clear_buffer();

#endif