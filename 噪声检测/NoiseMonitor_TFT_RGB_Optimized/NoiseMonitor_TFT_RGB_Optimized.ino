#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial softSerial(5, 6);

#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9
#define TFT_BL     7
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define RGB_R      12
#define RGB_G      4
#define RGB_B      2

static const unsigned char PROGMEM oled_st7735_STHeiti_16px_u5206_u8d1d[] = {
  0x0E,0x60,0x30,0x0C,0x00,0x00,0x1C,0x70,0x31,0x8C,0x00,0x00,0x38,0x38,0x31,0x8C,0x00,0x00,
  0x78,0x1C,0x31,0x8C,0x00,0x00,0xF0,0x0E,0x31,0x8C,0x00,0x00,0xC0,0x07,0xB1,0x8C,0x00,0x00,
  0x7F,0xFC,0x31,0x8C,0x00,0x00,0x0E,0x18,0x33,0x8C,0x00,0x00,0x0E,0x18,0x33,0x8C,0x00,0x00,
  0x0E,0x18,0x33,0xCC,0x00,0x00,0x1C,0x38,0x07,0xF0,0x00,0x00,0x38,0x38,0x0E,0x38,0x00,0x00,
  0xE3,0xF0,0x38,0x1E,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

char  data1[8]={0x02,0x03,0x00,0x00,0x00,0x01,0x84,0x39};

byte buffer[7];
int shuju = 0;
int lastShuju = -1; // 保存上一次的分贝值，用于判断是否需要刷新
int tempData = 0;
int lastTempData = -1; // 保存上一次的温度值，用于判断是否需要刷新
char tempBuffer[3];
unsigned long lastRefreshTime = 0; 
const unsigned long refreshInterval = 500; // 刷新间隔改为500ms（避免刷新太频繁）

void chuankoufasong(uint8_t *buf, uint8_t len)
{
  uint8_t t;
  for(t=0;t<len;t++)
  {
    softSerial.write(buf[t]);
  }
}

void RGB_set(String color)
{
  if(color == "red")
  {
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
  }
  else if(color == "green")
  {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
  }
  else if(color == "blue")
  {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
  }
}

// 局部刷新数字的函数：先清指定区域，再写新数字
void updateNumber(int x, int y, int oldNum, int newNum, uint16_t textColor, uint16_t bgColor, int textSize) {
  // 计算数字的显示宽度（textSize=2时，每个数字约16像素宽，预留4位数字空间）
  int numWidth = 4 * 16 * textSize / 2; 
  int numHeight = 12 * textSize;
  
  // 1. 清空旧数字区域（只清数字部分，不清标题）
  tft.fillRect(x, y, numWidth, numHeight, bgColor);
  // 2. 设置字体样式
  tft.setTextColor(textColor);
  tft.setTextSize(textSize);
  // 3. 写入新数字
  tft.setCursor(x, y);
  tft.print(newNum);
}

void setup(){
  Serial.begin(115200);
  softSerial.begin(9600);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);

  // 初始化显示固定标题（只显示一次，后续不刷新）
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(2);       
  tft.setCursor(10, 30);    
  tft.print("SPL: ");       // 分贝标题（固定）
  tft.setCursor(100, 30);   // 预留数字位置，先写初始值
  tft.print(shuju);
  tft.print("db");

  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 60);    
  tft.print("TEMP: ");      // 温度标题（固定）
  tft.setCursor(100, 60);   // 预留数字位置，先写初始值
  tft.print(tempData);
  tft.print("C");
}

void loop(){
  // 1. 读取分贝传感器数据
  chuankoufasong((uint8_t*)data1,8);
  delay(1000);
  if(softSerial.available() >= 7){
    softSerial.readBytes(buffer,7);
    shuju=(buffer[3]*256+buffer[4])/10;
  }

  Serial.println(shuju);

  // 2. 读取硬件串口温度数据
  if(Serial.available() >= 2) {
    Serial.readBytes(tempBuffer, 2);
    tempBuffer[2] = '\0';
    tempData = atoi(tempBuffer);
  }

  // 3. 控制RGB灯
  if(shuju <= 70)
    RGB_set("green");
  else if(shuju <= 100)
    RGB_set("blue");
  else
    RGB_set("red");

  // 4. 局部刷新屏幕（仅当数值变化且达到刷新间隔时执行）
  if(millis() - lastRefreshTime > refreshInterval){
    lastRefreshTime = millis();
    
    // 仅当分贝值变化时，刷新分贝数字区域
    if(shuju != lastShuju){
      updateNumber(70, 30, lastShuju, shuju, ST7735_GREEN, ST7735_BLACK, 2);
      // 刷新单位（可选，若数字长度变化可能遮挡单位，这里固定补回db）
      tft.setCursor(110, 30);
      tft.print("db");
      lastShuju = shuju; // 更新上一次的值
    }

    // 仅当温度值变化时，刷新温度数字区域
    if(tempData != lastTempData){
      updateNumber(80, 60, lastTempData, tempData, ST7735_CYAN, ST7735_BLACK, 2);
      // 刷新单位
      tft.setCursor(120, 60);
      tft.print("C");
      lastTempData = tempData; // 更新上一次的值
    }
  }
}