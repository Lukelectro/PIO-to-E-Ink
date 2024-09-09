/*
 * 2013 Petteri Aimonen <jpa@gfx.mail.kapsi.fi>
 * This file is released to the public domain.
 */

/* Board interface definitions for ED060SC4 PrimeView E-ink panel.
 *
 * You should implement the following functions to define the interface to
 * the panel on your board.
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include <stdio.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include <stdbool.h>

/*
 * IO pins assignments.
 */
#define EINK_VDD      8 // always on or share pin with SMPS_CTRL
#define EINK_GMODE    6
#define EINK_SPV      5
#define EINK_CKV      10
#define EINK_CL       11
#define EINK_LE       2
#define EINK_OE       3
#define EINK_SPH      4
#define EINK_D0       14 // must be consecutive with D0 on lowest bit
#define EINK_D1       15
#define EINK_D2       16
#define EINK_D3       17
#define EINK_D4       18
#define EINK_D5       19
#define EINK_D6       20
#define EINK_D7       21
#define SMPS_CTRL     8
#define VPOS_CTRL     9
#define VNEG_CTRL     7

/* Set up IO pins for the panel connection. */
static inline void init_board(void) {

 /* set pin functions to gpio, set them to output, etc*/
gpio_init_mask(1<<EINK_CL|1<<EINK_LE|1<<EINK_OE|1<<EINK_SPH|1<<EINK_GMODE|1<<EINK_SPV|1<<EINK_CKV|1<<SMPS_CTRL|1<<EINK_VDD|1<<VPOS_CTRL|1<<VNEG_CTRL|0xFF<<EINK_D0);
gpio_set_dir_out_masked(1<<EINK_CL|1<<EINK_LE|1<<EINK_OE|1<<EINK_SPH|1<<EINK_GMODE|1<<EINK_SPV|1<<EINK_CKV|1<<SMPS_CTRL||1<<EINK_VDD|1<<VPOS_CTRL|1<<VNEG_CTRL|0xFF<<EINK_D0);
// TODO: set pin function to SIO, which is default, but considering the plan to also use PIO to drive the display...

}

/* Delay for display waveforms. Should be an accurate microsecond delay. */
static void eink_delay(int us)
{
sleep_us(us);
}

/* Turn the E-ink panel Vdd supply (+3.3V) on or off. */
static inline void setpower_vdd(bool_t on) {
gpio_put(EINK_VDD,on);
gpio_put(SMPS_CTRL,on);
}

/* Turn the E-ink panel negative supplies (-15V, -20V) on or off. */
static inline void setpower_vneg(bool_t on) {
gpio_put(VNEG_CTRL,on);
}

/* Turn the E-ink panel positive supplies (-15V, -20V) on or off. */
static inline void setpower_vpos(bool_t on) {
gpio_put(VPOS_CTRL,on);
}

/* Set the state of the LE (source driver Latch Enable) pin. */
static inline void setpin_le(bool_t on) {
gpio_put(EINK_LE,on);
}

/* Set the state of the OE (source driver Output Enable) pin. */
static inline void setpin_oe(bool_t on) {
gpio_put(EINK_OE,on);
}

/* Set the state of the CL (source driver Clock) pin. */
static inline void setpin_cl(bool_t on) {
gpio_put(EINK_CL,on);
}

/* Set the state of the SPH (source driver Start Pulse Horizontal) pin. */
static inline void setpin_sph(bool_t on) {
gpio_put(EINK_SPH,on);
}

/* Set the state of the D0-D7 (source driver Data) pins. */
static inline void setpins_data(uint8_t value) {
gpio_put_masked(0xFF<<EINK_D0,value<<EINK_D0);
}

/* Set the state of the CKV (gate driver Clock Vertical) pin. */
static inline void setpin_ckv(bool_t on) {
gpio_put(EINK_CKV,on);
}

/* Set the state of the GMODE (gate driver Gate Mode) pin. */
static inline void setpin_gmode(bool_t on) {
gpio_put(EINK_GMODE,on);
}

/* Set the state of the SPV (gate driver Start Pulse Vertical) pin. */
static inline void setpin_spv(bool_t on) {
gpio_put(EINK_SPV,on);
}

#endif
