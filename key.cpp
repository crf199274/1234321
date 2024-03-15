#include "key.h"

extern Adafruit_ST7789 adafruit;
extern WHCB_config config1;

extern info info1;

void IRAM_ATTR key_callback(void);
void key_task(void *parameter);

void key_init(void)
{
  pinMode(KEY_CONFIG,INPUT); /* 配置GPIO为输入 */
  attachInterrupt(KEY_CONFIG,key_callback,FALLING); /* 外部中断 */
  pinMode(KEY_LCD_BLK,INPUT);
  attachInterrupt(KEY_LCD_BLK,key_callback,FALLING); /* 外部中断 */

  xTaskCreatePinnedToCore(
            key_task, /* 任务函数 */
            "key_task", /* 任务名字 */
            2000, /* 堆栈大小 word为单位*/
            NULL, /* 任务参数 */
            1, /* 任务优先级 */
            NULL, /* 任务句柄 */
            1); /* 任务运行核心 */
  
}

/* 中断回调函数 */
void key_callback(void)
{
  if(digitalRead(KEY_CONFIG)==LOW)
  {
    info1.type_=INFO_HEAT;
    if(config1.config)
    {
      config1.config = false; /* 设为配置状态 */
    }else {
      if(config1.heat==false)
        config1.config = true;
      else
      {
        if(info1.type_ != INFO_KEY)
          info1.type_=INFO_KEY;
      }
    }
  }else if(digitalRead(KEY_LCD_BLK)==LOW) {
    if(config1.lcd_blk_en)
    {
      config1.lcd_blk_en = false;
      lcd_blk_control(config1.lcd_blk_en);
    }else {
      config1.lcd_blk_en = true;
      lcd_blk_control(config1.lcd_blk_en);
    }
  }else {
    /* 无操作 */
  }
}

void key_task(void *parameter)
{
  while(1)
  {
    delay(100);
    if(config1.config == true) {
      /* 读取目标值 */
      get_rkValue();
      info1.type_=INFO_HEAT;
    }else {
      /* 无操作 */
    }
  }
  vTaskDelete(NULL);
}

void key_info_print(bool en)
{
  char text_[20];

  if(en)
    adafruit.drawRect((240-WIN_W)/2,(240-WIN_H)/2,WIN_W,WIN_H,ST77XX_WHITE); /* 弹窗绘制 */
  else
    adafruit.drawRect((240-WIN_W)/2,(240-WIN_H)/2,WIN_W,WIN_H,ST77XX_BLACK); /* 弹窗绘制 */

  /* 显示目标时间 */
  memset(text_,' ',sizeof(text_));
  sprintf(text_,"Warning !");
  if(en)
    lcd_printf(text_,2,ST77XX_RED,(240-9*12)/2,(240-16)/2);
  else
    lcd_printf(text_,2,ST77XX_BLACK,(240-9*12)/2,(240-16)/2);
}
