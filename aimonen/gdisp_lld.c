/*
 * 2013 Petteri Aimonen <jpa@gfx.mail.kapsi.fi>
 * This file is released to the public domain.
 */

/* Low-level E-ink panel driver routines for ED060SC4. */

// for use withouth uGFX, commented out 
// #include "gfx.h"
// #include "ed060sc4.h"

// #if GFX_USE_GDISP for use withouth uGFX, commented out
//#include "gdisp/lld/emulation.c"

//added:
#include "gdisp_lld.h"
#include <stdbool.h>

/* glue because stdbool is lowercase here */
#define TRUE true
#define FALSE false
/* */


/* =================================
 *      configuration
 * ================================= */

#ifndef GDISP_SCREEN_HEIGHT
#       define GDISP_SCREEN_HEIGHT 600 // Y was 600, but -SC7 is 3:4 where -SC4 is 4:3
#endif

#ifndef GDISP_SCREEN_WIDTH
#       define GDISP_SCREEN_WIDTH 800 //X was 800, but -SC7 is 3:4 where -SC4 is 4:3 // perhaps rotation is not done here?
#endif

/* Number of pixels per byte */
#ifndef EINK_PPB
#       define EINK_PPB 4
#endif

/* Delay for generating clock pulses.
 * Unit is approximate clock cycles of the CPU.
 * This should be atleast 50 ns. (on-SC4, possible minimum is 200 ns + on ED060SC7)
 */
#ifndef EINK_CLOCKDELAY
#       define EINK_CLOCKDELAY  60 
#endif

/* Do a "blinking" clear, i.e. clear to opposite polarity first.
 * This reduces the image persistence. */
#ifndef EINK_BLINKCLEAR
#       define EINK_BLINKCLEAR TRUE
#endif

/* Number of passes to use when clearing the display */
#ifndef EINK_CLEARCOUNT
#       define EINK_CLEARCOUNT 10
#endif

/* Number of passes to use when writing to the display */
#ifndef EINK_WRITECOUNT
#       define EINK_WRITECOUNT 4
#endif

/* ====================================
 *      Lower level driver functions
 * ==================================== */

//#include "gdisp_lld_board.h"
#include "gdisp_lld_board_RP2040.h"

uint32_t screenbuffer[GDISP_SCREEN_HEIGHT*GDISP_SCREEN_WIDTH/(EINK_PPB*4)] // 800*600 screen with 4 pixels per byte and 4 byte per uint32_t makes 30000 elements

/** Delay between signal changes, to give time for IO pins to change state. */
// RP2040 runs at 125 or 133 MHz or so, and the -sc7 e-ink is slower then the -sc 4, requiring Tle_off of 200 ns min where sc-4 has 40 min.
// at 133 Mhz, 7.5 ns per cycle. The NOPS as used here originally seemed not to work, but the SDK provides a "busy_wait_at_least_cycles" function so use that instead
static inline void clockdelay()
{

busy_wait_at_least_cycles(EINK_CLOCKDELAY);

}

/** Fast vertical clock pulse for gate driver, used during initializations */
static void vclock_quick()
{
    setpin_ckv(TRUE);
    eink_delay(1); 
    setpin_ckv(FALSE);
    eink_delay(4); 
}

/** Horizontal clock pulse for clocking data into source driver */
static void hclock()
{
    clockdelay();
    setpin_cl(TRUE);
    clockdelay();
    setpin_cl(FALSE);
}

/** Start a new vertical gate driver scan from top.
 * Note: Does not clear any previous bits in the shift register,
 *       so you should always scan through the whole display before
 *       starting a new scan.
 */
void vscan_start()
{
    setpin_gmode(TRUE);
    vclock_quick();
    setpin_spv(FALSE);
    vclock_quick();
    // for ed060SC7      VV
    setpin_ckv(TRUE); // instead of inverting CKV or always leaving it high, it suffices to raise it here. The next vclk_quick will lower it again. I don't understand why this is needed, but it works.
    // ed060sc7          ^^ 
    setpin_spv(TRUE);
    vclock_quick();
}

/** Waveform for strobing a row of data onto the display.
 * Attempts to minimize the leaking of color to other rows by having
 * a long idle period after a medium-length strobe period.
 */
