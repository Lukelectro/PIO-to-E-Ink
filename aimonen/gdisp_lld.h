/// just so these functions can be called from ... wherever this is included
#ifndef GDISP_LDD_ONCE
#define GDISP_LDD_ONCE
// all sorts of config still in the .c file

#define GDISP_NEED_CONTROL 1 // so it does not complain. TODO: maybe just remove

/* added glue */
#include <stdbool.h>
typedef bool color_t;
typedef bool bool_t;
typedef unsigned int coord_t;
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

void flush_buffers();

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
