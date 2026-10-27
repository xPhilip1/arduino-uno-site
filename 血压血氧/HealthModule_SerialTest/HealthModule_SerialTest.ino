// void setup() {
//   Serial.begin(9600);
//   pinMode(13, OUTPUT); // 配置引脚13为输出模式
// }

// void loop() {
//   int smokeVal = analogRead(A0);
//   int lightVal = analogRead(A1);
//   lightVal = 1023 - lightVal; // 建议加上这行，让光敏数值更符合直觉

//   // 烟雾报警逻辑：>500 报警（引脚13输出低电平）
//   if(smokeVal > 500) {
//     digitalWrite(13, LOW);
//   } else {
//     digitalWrite(13, HIGH);
//   }

//   // 控制台打印
//   Serial.print("烟雾传感器值：");
//   Serial.print(smokeVal);
//   Serial.print("  |  光敏传感器值：");
//   Serial.println(lightVal);

//   delay(500);
// }


/**********************************************************************************
 * 描述   ：Arduino UNO 读取健康监测模块
 * 硬件连接：
 * 健康监测模块：VCC->3.3V  RX->D1  TX->D0  GND->GND
 * 波特率    ：38400
 * 功能     ：串口控制台打印 心率、血氧、高压、低压
**********************************************************************************/

// 屏幕相关库全部删掉
#include <SoftwareSerial.h>

SoftwareSerial mySerial(6, 7); // 软串口（保留不影响）

byte buffer[128];
int heartrate, xueyang, high_ya, low_ya;

void setup() 
{
  Serial.begin(38400);   // 健康模块 + 电脑串口 都是 38400
  mySerial.begin(9600);
  Serial.write(0x8A);    // 启动健康模块
}

void loop() 
{
  delay(100);

  // 读取健康模块数据
  if(Serial.available() >= 88) {
    Serial.readBytes(buffer, 88);
    
    if(buffer[0] == 0xFF) {
      heartrate = buffer[65];
      xueyang   = buffer[66];
      high_ya   = buffer[71];
      low_ya    = buffer[72];
    }
  }

  // ========== 电脑串口控制台打印 ==========
  Serial.print("心率：");
  Serial.print(heartrate);
  
  Serial.print(" 血氧：");
  Serial.print(xueyang);
  
  Serial.print(" 高压：");
  Serial.print(high_ya);
  
  Serial.print(" 低压：");
  Serial.println(low_ya);
}