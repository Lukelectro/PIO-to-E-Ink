#include "pico/stdlib.h"
#include "aimonen/gdisp_lld.h"

#include "mcufont/fonts.h"
#include "mcufont/mcufont.h"

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
    {
    while (count--)
        {
            gdisp_lld_draw_pixel(x,y,0);/* your code goes here, ex: drawPixel(x, y, alpha, color::black); */
            x++;
        }
    }

static uint8_t char_callback(int16_t x0, int16_t y0, mf_char character, void *state)
    {
        return mf_render_character(&mf_rlefont_DejaVuSerif16.font, x0, y0, character, &pixel_callback, state);
    }

void text_to_eink(int16_t x, int16_t y, char* text){
    mf_render_aligned(
       &mf_rlefont_DejaVuSerif16.font,
       x, y,
       MF_ALIGN_LEFT,
       text, 0,
       &char_callback, NULL);
}