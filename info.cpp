#include "info.h"

hw_timer_t* timer1= NULL;  /* 实例化一个定时器 */

info info1={INFO_NONE};
extern WHCB_config config1;

void info_task(void *parameter);

void info_init(void)
{
  xTaskCreatePinnedToCore(
              info_task, /* 任务函数名称 */
              "info_task", /* 函数名字 */
              2000, /* 函数堆栈 word为单位*/
              NULL, /* 任务参数 */
              1, /* 任务优先级 */
              NULL, /* 任务句柄 */
              1); /* 任务运行核心 */
}

void updateShow(void)
{
  switch(info1.type_)
  {
    case INFO_NONE:
      break;
    case INFO_HEAT:
      WHCB_status_show();
      info1.type_=INFO_NONE;
      break;
    case INFO_KEY:
      key_info_print(1);
      delay(3000);
      key_info_print(0);
      info1.type_=INFO_NONE;
      break;
    default:
      Serial.println("default");
      break;
  }
}

void info_task(void *parameter)
{
  while(1)
  {
    delay(100);
    updateShow();
    if(Serial.available()==1)
    {
      config1.set_heat = Serial.read();
      Serial.println(config1.set_heat);
    }
  }
  vTaskDelete(NULL);
}
