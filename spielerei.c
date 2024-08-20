#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "pico/binary_info.h"
#include "spielerei.pio.h"
#include "chargepump.pio.h"

const uint LED_PIN = 15; // now uses pin 15,16,17,18,19,20,21,22 to output 8 bit parallel data
uint32_t dispdata[1+800*600*2/32]; // 800*600 pixels, 2bits per pixel, 32 bits per item. Dus 16 pixels per item. 600 pixel per row (of 800), dus komt niet eens mooi uit

dispdata_init()
{
    for (uint j = 0; j < 600; j++)
    {
        for (uint i = 0; i < 800; i++)
        {
            if (i < 400)
                dispdata[i + j] = 0x5555;
            else
                dispdata[i + j] = 0xAAAA;
        }
    }
    dispdata[0]=0; //initial row skip
}

int main()
{


    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "LED op breadboard"));
    stdio_init_all();
    
    pio_sm_claim(pio0,0); // reserve PIO0, State machine 0 for the DMA transfer


    PIO pio = pio0;
    uint offset_spiel = pio_add_program(pio, &spielerei_program);
    uint sm_spiel = pio_claim_unused_sm(pio, true);
    uint offset_ch = pio_add_program(pio, &chargepump_program);
    uint sm_ch = pio_claim_unused_sm(pio, true);

    spielerei_program_init(pio, sm_spiel, offset_spiel, LED_PIN); // TODO: should modify the init fucntion to make it clear it uses multiple pins for parallel data
    chargepump_program_init_and_start(pio,sm_ch,offset_ch,12,50000);// 50 kHz charge pump waveforms on pins 12 and 13

    EPD_GPIO_Init();
    EPD_Init();
    EPD_Power_On();

    EPD_String_24(10,10,"Hello World!!",1);

    dispdata_init();
    int dmach = dma_claim_unused_channel(true);
    dma_channel_config eink_dma_ch_config = dma_channel_get_default_config(dmach);
    //default config has read increment and write to fixed adres, 32 bits wide, which is indeed what's needed here
    channel_config_set_dreq(&dmach,DREQ_PIO0_TX0); // sets DREQ to PIO TX. TODO: make sure this is the right PIO and/of maybe make this dynamic instead of hardcoded

// write the config and start the transfer
    dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        PIO0_BASE+TXF0, // TODO: Figure out how to make DMA write to PIO0 TXR
        &dispdata[0],
        true
    )



    while (1)
    {
        uint counter;
        counter++;
        pio_sm_put_blocking(pio,sm_spiel,counter); // for now, put a dataword this way. Later, figure out DMA to write out a display buffer
    }

}