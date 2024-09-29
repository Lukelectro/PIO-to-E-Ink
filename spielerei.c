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
    gdisp_lld_clear(1); // clear to white (1) or black (0)

// add a bit of test data 
// x is 800 pixels, y is 600 pixels, in total
    
    for(uint y=300;y<400;y++){
        for(uint x=100;x<200;x++){
        gdisp_lld_draw_pixel(x,y,0); // black block
    }
    }

    epd_refresh_program_init(pio,sm_dmarw,offset_dmarw,14,10,2); // now let PIO snatch the pins

    // put text in buffer:
       text_to_eink(100, 250, "e-ink.eluke.nl -- Demo with e-ink driver that does DMA to PIO",0);
       text_to_eink(400, 420, "And grayscale!",0);
       text_to_eink(250,550, "rotated", 1);
       text_to_eink(300,150, "rotated even more!", 2);
       text_to_eink(200,100, "Upside down", 3);

/* write the config and DO NOT YET start the transfer */
   dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_dmarw],
        &displaydata.sb_words[0][0],
        GDISP_SCREEN_HEIGHT*(GDISP_SCREEN_WIDTH/(EINK_PPB*4)),
        false // true to start imeadeately, false to start later
    );


   for (int grayframe = 0; grayframe < 3; grayframe++)
   {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       for (uint y = 300; y < 400; y++)
       {
           for (uint x = 100; x < 300 + grayframe * 120; x++)
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
    busy_wait_ms(1000); // then wait a bit longer just for the bit in FIFO to be writen to the display. 
    //(TODO: in practice CPU should be doing something usefull and/or the busy/done signal should be used to know when to powerdown the eink)

    EPD_power_off(); 

}