/// just so these functions can be called from ... wherever this is included
#ifndef GDISP_LDD_ONCE
#define GDISP_LDD_ONCE


/* =================================
 *      configuration
 * ================================= */

#ifndef GDISP_SCREEN_HEIGHT
#       define GDISP_SCREEN_HEIGHT 600 // Y was 600, but -SC7 is 3:4 where -SC4 is 4:3
#endif

#ifndef GDISP_SCREEN_WIDTH
#       define GDISP_SCREEN_WIDTH 800 //X was 800, but -SC7 is 3:4 where -SC4 is 4:3 // perhaps rotation is not done here?
#endif

#define INVERT_X FALSE
#define INVERT_Y FALSE

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
#       define EINK_CLEARCOUNT 10 // todo: maybe this can be less?
#endif

/* Number of passes to use when writing to the display */
#ifndef EINK_WRITECOUNT
#       define EINK_WRITECOUNT 4
#endif


/* added glue */
#include <stdbool.h>
#include "pico/stdlib.h"
typedef enum{BLACK,WHITE,NOCHANGE11,NOCHANGE00} color_t;
typedef bool bool_t;
typedef int coord_t;
extern union screenbuffer{
    uint32_t sb_words[GDISP_SCREEN_HEIGHT][GDISP_SCREEN_WIDTH/(EINK_PPB*4)]; // 800*600 screen with 4 pixels per byte and 4 byte per uint32_t makes 30000 elements
    uint8_t sb_bytes[GDISP_SCREEN_HEIGHT][GDISP_SCREEN_WIDTH/EINK_PPB]; // for byte-acces to the same buffer
    } displaydata;
/*/glue*/

void vscan_start();
void vscan_write();
void vscan_bulkwrite();
void vscan_skip();
void vscan_stop();
void hscan_start();
void hscan_write();
void hscan_stop();
void EPD_power_on();
void EPD_power_off();

void screenrefresh();
void clear_screenbuffer(color_t color);

/* ===============================
 *         Public functions
 * =============================== */

bool_t gdisp_lld_init(void);
void gdisp_lld_draw_pixel(coord_t x, coord_t y, color_t color);

/* ===============================
 *       Accelerated routines
 * =============================== */


static void subclear(color_t color);
void gdisp_lld_clear(color_t color);

#endif
