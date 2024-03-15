#ifndef __LCD_H
#define __LCD_H

#include "arduino.h"
#include <Adafruit_ST7789.h>

#define LCD_CS_PIN      13
#define LCD_DC_PIN      12
#define LCD_MOSI_PIN    15
#define LCD_SCLK_PIN    16
#define LCD_RST_PIN     14
//#define LCD_TE_PIN      15
#define LCD_BLK_PIN     11

void lcd_init(void);
void lcd_blk_control(bool status);
void lcd_printf(const char *str,uint8_t s,uint16_t color,uint8_t x,uint8_t y);

#endif
