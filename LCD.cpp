#include "LCD.h"

Adafruit_ST7789 adafruit = Adafruit_ST7789(LCD_CS_PIN,LCD_DC_PIN,LCD_MOSI_PIN,LCD_SCLK_PIN,LCD_RST_PIN); /* 初始化SPI引脚 */;

void lcd_blk_control(bool status); /* 背光控制函数 */

void lcd_init(void)
{
  /* 初始化LCD引脚 */
  pinMode(LCD_BLK_PIN,OUTPUT); /* 配置背光引脚为输出 */

  lcd_blk_control(true); /* 打开背光 */
  
  adafruit.init(240,240); /* 初始LCD */
//  adafruit.setRotation(0) /* 设备旋转角度 */

  adafruit.fillScreen(ST77XX_BLACK); /* 填充黑色 */
}

void lcd_printf(const char *str,uint8_t s,uint16_t color,uint8_t x,uint8_t y)
{
  adafruit.setTextWrap(false); /* 不自动换行 */
  adafruit.setTextSize(s); /* 设置字号 */
  adafruit.setTextColor(color,ST77XX_BLACK);
  adafruit.setCursor(x,y);
  adafruit.println(str);
}

void lcd_blk_control(bool status)
{
  if(status)
    digitalWrite(LCD_BLK_PIN,HIGH);
  else
    digitalWrite(LCD_BLK_PIN,LOW);
}

//void SerialPrintColor(void)
//{
//  if(Serial.available()==2)
//  {
//    adafruit.fillScreen(((Serial.read()<<8)|Serial.read())); /* 填充颜色 */
//  }
//}
