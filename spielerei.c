#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "pico/binary_info.h"
#include "ED060SC7_refresh.pio.h"
#include "aimonen/gdisp_lld.h"

#include "mcufont/fonts.h"
#include "mcufont/mcufont.h"

    union screenbuffer displaydata; // TODO: make it local to gdisp_lld.c and include all the DMA-related config there too...

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
        return mf_render_character(&mf_rlefont_DejaVuSans12.font, x0, y0, character, &pixel_callback, state);
    }


int main()
{

    bi_decl(bi_program_description("This is a test binary."));
    stdio_init_all();
    
    pio_sm_claim(pio0,0); // reserve PIO0, State machine 0 for the DMA transfer

    PIO pio = pio0;
    uint offset_dmarw = pio_add_program(pio, &epd_refresh_program);
    uint sm_dmarw = pio_claim_unused_sm(pio, true);

    int dmach = dma_claim_unused_channel(true);
    dma_channel_config eink_dma_ch_config = dma_channel_get_default_config(dmach);
    //default config has read increment and write to fixed adres, 32 bits wide, which is indeed what's needed here

    uint dreq = pio_get_dreq(pio,sm_dmarw,true); // get the correct DREQ for this pio & statemachine
    channel_config_set_dreq(&eink_dma_ch_config, dreq); // sets DRE

    
    gdisp_lld_init();
    EPD_power_on();
    //gdisp_lld_clear(0); // clear to black
    gdisp_lld_clear(1); // clear to white

// add a bit of test data 

// x is 800 pixels, y is 600 pixels, in total
    
    for(uint y=300;y<400;y++){
        for(uint x=100;x<200;x++){
        gdisp_lld_draw_pixel(x,y,0); // black block
    }
    }
 

    for(uint y=600;y>0;y--){
        uint x=y*800/600;
        gdisp_lld_draw_pixel(x,y,0); // black diagonal line
    }

        for(uint y=0;y<600;y++){
        uint x=y;
        gdisp_lld_draw_pixel(x,y,0); // black diagonal line
    }



    //screenrefresh(); // schrijf naar display (note: it wrote a bit before due to lack of buffer, oh well)

  // EPD_power_off();
  // while(1);

    epd_refresh_program_init(pio,sm_dmarw,offset_dmarw,14,10,2); // now let PIO snatch the pins

    mf_render_aligned(
       &mf_rlefont_DejaVuSans12.font,
       50, 50,
       MF_ALIGN_LEFT,
       "e-ink.eluke.nl test with e-ink driver that does DMA to PIO", strlen("e-ink.eluke.nl test with e-ink driver that does DMA to PIO"),
       &char_callback, NULL);


/* write the config and DO NOT YET start the transfer */
   dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_dmarw],
        &displaydata.sb_words[0][0],
        GDISP_SCREEN_HEIGHT*(GDISP_SCREEN_WIDTH/(EINK_PPB*4)),
        false // true to start imeadeately, false to start later
    );


   for (int grayframe = 0; grayframe < 4; grayframe++)
   {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       for (uint y = 300; y < 400; y++)
       {
           for (uint x = 100; x < 300 + grayframe * 100; x++)
           {
               gdisp_lld_draw_pixel(x, y, 0); // expanding black block (resulting in various shades of gray)
           }
       }

       while (dma_channel_is_busy(dmach))
       {
       };                 // wait untill DMA is done before powering off
       busy_wait_us(350); // test with a forced delay in between rewrites
       // might it be a power supply issue?
       // busy_wait_ms(500); // lets try a reeeal ssllllloowwwwww delay to see if it is the power suply
    }
    busy_wait_ms(5000); // then wait a bit longer just for the bit in FIFO to be writen to the display. 
    //(TODO: in practice CPU should be doing something usefull and/or the busy/done signal should be used to know when to powerdown the eink)

    EPD_power_off(); 

}