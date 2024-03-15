#ifndef __heat_H
#define __heat_H

#include "arduino.h"
#include "config.h"
#include "string.h"
#include "LCD.h"
#include "info.h"

#define HEAT_PIN  13

void heat_init(void);
void WHCB_status_show(void);

#endif
