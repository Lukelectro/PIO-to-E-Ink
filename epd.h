/*******************************************************************************
  EPD DRIVER FOR STM32F2/4 w/ FSMC
  By ZephRay(zephray@outlook.com)
  Modified for RP2040 by Luke www.eluke.nl
*******************************************************************************/
#ifndef __EPD_H__
#define __EPD_H__

#include "hardware/gpio.h"

//Use new ED060SC4 H1/H2 screens
//if you are using ED060SC4 without any postfix, comment this
//#define USE_H_SCREEN

extern unsigned char EPD_FB[60000];


#define PulseDelay()    {Delay_Us(1);} // todo: might need to delay on RP2040. Preferably with a timer/sleep/rtos-compatible something. RP2040 System timer? (us tick)

//SOURCE DRIVER
#define EPD_CL_L()        {gpio_put(EPD_CL, 0);  PulseDelay();}
#define EPD_CL_H()        {gpio_put(EPD_CL, 1);  PulseDelay();}
#define EPD_LE_L()        {gpio_put(EPD_LE, 0);  PulseDelay();}
#define EPD_LE_H()        {gpio_put(EPD_LE, 1);  PulseDelay();}
#define EPD_OE_L()        {gpio_put(EPD_OE, 0);  PulseDelay();}
#define EPD_OE_H()        {gpio_put(EPD_OE, 1);  PulseDelay();}
//no shr
#define EPD_SPH_L()       {gpio_put(EPD_SPH,0); PulseDelay();}
#define EPD_SPH_H()       {gpio_put(EPD_SPH,1); PulseDelay();}

//GATE DRIVER
#define EPD_GMODE1_L()    {gpio_put(EPD_GMODE1,0); PulseDelay();}
#define EPD_GMODE1_H()    {gpio_put(EPD_GMODE1,1); PulseDelay();}
// no gmode2, no XRL
#define EPD_SPV_L()       {gpio_put(EPD_SPV,0); PulseDelay();}
#define EPD_SPV_H()       {gpio_put(EPD_SPV,1); PulseDelay();}
#define EPD_CKV_L()       {gpio_put(EPD_CKV,0); PulseDelay();}
#define EPD_CKV_H()       {gpio_put(EPD_CKV,1); PulseDelay();}

//added for pwr on/of controll
#define EPD_PWR_CTRL_H()   {gpio_put(EPD_PWR_CTRL,1); PulseDelay();}
#define EPD_VNEG_H()       {gpio_put(EPD_VNEG_EN,1); PulseDelay();}
#define EPD_VPOS_H()       {gpio_put(EPD_VPOS_EN,1); PulseDelay();}
#define EPD_PWR_CTRL_L()   {gpio_put(EPD_PWR_CTRL,0); PulseDelay();}
#define EPD_VNEG_L()       {gpio_put(EPD_VNEG_EN,0); PulseDelay();}
#define EPD_VPOS_L()       {gpio_put(EPD_VPOS_EN,0); PulseDelay();}

/* and because u8 seems undefined:*/
typedef unsigned char u8;

#define ROTATE /* to swap X an Y */

void EPD_Init(void);
void EPD_GPIO_Init(void);
void EPD_Power_Off(void);
void EPD_Power_On(void);
void EPD_Clear(void);
void EPD_PrepareWaveform(void);
void EPD_DispPic();
void EPD_DispScr(unsigned int startLine, unsigned int lineCount);
void EPD_ClearFB(unsigned char c);
void EPD_SetPixel(unsigned short x,unsigned short y,unsigned char color);
void EPD_Line(unsigned short x0,unsigned short y0,unsigned short x1,unsigned short y1,unsigned short color);
void EPD_PutChar_16(unsigned short x, unsigned short y, unsigned short chr, unsigned char color);
void EPD_PutChar_Legacy(unsigned short x, unsigned short y, unsigned short chr, unsigned char color);
void EPD_String_16(unsigned short x,unsigned short y,unsigned char *s,unsigned char color);
void EPD_String_24(unsigned short x,unsigned short y,unsigned char *s,unsigned char color);
void EPD_FillRect(unsigned short x1,unsigned short y1,unsigned short x2,unsigned short y2,unsigned char color);

#endif
