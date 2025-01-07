#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "pico/binary_info.h"
#include "ED060SC7_refresh.pio.h"
#include "aimonen/gdisp_lld.h"

#include "gdisp_hld.h"

    union screenbuffer displaydata; // TODO: make it local to gdisp_lld.c or ...hld.c and include all the DMA-related config there too...


int main()
{
    bi_decl(bi_program_description("E-ink Pio demo"));
    stdio_init_all();
    
    /*setup PIO and DMA*/
    pio_sm_claim(pio0,0); // reserve PIO0, State machine 0 for the DMA transfer
    PIO pio = pio0;
    uint offset_dmarw = pio_add_program(pio, &epd_refresh_program);
    uint sm_dmawr = pio_claim_unused_sm(pio, true);

    int dmach = dma_claim_unused_channel(true);
    dma_channel_config eink_dma_ch_config = dma_channel_get_default_config(dmach);
    //default config has read increment and write to fixed adres, 32 bits wide, which is indeed what's needed here

    uint dreq = pio_get_dreq(pio,sm_dmawr,true); // get the correct DREQ for this pio & statemachine
    channel_config_set_dreq(&eink_dma_ch_config, dreq); // sets DRE

    /* Init display - this is done by MCU, not through PIO */
    gdisp_lld_init();
    EPD_power_on();
    gdisp_lld_clear(WHITE); 

    epd_refresh_program_init(pio,sm_dmawr,offset_dmarw,9,7,2); // now let PIO snatch the pins so PIO can write the display

    /* write the config and DO NOT YET start the transfer */
   dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_dmawr],
        &displaydata.sb_words[0][0],
        GDISP_SCREEN_HEIGHT*(GDISP_SCREEN_WIDTH/(EINK_PPB*4)),
        false // true to start imeadeately, false to start later
    );

    clear_screenbuffer(WHITE); // prefil buffer with "WHITE" as background color. (just the BUFFER. Screen itself is unchanged untill write)

    /* Write grayscale test block */
   for (int grayframe = 0; grayframe < 3; grayframe++)
   {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       for (uint y = 500; y < 600; y++)
       {
           for (uint x = 10; x < 110 + grayframe * 100; x++)
           {
               gdisp_lld_draw_pixel(x, y, BLACK); // expanding black block (resulting in various shades of gray)
           }
       }

       while (dma_channel_is_busy(dmach))
       {
       };                 // wait untill DMA is done before changing data and starting next write
       busy_wait_us(350); // TODO: Why is this delay needed for a proper display?
    }

    clear_screenbuffer(NOCHANGE); // prefill screen buffer with "no change" data. (just the BUFFER. Screen itself is unchanged untill write)

    /*prepare black background in top right corner */
    for (uint y = 0; y < 100; y++)
    {
        for (uint x = 600; x < 800; x++)
        {
            gdisp_lld_draw_pixel(x, y, BLACK);
        }
    }


    /* prepare text data to write to display */
    eink_set_font("DejaVuSerif32");
    text_to_eink(10, 0, "E-ink.Eluke.nl",ROT_0);
    text_to_eink(10, 0, "E-ink.Eluke.nl",ROT_180);
    eink_set_font("DejaVuSerif16");
    text_to_eink(10, 35, "Demo with e-ink driven by DMA to PIO",ROT_0);
    text_to_eink(10, 35, "Demo with e-ink driven by DMA to PIO",ROT_180);
    text_to_eink(10, 60, "Downside up, Upside down",ROT_180);
    text_to_eink(10, 60, "Source code available! See blogpost at eluke.nl!",ROT_0);
    text_to_eink(250,380, "Hello World!", ROT_90); 
    text_to_eink(250,380, "Yellow Bird!", ROT_270);
    eink_set_font("fixed_10x20");
    text_to_eink(10, 480, "Grayscale withouth dithering.",ROT_0);
    //text_to_eink(610,60, "White text", ROT_0|WHITE); -- When written at the same time as the background colour, text bleeds.


    for(uint i = 250; i<350; i++) {
    gdisp_lld_draw_pixel(400,i, BLACK);  // 1px line between Hellow World and Yellow Bird
    }



/* Write text. Because background now is "no change" the grayscale block stays on the display. */
       for (int grayframe = 0; grayframe < 3; grayframe++) /* text is written multiple times too, so it can also be displayed in shades. */
   {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       while (dma_channel_is_busy(dmach))
       {
       };               
       busy_wait_us(350); 
    }

    /* prepare data for white text & line */
    clear_screenbuffer(NOCHANGE); 
    text_to_eink(610,30, "White text & line", ROT_0|WHITE);
    for(uint i = 610; i<790; i++) {
    gdisp_lld_draw_pixel(i, 55, WHITE);  // 1px white line on black bg
    }
    for (int grayframe = 0; grayframe < 3; grayframe++) /* text is written multiple times too, so it can also be displayed in shades. */
    {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       while (dma_channel_is_busy(dmach))
       {
       };               
       busy_wait_us(350); 
    }

    
    busy_wait_ms(500); // then wait a bit longer just for the bit in FIFO to be writen to the display. 
    //(TODO: in practice CPU should be doing something usefull and/or the busy/done signal should be used to know when to powerdown the eink)

    pio_sm_set_enabled(pio, sm_dmawr, false);// stop the PIO
    gdisp_lld_init(); // re-initialising display grabs the pins back to SIO and turns the display off. It also clears the screen buffer.
    
    /*ALTERNATIVELY could use PIO to force pins, but that wont work with a stalled PIO*/

}