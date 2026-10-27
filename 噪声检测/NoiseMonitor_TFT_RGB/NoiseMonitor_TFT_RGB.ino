#include <SPI.h>
#include <Adafruit_GFX.h>    // ST7735图形库核心
#include <Adafruit_ST7735.h> // ST7735驱动库
#include <Wire.h>
#include <SoftwareSerial.h>  // 软件串口库

// ===================== 软件串口定义（和分贝传感器通信） =====================
SoftwareSerial softSerial(5, 6);  // RX=5(A4), TX=6(A5)

// ===================== ST7735 TFT引脚定义 =====================
#define TFT_CS     10  // 片选引脚
#define TFT_RST    8   // 复位引脚
#define TFT_DC     9   // 数据/命令引脚
#define TFT_BL     7   // 背光引脚（PWM调光）
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ===================== RGB 灯引脚定义 =========================
#define RGB_R      12  // red灯引脚
#define RGB_G      4   // green灯引脚
#define RGB_B      2   // blue灯引脚

// ===================== 分贝位图数据 =====================
static const unsigned char PROGMEM oled_st7735_STHeiti_16px_u5206_u8d1d[] = {
  0x0E,0x60,0x30,0x0C,0x00,0x00,0x1C,0x70,0x31,0x8C,0x00,0x00,0x38,0x38,0x31,0x8C,0x00,0x00,
  0x78,0x1C,0x31,0x8C,0x00,0x00,0xF0,0x0E,0x31,0x8C,0x00,0x00,0xC0,0x07,0xB1,0x8C,0x00,0x00,
  0x7F,0xFC,0x31,0x8C,0x00,0x00,0x0E,0x18,0x33,0x8C,0x00,0x00,0x0E,0x18,0x33,0x8C,0x00,0x00,
  0x0E,0x18,0x33,0xCC,0x00,0x00,0x1C,0x38,0x07,0xF0,0x00,0x00,0x38,0x38,0x0E,0x38,0x00,0x00,
  0xE3,0xF0,0x38,0x1E,0x00,0x00,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// ===================== 功能参数定义 =====================
char  data1[8]={0x02,0x03,0x00,0x00,0x00,0x01,0x84,0x39};  

char jieshou[16];
int aa=0;
byte buffer[7];  // 软串口接收缓冲区（分贝传感器）
int shuju = 0;   // 分贝数值
int tempData = 0; // 硬件串口接收的温度数据（两位数）
char tempBuffer[3]; // 存储温度字符串的缓冲区

// ===================== 串口发送函数（软串口发指令给分贝传感器） =====================
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

// ===================== 初始化函数 =====================
void setup(){
  // 硬件串口初始化（和电脑通信，接收温度数据，波特率115200）
  Serial.begin(115200);
  // 软件串口初始化（和分贝传感器通信）
  softSerial.begin(9600);
  
  // TFT屏幕初始化
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // 打开背光
  tft.initR(INITR_BLACKTAB);  // 适配128x160 ST7735B
  tft.setRotation(1);         // 横屏显示（160x128）
  tft.fillScreen(ST7735_BLACK); // 清屏为黑色
  
  // RGB灯初始化
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  digitalWrite(RGB_R, LOW);
  digitalWrite(RGB_G, LOW);
  digitalWrite(RGB_B, LOW);
  
  // 初始提示
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 20);
  tft.print("等待温度数据...");
}

// ===================== 主循环 =====================
void loop(){
  // 1. 软串口和分贝传感器通信（原有逻辑不变）
  chuankoufasong((uint8_t*)data1,8);
  delay(100);
  if(softSerial.available() >= 7){ 
    softSerial.readBytes(buffer,7);  
    shuju=(buffer[3]*256+buffer[4])/10; // 换算分贝值
  }
  
  // 2. 硬件串口接收温度数据（核心修改：从Serial读取两位数）
  if(Serial.available() >= 2) { // 检测硬件串口是否有至少2个字节（两位数）
    Serial.readBytes(tempBuffer, 2); // 读取2个字符
    tempBuffer[2] = '\0'; // 补字符串结束符，避免乱码
    tempData = atoi(tempBuffer); // 转换为整数（如"25"→25）
  }
  
  // 3. RGB灯控制（原有逻辑不变）
  if(shuju <= 70)
    RGB_set("green");
  else if(shuju <= 100)
    RGB_set("blue");
  else
    RGB_set("red");

  // 4. TFT屏幕显示（新增温度显示，其余不变）
  tft.fillScreen(ST7735_BLACK); // 清屏避免重影
  // 显示分贝相关（原有）
  tft.drawBitmap(10, 20, oled_st7735_STHeiti_16px_u5206_u8d1d, 48, 16, ST7735_RED);
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(4);       
  tft.setCursor(25, 50);    
  tft.print(shuju);        
  tft.setCursor(90, 50);    
  tft.print("db");
  
  // 显示温度数据（新增）
  tft.setTextColor(ST7735_CYAN); // 青色显示温度
  tft.setTextSize(2);
  tft.setCursor(25, 85);
  tft.print(tempData);
  tft.print("°C"); // 温度单位
}