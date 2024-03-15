#ifndef __config_H
#define __config_H

#include "arduino.h"
#include "config.h"

typedef enum heat_op_{
  OPEN,
  CLOSE,
  NONE,
}heat_op;

typedef struct WHCB_config_{
  bool config; /* 配置标志 */
  uint16_t obj_time_max;
  uint16_t time_dpi;
  uint16_t time_remaining_m; /* 剩余时间 单位分钟*/
  uint16_t time_remaining_s; /* 剩余时间 单位秒*/
  uint16_t obj_time; /* 目标时间 单位分钟*/
  bool lcd_blk_en; /* 背光标志 */
  bool heat; /* 加热标志 */
  uint8_t set_heat; /* 加热操作 */
}WHCB_config;

#endif