void vscan_write()
{
    setpin_ckv(TRUE);
    setpin_oe(TRUE);
    eink_delay(5);
    setpin_oe(FALSE);
    setpin_ckv(FALSE);
    eink_delay(200);
}

/** Waveform used when clearing the display. Strobes a row of data to the
 * screen, but does not mind some of it leaking to other rows.
 */
void vscan_bulkwrite()
{
    setpin_ckv(TRUE);
    eink_delay(20);
    setpin_ckv(FALSE);
    eink_delay(200);
}

/** Waveform for skipping a vertical row without writing anything.
 * Attempts to minimize the amount of change in any row.
 */
void vscan_skip()
{
    setpin_ckv(TRUE);
    eink_delay(1);
    setpin_ckv(FALSE);
    eink_delay(100);
}

/** Stop the vertical scan. The significance of this escapes me, but it seems
 * necessary or the next vertical scan may be corrupted.
 */
void vscan_stop()
{
    setpin_gmode(FALSE);
    vclock_quick();
    vclock_quick();
    vclock_quick();
    vclock_quick();
    vclock_quick();
}

/** Start updating the source driver data (from left to right). */
void hscan_start()
{
    /* Disable latching and output enable while we are modifying the row. */
    setpin_le(FALSE);
    setpin_oe(FALSE);
    
    /* The start pulse should remain low for the duration of the row. */
    setpin_sph(FALSE);
}

/** Write data to the horizontal row. */
void hscan_write(const uint8_t *data, int count)
{
    while (count--)
    {
        /* Set the next byte on the data pins */
        setpins_data(*data++);
        
        /* Give a clock pulse to the shift register */
        hclock();
    }
}

/** Finish and transfer the row to the source drivers.
 * Does not set the output enable, so the drivers are not yet active. */
void hscan_stop()
{
    /* End the scan */
    setpin_sph(TRUE);
    hclock();
    
    /* Latch the new data */
    setpin_le(TRUE);
    clockdelay();
    setpin_le(FALSE);
}

/** Turn on the power to the E-Ink panel, observing proper power sequencing. */
void EPD_power_on()
{
    unsigned i;
    
    /* First the digital power supply and signal levels. */
    setpower_vdd(TRUE);
    setpin_le(FALSE);
    setpin_oe(FALSE);
    setpin_cl(FALSE);
    setpin_sph(TRUE);
    setpins_data(0);
    setpin_ckv(FALSE);
    setpin_gmode(FALSE);
    setpin_spv(TRUE);
    
    /* Min. 100 microsecond delay after digital supply */
    sleep_ms(100); // TODO: eliminate busy waits
    
    /* Then negative voltages and min. 1000 microsecond delay. */
    setpower_vneg(TRUE);
    sleep_ms(1000); // TODO: eliminate busy waits
    
    /* Finally the positive voltages. */
    setpower_vpos(TRUE);
    
    /* Clear the vscan shift register */
    vscan_start();
    for (i = 0; i < GDISP_SCREEN_HEIGHT; i++)
        vclock_quick();
    vscan_stop();
}

/** Turn off the power, observing proper power sequencing. */
void EPD_power_off()
{
    /* First the high voltages */
    setpower_vpos(FALSE);
    setpower_vneg(FALSE);
    
    /* Wait for any capacitors to drain */
    sleep_ms(100); // TODO: eliminate busy waits
    
    /* Then put all signals and digital supply to ground. */
    setpin_le(FALSE);
    setpin_oe(FALSE);
    setpin_cl(FALSE);
    setpin_sph(FALSE);
    setpins_data(0);
    setpin_ckv(FALSE);
    setpin_gmode(FALSE);
    setpin_spv(FALSE);
    setpower_vdd(FALSE);
}

/* ====================================
 *      Framebuffer emulation layer
 * ==================================== */

#if EINK_PPB == 4
#define PIXELMASK 3
#define PIXEL_WHITE 2
#define PIXEL_BLACK 1
#define BYTE_WHITE 0xAA
#define BYTE_BLACK 0x55
#else
#error Unsupported EINK_PPB value.
#endif

