#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "pico/binary_info.h"
#include "spielerei.pio.h"
#include "chargepump.pio.h"
#include "ED060SC4_row_write.pio.h"
#include "epd.h"

const uint LED_PIN = 14; // now uses pin 15,16,17,18,19,20,21,22 to output 8 bit parallel data
#define DISPDATASIZE (1+800*600*2/32)
uint32_t dispdata[DISPDATASIZE]; // 800*600 pixels, 2bits per pixel, 32 bits per item. Dus 16 pixels per item. 600 pixel per row (of 800), dus komt niet eens mooi uit

// TODO: test of die statemachien steeds de laaste 8 bits pakt van de 32 en de rest wegmikt, of dat ze alle 32 gebruikt worden in groepjes van 8, want afaik is dat configureerbaar maar ik weet niet wat de default is.
// Done: Met autopull kun je inderdaad alle bits gebruiken, met 'handmatige' pull pak je elke pull 32 nieuwe bits en zou je dus 4 out instructies moeten gebruiken hetgeen geheugen verspilt. (Want autopull kost ook nog eens geen intstructie)
void dispdata_init()
{
        for (uint i = 0; i < DISPDATASIZE; i++)
        {
            if (i<(DISPDATASIZE/2))
                dispdata[i] = 0x55555555; // testen met 0x55555555 en 0xAAAAAAAA om zwart en wit op display te krijgen, 0b01 en 0b10 doen iets maar 11 en 00 zijn 'doe niks'.
                else
                dispdata[i] = 0xAAAAAAAA;
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
    uint offset_dmarw = pio_add_program(pio, &rowwrite_program);
    uint sm_dmarw = pio_claim_unused_sm(pio, true);
    //uint offset_ch = pio_add_program(pio, &chargepump_program);
    //uint sm_ch = pio_claim_unused_sm(pio, true);

    //chargepump_program_init_and_start(pio,sm_ch,offset_ch,12,50000);// 50 kHz charge pump waveforms on pins 12 and 13

    int dmach = dma_claim_unused_channel(true);
    dma_channel_config eink_dma_ch_config = dma_channel_get_default_config(dmach);
    //default config has read increment and write to fixed adres, 32 bits wide, which is indeed what's needed here

    uint dreq = pio_get_dreq(pio,sm_dmarw,true); // get the correct DREQ for this pio & statemachine
    channel_config_set_dreq(&eink_dma_ch_config, dreq); // sets DRE

    dispdata_init();

    EPD_Init();
    EPD_Power_On();
    EPD_Clear(); // dit nog in software

    rowwrite_program_init(pio,sm_dmarw,offset_dmarw,14,10,2); // now let PIO snatch the pins

    
// write the config and DO NOT YET start the transfer
   dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_dmarw],
        &dispdata,
        DISPDATASIZE,
        false // true to start imeadeately, false to start later
    );


    if(!dma_channel_is_busy(dmach))
        {
            // once DMA is no longer busy, load new data and restart transfer           
            dma_channel_set_read_addr(dmach, dispdata, true); // re-set read adress and restart transfer
        }

    while(dma_channel_is_busy(dmach)){}; // wait untill DMA is done before powering off
    busy_wait_ms(500); // then wait a bit longer just for the bit in FIFO to be writen to the display. 
    //(TODO: in practice CPU should be doing something usefull and/or the busy/done signal should be used to know when to powerdown the eink)

    EPD_Power_Off(); 



// spielerij pio program is on the same pins, so init/start it only AFTER the other test with the display
    //spielerei_program_init(pio, sm_spiel, offset_spiel, LED_PIN); // TODO: should modify the init fucntion to make it clear it uses multiple pins for parallel data
    
   
    //int dmach = dma_claim_unused_channel(true);
    //dma_channel_config eink_dma_ch_config = dma_channel_get_default_config(dmach);
    //default config has read increment and write to fixed adres, 32 bits wide, which is indeed what's needed here

   // uint dreq = pio_get_dreq(pio,sm_spiel,true); // get the correct DREQ for this pio & statemachine
   // channel_config_set_dreq(&eink_dma_ch_config, dreq); // sets DREQ

// write the config and DO NOT YET start the transfer
   /* dma_channel_configure(
        dmach, 
        &eink_dma_ch_config,
        &pio->txf[sm_spiel],
        &dispdata,
        DISPDATASIZE,
        false // true to start imeadeately, false to start later
    );

    while (1)
    {
        uint counter;

        if(!dma_channel_is_busy(dmach))
        {
            // once DMA is no longer busy, load new data and restart transfer
            dispdata[0] = 0; 
            dispdata[1] = counter;
            counter++;
           
            //dma_channel_set_read_addr(dmach, &dispdata, true); // need to Re-set read adress, as that is aut-incremented during a transfer. If a transfer is re-started without re-setting the read adres, it will read from there onwards. 
            //and sset_read_addr re-triggers (true) so no need for dma_channel_start(dmach); // re-start DMA transfer 
            
            //TODO: use (re-)starting the DMA transfer to start a rowwrite after other display-setup is done and/or let pio handle setup as well
        }

        // pio_sm_put_blocking(pio,sm_spiel,counter); // for now, put a dataword this way. Later, figure out DMA to write out a display buffer
    }
    */
}