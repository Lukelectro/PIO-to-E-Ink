#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "spielerei.pio.h"

const uint LED_PIN = 15; // now uses pin 15,16,17,18,19,20,21,22 to output 8 bit parallel data

int main()
{
    bi_decl(bi_program_description("This is a test binary."));
    bi_decl(bi_1pin_with_name(LED_PIN, "LED op breadboard"));
    stdio_init_all();
    
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &spielerei_program);
    uint sm = pio_claim_unused_sm(pio, true);

    spielerei_program_init(pio, sm, offset, LED_PIN); // TODO: should modify the init fucntion to make it clear it uses multiple pins for parallel data
    
    while (1)
    {
        uint counter;
        counter++;
        pio_sm_put_blocking(pio,sm,counter); // for now, put a dataword this way. Later, figure out DMA to write out a display buffer
    }

}