/**********************************************************************************
  无串口打印版
  功能：OLED显示 + 健康模块 + MQ2烟雾 + 光敏传感器
  接线：
  健康模块 TX→D2  RX→D3
  MQ2 → A0
  光敏 → A1
  OLED → A4 A5
**********************************************************************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial healthSerial(2, 3); // RX=2, TX=3

byte buffer[128];

int heartrate = 75;
int xueyang   = 98;
int high_ya   = 120;
int low_ya    = 80;

int smokeVal;
int lightVal;

void setup() {
  healthSerial.begin(38400);
  
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  healthSerial.write(0x8A);
}

void loop() {
  // 读取传感器
  smokeVal = analogRead(A0);
  lightVal = analogRead(A1);
  lightVal = 1023 - lightVal;

  // 读取健康数据
  if (healthSerial.available() >= 88) {
    healthSerial.readBytes(buffer, 88);
    if (buffer[0] == 0xFF) {
      if (buffer[65] > 0) heartrate = buffer[65];
      if (buffer[66] > 0) xueyang   = buffer[66];
      if (buffer[71] > 0) high_ya   = buffer[71];
      if (buffer[72] > 0) low_ya    = buffer[72];
    }
  }

  // OLED显示
  display.clearDisplay();
  display.setTextSize(2);

  display.setCursor(0, 0);  display.print("HR:");
  display.setCursor(72, 0); display.print(heartrate);

  display.setCursor(0, 16); display.print("SPO2:");
  display.setCursor(72, 16); display.print(xueyang);

  display.setCursor(0, 32); display.print("SBP:");
  display.setCursor(72, 32); display.print(high_ya);

  display.setCursor(0, 48); display.print("DBP:");
  display.setCursor(72, 48); display.print(low_ya);

  display.display();
  delay(100);
}