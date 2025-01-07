#include "pico/stdlib.h"
#include "aimonen/gdisp_lld.h"

#include "mcufont/fonts.h"
#include "mcufont/mcufont.h"

#include "gdisp_hld.h"

const struct mf_font_s *used_font = &mf_rlefont_DejaVuSerif16.font; //default font, can be changed


static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
    {
     // alpha gets ignored.
     unsigned int rot =0xFF00 & *(int*) state;
     color_t colour = 0x00FF & (*(int*)state);
        switch(rot){
            default:
            case ROT_0:
                while (count--)
                {
                    gdisp_lld_draw_pixel(x,y,colour);
                    x++;
                }
            break;
            case ROT_90:
                while (count--)
                {
                    gdisp_lld_draw_pixel(y, GDISP_SCREEN_HEIGHT-x ,colour);
                    x++;
                }
            break;
            case ROT_180:
                while (count--)
                {
                    gdisp_lld_draw_pixel(GDISP_SCREEN_WIDTH-x, GDISP_SCREEN_HEIGHT-y ,colour);
                    x++;
                }
            break;
            case ROT_270:
               while (count--)
                {
                    gdisp_lld_draw_pixel(GDISP_SCREEN_WIDTH-y, x ,colour);
                    x++;
                }
            break;
        }
    }

static uint8_t char_callback(int16_t x0, int16_t y0, mf_char character, void *state) 
    {
        return mf_render_character(used_font, x0, y0, character, &pixel_callback, state);
    }


void eink_set_font(char* fontname){
    used_font = mf_find_font(fontname);
    if(used_font==NULL) used_font= mf_find_font("DejaVuSerif16"); // default
}

void text_to_eink(int16_t x, int16_t y, char* text, enum rotation dir){
    unsigned int rot = dir;

    mf_render_aligned(
       used_font,
       x, y,
       MF_ALIGN_LEFT,
       text, 0,
       &char_callback, &rot);
}