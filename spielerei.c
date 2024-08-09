#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "spielerei.pio.h"
#include "chargepump.pio.h"

const uint LED_PIN = 15; // now uses pin 15,16,17,18,19,20,21,22 to output 8 bit parallel data

int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "LED op breadboard"));
    stdio_init_all();
    
    PIO pio = pio0;
    uint offset_spiel = pio_add_program(pio, &spielerei_program);
    uint sm_spiel = pio_claim_unused_sm(pio, true);
    uint offset_ch = pio_add_program(pio, &chargepump_program);
    uint sm_ch = pio_claim_unused_sm(pio, true);

    spielerei_program_init(pio, sm_spiel, offset_spiel, LED_PIN); // TODO: should modify the init fucntion to make it clear it uses multiple pins for parallel data
    chargepump_program_init_and_start(pio,sm_ch,offset_ch,12,50000);// 50 kHz charge pump waveforms on pins 12 and 13

    
    while (1)
    {
        uint counter;
        counter++;
        pio_sm_put_blocking(pio,sm_spiel,counter); // for now, put a dataword this way. Later, figure out DMA to write out a display buffer
    }

}