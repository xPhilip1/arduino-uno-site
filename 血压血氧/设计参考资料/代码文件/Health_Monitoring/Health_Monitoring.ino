/**********************************************************************************
 * 描述    ：Arduino UNO, 波特率：38400, 串口读取健康监测模块生理数据 
 * 硬件连接：        
 * 4针OLED：VCC -> 5V; GND -> GND; SCL -> A5; SDA -> A4; 
 * 健康监测模块： VCC -> 3.3V; RX -> D1; TX -> D0; GND -> GND;  ----硬件串口   波特率：38400
 * USB转TTL模块：VCC -> 5.0V; RX -> D7; TX -> D6; GND -> GND;  ----模拟软串口  波特率：9600
 * 功能描述：硬件串口外接健康监测模块读取生理数据 
 *         软件模拟串口输出生理数据，在OLED液晶屏显示生理数据
 * 使用说明：先下载好程序再接线
**********************************************************************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//实例化软串口
SoftwareSerial mySerial(6, 7); // RX, TX

byte buffer[128];  // 创建一个长度为128的字节数组
int heartrate,xueyang,high_ya,low_ya; // 定义变量 心率 血氧 高压 低压
int heart[64];     
int dataArraySize = sizeof(heart) / sizeof(heart[0]); // 获取数组长度

void setup() 
{
  Serial.begin(38400);
  mySerial.begin(9600);
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // 初始化IIC OLED显示屏
  display.clearDisplay();  // 清空屏幕
  display.setTextColor(SSD1306_WHITE);  // 设置字体颜色
  Serial.write(0x8A);  
}

void loop() 
{
  delay(100);   
//if (Serial.available()>=80) { // 如果串口接收到的字节数大于等于80个

    Serial.readBytes(buffer, 88);  // 读取128个字节到数组中
    if(buffer[0]==0xFF)
   {
      for(int i=0;i<=63;i++)   
      {
        heart[i] = (buffer[i+1] & 0x7F) - (buffer[i+1] & 0x80);  //读取 脉搏波形    
      }
       heartrate = buffer[65]; //读取 心率
       xueyang   = buffer[66]; //读取 血氧
       high_ya   = buffer[71]; //读取 血压 高压
       low_ya    = buffer[72]; //读取 血压 低压             
   }
/*
  for (int ii = 0; ii < dataArraySize - 1; ii++) 
  {
      int x1 = ii ;
      int y1 = map(heart[ii], -128, 127, 0, display.height());
      int x2 = (ii + 1);
      int y2 = map(heart[ii+1], -128, 127, 0, display.height());
      
      display.drawLine(x1,display.height() - y1, x2, display.height() - y2, WHITE); // OLED屏显示 脉搏波形 绘制曲线
  }*/
  
  display.setTextSize(2);    //设置字体大小（1-8）
  
  display.setCursor(72, 0);
  display.print(heartrate);  //OLED屏显示 心率
display.setCursor(0, 0);
  display.print("HR:");
  display.setCursor(72, 16);
  display.print(xueyang);    //OLED屏显示 血氧
display.setCursor(0, 16);
  display.print("SPO2:");

  
  display.setCursor(72, 32);
  display.print(high_ya);    //OLED屏显示 血压 高压
display.setCursor(0, 32);
  display.print("SBP:");
                                      
  
  display.setCursor(72, 48);
  display.print(low_ya);     //OLED屏显示 血压 低压

  display.setCursor(0, 48);
  display.print("DBP:");
/*
  mySerial.print("heartrate:");  
  mySerial.println(heartrate); //模拟串口显示 心率

  mySerial.print("xueyang:");  
  mySerial.println(xueyang);   //模拟串口显示 血氧
    
  mySerial.print("high_ya:");  
  mySerial.println(high_ya);   //模拟串口显示 血压 高压

  mySerial.print("low_ya:");  
  mySerial.println(low_ya);    //模拟串口显示 血压 低压
*/
/*for(int ii=0;ii<64;ii++)
  {display.setTextSize(2);  // 设置字体大小（1-8）
  display.setCursor(20, 20);
  display.print(heart[ii]);  }*/
  display.display(); // 刷新显示屏内容
  delay(100); // 延迟1秒钟
  display.clearDisplay(); // 清空显示屏，准备绘制下一条曲线

//  }  
/* display.setTextSize(2);  // 设置字体大小（1-8）
  display.setCursor(0, 0);
  display.print(heartrate);
  display.setCursor(0, 16);
  display.print(xueyang);  
  display.setCursor(0, 32);
  display.print(high_ya);   
  display.setCursor(60, 32);
  display.print(low_ya);    
  display.display();
  display.clearDisplay(); */
}
