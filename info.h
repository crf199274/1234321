#ifndef __info_H
#define __info_H

#include "arduino.h"
#include "heat.h"
#include "key.h"

typedef enum infoType_{
  INFO_HEAT,
  INFO_KEY,
  INFO_NONE
}infoType;

typedef struct info_{
  uint8_t type_;
//  bool CWarn;
}info;

void info_init(void);
void updateShow(void);

#endif
