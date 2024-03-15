#include "rotary_knob.h"

extern WHCB_config config1;

void rk_init(void)
{
  pinMode(RK_PIN,INPUT); /* 设置旋钮的GPIO为输入 */
  analogReadResolution(12);  /* 设置ADC的分辨率为12位(0-4096) */
}

void get_rkValue(void)
{
  uint16_t adc_value;
  adc_value = analogRead(RK_PIN);
  config1.obj_time = adc_value/config1.time_dpi;
}
