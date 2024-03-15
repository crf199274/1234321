#include "heat.h"

WHCB_config config1; /* 加热配置 */
hw_timer_t* timer0= NULL;  /* 实例化一个定时器 */

extern info info1;

void heat_task(void *parameter); /* 加热任务 */
static void timer_init(uint64_t time); /* 定时器初始化 */
void IRAM_ATTR heat_timer(void); /* 加热定时 */
void WHCB_status_show(void); /* 打印状态信息 */

void heat_init(void)
{
  /* 初始配置 */
  config1.config = 0; /* 配置失能 */
  config1.obj_time_max = 120;
  config1.time_dpi = 4096/120;
  config1.time_remaining_m = 0; /* 剩余时间 单位m */
  config1.time_remaining_s = 0; /* 剩余时间 单位s */
  config1.obj_time = 0; /* 目标时间 单位m */
  config1.lcd_blk_en = 1; /* 背光状态 */
  config1.heat = false; /* 加热状态 */
  config1.set_heat = NONE; /* 加热操作 */

  info1.type_ = INFO_HEAT;
  
  pinMode(HEAT_PIN,OUTPUT); /* 配置加热管脚为输出 */
  digitalWrite(HEAT_PIN,LOW); /* 关闭加热 */

  timer_init(1000000ul); /* 初始化定时器 中断间隔1s*/
  
  xTaskCreatePinnedToCore(
              heat_task, /* 加热任务 */
              "heat_task", /* 任务名字 */
              2000, /* 堆栈大小 word为单位 */
              NULL, /* 任务参数 */
              1, /* 任务优先级 */
              NULL, /* 任务句柄 */
              1); /* 任务运行核心 */
}

static void heat(bool status)
{
  if(status)
  {
    digitalWrite(HEAT_PIN,HIGH); /* 关闭加热 */
  }else {
    digitalWrite(HEAT_PIN,LOW); /* 开启加热 */
  }
}

void heat_task(void *parameter)
{
  while(1)
  {
    delay(100);
    if(!config1.config)
    {
      if((config1.set_heat==OPEN)&&(config1.heat==false)) /* 符合加热条件 触发加热 */
      {
        Serial.println("heat(true);");
        heat(true);
        config1.time_remaining_s = config1.obj_time*60;
        config1.time_remaining_m = config1.obj_time;
        timerAlarmEnable(timer0);  /* 打开定时器 */
        config1.set_heat = NONE; /* 把操作标志清空 */
        config1.heat = true; /* 更新加热标志 */
        info1.type_=INFO_HEAT;
      }else if((config1.set_heat==CLOSE)&&config1.heat){
        Serial.println("heat(false);");
        heat(false);
        timerAlarmDisable(timer0);  /* 停止定时器 */
        config1.set_heat = NONE; /* 把操作标志清空 */
        config1.heat = false; /* 更新加热标志 */
        info1.type_=INFO_HEAT;
      }else {
        /* 无作为 */
        config1.set_heat = NONE; /* 把操作标志清空 */
      }
    }
  }
  vTaskDelete(NULL);
}

void WHCB_status_show(void)
{
  char text_[20];

  /* 显示目标时间 */
  memset(text_,' ',sizeof(text_));
  sprintf(text_,"OT: %3d",config1.obj_time);
  lcd_printf(text_,2,ST77XX_BLUE,0,0);

  /* 显示剩余时间 */
  memset(text_,' ',sizeof(text_));
  sprintf(text_,"RT: %3d",config1.time_remaining_m);
  lcd_printf(text_,2,ST77XX_GREEN,0,20);

  /* 显示加热 */
  if(config1.heat)
    lcd_printf("Status: RUN ",2,ST77XX_YELLOW,0,40);
  else
    lcd_printf("Status: STOP",2,ST77XX_YELLOW,0,40);

  if(config1.config)
    lcd_printf("Config",2,ST77XX_RED,0,60);
  else
    lcd_printf("Config",2,ST77XX_BLACK,0,60);
}

void IRAM_ATTR heat_timer(void)
{
  if(config1.time_remaining_s)
  {
    config1.time_remaining_s--;
    config1.time_remaining_m = config1.time_remaining_s/60;
    info1.type_=INFO_HEAT;
  }else {
    timerAlarmDisable(timer0);  /* 停止定时器 */
    config1.set_heat = CLOSE; /* 关闭加热 */
  }
}

static void timer_init(uint64_t time)
{
  timer0 = timerBegin(0, 80, true);  //设置定时器1，80分频，定时器是否上下计数
  timerAttachInterrupt(timer0, heat_timer, true);  //定时器地址指针，中断函数名称，中断边沿触发类型
  timerAlarmWrite(timer0, time , true);  //操作那个定时器，定时时长单位us，是否自动重装载
}
