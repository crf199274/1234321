#include "RUN_LED.h"

void run_led_task(void *parameter);

void run_led_init(void)
{
  pinMode(RUN_LED_PIN,OUTPUT); /* 配置GPIO为输出 */
  digitalWrite(RUN_LED_PIN,HIGH);
  
  xTaskCreatePinnedToCore(
            run_led_task, /* 函数名称 */
            "run_led_task", /* 任务名字 */
            2000, /* 堆栈大小 word为单位 */
            NULL, /* 任务参数 */
            1, /* 任务优先级 */
            NULL, /* 任务句柄 */
            1); /* 任务运行的核心 */
}

void run_led_task(void *parameter)
{
  while(1)
  {
    digitalWrite(RUN_LED_PIN,LOW);
    delay(500);
    digitalWrite(RUN_LED_PIN,HIGH);
    delay(500);
  }
  vTaskDelete(NULL);
}
