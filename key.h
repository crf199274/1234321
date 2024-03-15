#ifndef __KEY_H
#define __KEY_H

#include "arduino.h"
#include "config.h"
#include "LCD.h"
#include "rotary_knob.h" 
#include "info.h"

#define WIN_W     120
#define WIN_H     120

#define KEY_CONFIG  14
#define KEY_LCD_BLK 12

void key_init(void);
void key_info_print(bool en);

#endif
