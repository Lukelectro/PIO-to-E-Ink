#include "pico/stdlib.h"
#include "aimonen/gdisp_lld.h"

#include "mcufont/fonts.h"
#include "mcufont/mcufont.h"

void clear_buffer()
{
    clear_screenbuffer(3); // clear to 'No change'
}

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
    {
    while (count--)
        {
            gdisp_lld_draw_pixel(x,y,0);
            x++;
        }
    }

static void pixel_callback_90(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
    {
    while (count--)
        {
            gdisp_lld_draw_pixel(y, GDISP_SCREEN_HEIGHT-x ,0);
        }
    }

    static void pixel_callback_270(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
    {
    while (count--)
        {
            gdisp_lld_draw_pixel(GDISP_SCREEN_WIDTH-y, x ,0);
        }
    }

    static void pixel_callback_360(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
    {
    while (count--)
        {
            gdisp_lld_draw_pixel(GDISP_SCREEN_WIDTH-x, GDISP_SCREEN_HEIGHT-y ,0);
        }
    }


//TODO: Hum, would be nicer if font and orentation would be a variable passed into this, so no seperate funtion for each of these.
static uint8_t char_callback(int16_t x0, int16_t y0, mf_char character, void *state) 
    {
        return mf_render_character(&mf_rlefont_DejaVuSerif16.font, x0, y0, character, &pixel_callback, state);
    }

    static uint8_t char_callback_90(int16_t x0, int16_t y0, mf_char character, void *state)
    {
        return mf_render_character(&mf_rlefont_DejaVuSerif16.font, x0, y0, character, &pixel_callback_90, state);
    }

    static uint8_t char_callback_270(int16_t x0, int16_t y0, mf_char character, void *state)
    {
        return mf_render_character(&mf_rlefont_DejaVuSerif16.font, x0, y0, character, &pixel_callback_270, state);
    }

        static uint8_t char_callback_360(int16_t x0, int16_t y0, mf_char character, void *state)
    {
        return mf_render_character(&mf_rlefont_DejaVuSerif16.font, x0, y0, character, &pixel_callback_360, state);
    }


void text_to_eink(int16_t x, int16_t y, char* text, uint8_t dir){
    switch(dir) {
        default:
        case 0: // long edge, left to right
    mf_render_aligned(
       &mf_rlefont_DejaVuSerif16.font,
       x, y,
       MF_ALIGN_LEFT,
       text, 0,
       &char_callback, NULL);
       break;
       case 1: // short edge, left to right
       mf_render_aligned(
       &mf_rlefont_DejaVuSerif16.font,
       x, y,
       MF_ALIGN_LEFT,
       text, 0,
       &char_callback_90, NULL);
       break;
      case 2: // short edge, upside down
       mf_render_aligned(
       &mf_rlefont_DejaVuSerif16.font,
       x, y,
       MF_ALIGN_LEFT,
       text, 0,
       &char_callback_270, NULL);
       break;
        case 3: // long edge, upside down
       mf_render_aligned(
       &mf_rlefont_DejaVuSerif16.font,
       x, y,
       MF_ALIGN_LEFT,
       text, 0,
       &char_callback_360, NULL);
       break;
    }
}