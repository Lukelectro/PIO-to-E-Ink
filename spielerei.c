#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "pico/binary_info.h"
#include "ED060SC4_row_write.pio.h"
#include "aimonen/gdisp_lld.h"

    union screenbuffer displaydata; // TODO: make it local to gdisp_lld.c and include all the DMA-related config there too...

int main()
{

    bi_decl(bi_program_description("This is a test binary."));
    stdio_init_all();
    
    pio_sm_claim(pio0,0); // reserve PIO0, State machine 0 for the DMA transfer

    PIO pio = pio0;
    uint offset_dmarw = pio_add_program(pio, &rowwrite_program);
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
    
    for(uint y=200;y<400;y++){
        for(uint x=200;x<400;x++){
        gdisp_lld_draw_pixel(x,y,0); // black block
    }
    }

        for(uint x=220;x<320;x++){
        for(uint y=220;y<370;y++){
        gdisp_lld_draw_pixel(x,y,1); // white center block
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

    rowwrite_program_init(pio,sm_dmarw,offset_dmarw,14,10,2); // now let PIO snatch the pins


/* write the config and DO NOT YET start the transfer */
   dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_dmarw],
        &displaydata.sb_words[0][0],
        GDISP_SCREEN_HEIGHT*(GDISP_SCREEN_WIDTH/(EINK_PPB*4)),
        false // true to start imeadeately, false to start later
    );

    for(int grayframe=0;grayframe<3;grayframe++){
    if(!dma_channel_is_busy(dmach))
        {
            // once DMA is no longer busy, load new data and restart transfer           
            dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
        }
    while(dma_channel_is_busy(dmach)){}; // wait untill DMA is done before powering off
    busy_wait_us(312); // test with a forced delay in between rewrites
    }
    busy_wait_ms(500); // then wait a bit longer just for the bit in FIFO to be writen to the display. 
    //(TODO: in practice CPU should be doing something usefull and/or the busy/done signal should be used to know when to powerdown the eink)

    EPD_power_off(); 

}