#if GDISP_SCREEN_HEIGHT % EINK_BLOCKHEIGHT != 0
#error GDISP_SCREEN_HEIGHT must be evenly divisible by EINK_BLOCKHEIGHT
#endif

#if GDISP_SCREEN_WIDTH % EINK_BLOCKWIDTH != 0
#error GDISP_SCREEN_WIDTH must be evenly divisible by EINK_BLOCKWIDTH
#endif

#if EINK_BLOCKWIDTH % EINK_PPB != 0
#error EINK_BLOCKWIDTH must be evenly divisible by EINK_PPB
#endif


/** Write out a block row. */
static void write_block_row(unsigned by)
{
    unsigned bx, dy, dx;
    for (dy = 0; dy < EINK_BLOCKHEIGHT; dy++)
    {
        hscan_start();
        for (bx = 0; bx < BLOCKS_X; bx++)
        {
            if (g_blockmap[by][bx] == 0)
            {
                for (dx = 0; dx < WIDTH_BYTES; dx++)
                {
                    const uint8_t dummy = 0;
                    hscan_write(&dummy, 1);
                }
            }
            else
            {
                block_t *block = &g_blocks[g_blockmap[by][bx] - 1];
                hscan_write(&block->data[dy][0], WIDTH_BYTES);
            }
        }
        hscan_stop();
        
        vscan_write();
    }
}


/** write buffer to display. */
void screenrefresh()
{
    unsigned by, dy, i;
    
    for (i = 0; i < EINK_WRITECOUNT; i++)
    {
        vscan_start();
        
        for (y = 0; y < GDISP_SCREEN_HEIGHT; y++)
        {
            /* Write out the blocks. */
            write_block_row(y);
        }
        
        vscan_stop();
    }
}



/* ===============================
 *         Public functions
 * =============================== */

bool_t gdisp_lld_init(void)
{
    init_board();
    
    /* Make sure that all the pins are in "off" state.
     * Having any pin high could cause voltage leaking to the
     * display, which in turn causes the image to leak slowly away.
     */
    EPD_power_off();

    //TODO: iets dat de buffer wist (memcpy)?
    
    return TRUE;
}

void gdisp_lld_draw_pixel(coord_t x, coord_t y, color_t color)
{ // todo: perhaps test this with a diagonal line or something, but it needs testing and might contain bugs
    uint32_t word;
    uint8_t bitpos;
    
    if (x < 0 || x >= GDISP_SCREEN_WIDTH || y < 0 || y >= GDISP_SCREEN_HEIGHT)
        return;
     
    bitpos = (30 - 2 * (x % EINK_PPB*4));
    word = displaybuffer[y][x / (EINK_PPB*4)];
    word &= ~(PIXELMASK << bitpos);
    if (color)
    {
        word |= PIXEL_WHITE << bitpos;
    }
    else
    {
        word |= PIXEL_BLACK << bitpos;   
    }
    displaybuffer[y][x / (EINK_PPB*4)] = word;
}

//todo: something that clears the buffer and/or something that erases bits?


/* ===============================
 *       Accelerated routines
 * =============================== */

//#if GDISP_HARDWARE_CLEARS

static void subclear(color_t color)
{
    unsigned x, y;
    uint8_t byte;
    
    hscan_start();
    byte = color ? BYTE_WHITE : BYTE_BLACK;
    for (x = 0; x < GDISP_SCREEN_WIDTH; x++)
    {
        hscan_write(&byte, 1);
    }
    hscan_stop();
    
    setpin_oe(TRUE);
    vscan_start();
    for (y = 0; y < GDISP_SCREEN_HEIGHT; y++)
    {
        vscan_bulkwrite();
    }
    vscan_stop();
    setpin_oe(FALSE);
}

void gdisp_lld_clear(color_t color)
{
    unsigned i;
    clear_block_map();
    
    if (EINK_BLINKCLEAR)
    {
        subclear(!color);
       sleep_ms(50); // TODO: eliminate busy waits
    }
    
    for (i = 0; i < EINK_CLEARCOUNT; i++)
    {
        subclear(color);
        sleep_ms(10); // TODO: eliminate busy waits
    }
    
}
//#endif

//#endif
