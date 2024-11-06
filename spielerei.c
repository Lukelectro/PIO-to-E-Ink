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
    gdisp_lld_clear(WHITE); 
    //clear_screenbuffer(NOCHANGE11); // TODO: Hah! this matters! On a white background fine 1 pixel black lines disapear but on a 0b00 or 0b11 background they are sharp!
    // so TODO: make a demo that demonstrates this with explanation in text on the display!
    clear_screenbuffer(NOCHANGE00);

    epd_refresh_program_init(pio,sm_dmarw,offset_dmarw,9,7,2); // now let PIO snatch the pins

        //set font:
    //   eink_set_font("DejaVuSerif32");
    // put text in buffer:
       text_to_eink(100, 250, "e-ink.eluke.nl -- Demo with e-ink driver that does DMA to PIO",ROT_0);
       text_to_eink(400, 420, "And grayscale!",ROT_0);
       text_to_eink(250,550, "rotated 90 degrees", ROT_90);
       eink_set_font("DejaVuSerif32");
       text_to_eink(300,150, "rotated even more! (270)", ROT_270);
       eink_set_font("DoesNotExist");
       text_to_eink(200,100, "Upside down (180)", ROT_180);
       eink_set_font("fixed_7x14"); 
       text_to_eink(0,50, "Single pixel and double pixel lines below.", ROT_0);
       for(uint i = 0; i<500; i++) {
        gdisp_lld_draw_pixel(i, 70, 0); // lijntje dat hopelijk niet verdwijnt, nog steeds enkel

        gdisp_lld_draw_pixel(i, 80, 0); // lijntje dat hopelijk niet verdwijnt, dubbel
        gdisp_lld_draw_pixel(i, 81, 0); // 
        
        gdisp_lld_draw_pixel(780, i, 0); // lijntje dat hopelijk niet verdwijnt, nog steeds enkel

        gdisp_lld_draw_pixel(790, i, 0); // lijntje dat hopelijk niet verdwijnt, dubbel
        gdisp_lld_draw_pixel(791, i, 0); // 

       }

/* write the config and DO NOT YET start the transfer */
   dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_dmarw],
        &displaydata.sb_words[0][0],
        GDISP_SCREEN_HEIGHT*(GDISP_SCREEN_WIDTH/(EINK_PPB*4)),
        false // true to start imeadeately, false to start later
    );

// first write text, then later write the gray block. Both at onces gives less crisp text
       for (int grayframe = 0; grayframe < 4; grayframe++)
   {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       while (dma_channel_is_busy(dmach))
       {
       };                 // wait untill DMA is done before powering off
       busy_wait_ms(1); // test with a forced delay in between rewrites
       // might it be a power supply issue?
    }

    clear_screenbuffer(3); // buffer default (background) to "no change" 0b00 (not white)
    clear_screenbuffer(2); // buffer default (background) to "no change" 0b11 (not white)

   for (int grayframe = 0; grayframe < 3; grayframe++)
   {
       if (!dma_channel_is_busy(dmach))
       {
           // once DMA is no longer busy, load new data and restart transfer
           dma_channel_set_read_addr(dmach, &displaydata.sb_words[0][0], true); // re-set read adress and restart transfer
       }

       for (uint y = 300; y < 400; y++)
       {
           for (uint x = 100; x < 200 + grayframe * 100; x++)
           {
               gdisp_lld_draw_pixel(x, y, 0); // expanding black block (resulting in various shades of gray)
           }
       }

       while (dma_channel_is_busy(dmach))
       {
       };                 // wait untill DMA is done before powering off
       busy_wait_us(350); // test with a forced delay in between rewrites
       // might it be a power supply issue?
    }
    busy_wait_ms(500); // then wait a bit longer just for the bit in FIFO to be writen to the display. 
    //(TODO: in practice CPU should be doing something usefull and/or the busy/done signal should be used to know when to powerdown the eink)

    EPD_power_off(); // TODO: since PIO has the pins, this now doesn't set the pins before removing power...
    // So force the PIO to set all pins it controls to low: (TODO! THis needs work as it won't work with a stalled PIO but with a running PIO it will change the pins during its run...)
    pio_sm_put(pio,sm_dmarw,0); // write 0 to un-stall PIO waiting for data TODO: maybe stop the PIO from running instead and then force the pins after? Set them back to SIO function?
    pio_sm_exec_wait_blocking(pio,sm_dmarw,pio_encode_out(pio_pins,0x00));
    pio_sm_exec_wait_blocking(pio,sm_dmarw,pio_encode_sideset(pio_pins,0x00));
    pio_sm_exec_wait_blocking(pio,sm_dmarw,pio_encode_set(pio_pins,0x00));
}