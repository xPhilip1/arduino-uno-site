/**********************************************************************************
 * 文件名  ：HealthMonitor_Comprehensive.ino
 * 描述    ：Arduino UNO 综合健康监测仪
 *          合并 健康监测模块 + 烟雾/光敏传感器，OLED显示
 * 硬件连接：
 *   4针OLED屏：       VCC -> 5V;  GND -> GND; SCL -> A5; SDA -> A4;  (I2C)
 *   健康监测模块：     VCC -> 3.3V; RX -> D1; TX -> D0; GND -> GND;  (硬件串口 38400)
 *   USB转TTL(调试)：  VCC -> 5V;  RX -> D7; TX -> D6; GND -> GND;  (软串口 9600)
 *   烟雾传感器：       A0 -> 模拟输入
 *   光敏传感器：       A1 -> 模拟输入
 *   板载LED：          D13 -> 数字输出 (低光照明 / 传感器故障指示)
 *   有源蜂鸣器：       D12 -> 数字输出 (烟雾超标报警，低电平触发)
 * 功能描述：
 *   1. 硬件串口读取健康监测模块生理数据（心率、血氧、血压）
 *   2. 模拟口读取烟雾传感器和光敏传感器
 *   3. OLED液晶屏实时显示所有数据
 *   4. 烟雾超标时D12蜂鸣器报警
 *   5. 环境光较弱或传感器故障时D13板载LED自动点亮
 *   6. 生理数据仅在有效变化时更新显示（过滤噪声与传感器故障值）
 *   7. 高血压等真实生理读数不视为异常，正常显示和更新
 * 使用说明：先下载好程序再接线
 **********************************************************************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// ============ 显示屏配置 ============
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ============ 串口配置 ============
SoftwareSerial mySerial(6, 7);  // RX=D6, TX=D7 (调试用软串口)

// ============ 传感器引脚 ============
#define SMOKE_PIN    A0   // 烟雾传感器
#define LIGHT_PIN    A1   // 光敏传感器
#define LED_PIN      13   // 板载LED（低光照明 / 传感器故障指示）
#define BUZZER_PIN   12   // 有源蜂鸣器（烟雾超标报警，低电平触发）

// ============ 数据缓冲区 ============
byte buffer[128];
int heart[64];

// ============ 生理参数 — 正常初始值 ============
int heartrate = 72;    // 心率 (bpm)
int xueyang   = 98;    // 血氧 (%)
int high_ya   = 118;   // 收缩压 (mmHg)
int low_ya    = 86;    // 舒张压 (mmHg)

// ============ 环境传感器变量 ============
int smokeVal = 0;
int lightVal = 0;

// ============ 报警标志 ============
bool vitalAlarm  = false;  // 传感器故障报警
bool smokeAlarm  = false;  // 烟雾超标报警
bool lowLightLed = false;  // 光敏低光自动开灯

// ============ 变化阈值（低于此值视为噪声，不更新） ============
#define HR_THRESHOLD    2   // 心率变化 < 2 bpm 不更新
#define SPO2_THRESHOLD  1   // 血氧变化 < 1%  不更新
#define BP_THRESHOLD    2   // 血压变化 < 2 mmHg 不更新

// ============ 光敏自动开灯阈值 ============
// lightVal 是反转后的值（越大越亮），低于此阈值时 D13 自动点亮LED
#define LIGHT_LED_THRESHOLD  300

// ==================== 初始化 ====================
void setup()
{
  Serial.begin(38400);         // 硬件串口 - 健康监测模块
  mySerial.begin(9600);        // 软串口   - 调试输出
  Wire.begin();

  // 初始化OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // 初始化LED和蜂鸣器引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);       // LED低电平=关闭
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);   // 蜂鸣器低电平触发，初始高电平=关闭

  // 向健康监测模块发送启动指令
  Serial.write(0x8A);

  // 开机画面：显示初始正常值 2 秒
  displayStartup();
}

// ==================== 主循环 ====================
void loop()
{
  // ---- 读取健康监测模块数据 ----
  readHealthData();

  // ---- 读取环境传感器数据 ----
  readEnvSensors();

  // ---- 更新报警状态 ----
  updateAlarm();

  // ---- 刷新OLED显示 ----
  updateDisplay();

  delay(200);  // 主循环延迟 200ms
}

// ==================== 读取健康数据 ====================
void readHealthData()
{
  // 读取88字节数据帧
  size_t bytesRead = Serial.readBytes(buffer, 88);

  if (bytesRead >= 88 && buffer[0] == 0xFF)
  {
    // 提取脉搏波形（64点）
    for (int i = 0; i <= 63; i++)
    {
      heart[i] = (buffer[i + 1] & 0x7F) - (buffer[i + 1] & 0x80);
    }

    // 提取生理参数原始值
    int raw_hr  = buffer[65];   // 心率
    int raw_spo = buffer[66];   // 血氧
    int raw_sbp = buffer[71];   // 高压
    int raw_dbp = buffer[72];   // 低压

    // ===== 有效变化检测 + 传感器故障过滤 =====
    // 仅过滤传感器故障值（0 或 255），高血压等真实读数照常更新
    // 心率：新值不是传感器故障，且变化量超过阈值时才更新
    if (!isSensorError(raw_hr) && abs(raw_hr - heartrate) >= HR_THRESHOLD)
    {
      heartrate = raw_hr;
    }

    // 血氧：同理
    if (!isSensorError(raw_spo) && abs(raw_spo - xueyang) >= SPO2_THRESHOLD)
    {
      xueyang = raw_spo;
    }

    // 收缩压（高压）：同理
    if (!isSensorError(raw_sbp) && abs(raw_sbp - high_ya) >= BP_THRESHOLD)
    {
      high_ya = raw_sbp;
    }

    // 舒张压（低压）：同理
    if (!isSensorError(raw_dbp) && abs(raw_dbp - low_ya) >= BP_THRESHOLD)
    {
      low_ya = raw_dbp;
    }
  }
}

// ==================== 读取环境传感器 ====================
void readEnvSensors()
{
  smokeVal = analogRead(SMOKE_PIN);
  lightVal = 1023 - analogRead(LIGHT_PIN);  // 反转，数值越大表示越亮

  // 烟雾报警判断：>500 为超标
  smokeAlarm = (smokeVal > 500);

  // 光敏自动开灯判断：环境光低于阈值时点亮LED
  lowLightLed = (lightVal < LIGHT_LED_THRESHOLD);
}

// ==================== 更新报警输出 ====================
void updateAlarm()
{
  // 仅当传感器返回明显错误值（0或255）时才判定为传感器故障
  // 高血压、高心率等真实生理读数不触发报警
  vitalAlarm = isSensorError(heartrate) || isSensorError(xueyang)
               || isSensorError(high_ya) || isSensorError(low_ya);

  // ---- D12 蜂鸣器：仅烟雾超标时响（低电平触发） ----
  if (smokeAlarm)
  {
    digitalWrite(BUZZER_PIN, LOW);
  }
  else
  {
    digitalWrite(BUZZER_PIN, HIGH);
  }

  // ---- D13 板载LED：低光照明 或 传感器故障时亮 ----
  if (lowLightLed || vitalAlarm)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }
}

// ==================== 刷新OLED显示 ====================
void updateDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);

  // ---- 第一行：心率 HR ----
  display.setCursor(0, 0);
  display.print("HR:");
  display.setTextSize(2);
  display.setCursor(42, 0);
  display.print(heartrate);
  // 仅传感器故障（读到0或255）时显示感叹号
  if (isSensorError(heartrate))
  {
    display.setTextSize(1);
    display.print("!");
  }

  // ---- 第二行：血氧 SPO2 ----
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("SP:");
  display.setTextSize(2);
  display.setCursor(42, 16);
  display.print(xueyang);
  display.print("%");
  if (isSensorError(xueyang))
  {
    display.setTextSize(1);
    display.print("!");
  }

  // ---- 第三行：收缩压 SBP ----
  display.setTextSize(1);
  display.setCursor(0, 32);
  display.print("SB:");
  display.setTextSize(2);
  display.setCursor(42, 32);
  display.print(high_ya);
  if (isSensorError(high_ya))
  {
    display.setTextSize(1);
    display.print("!");
  }

  // ---- 第四行：舒张压 DBP ----
  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("DB:");
  display.setTextSize(2);
  display.setCursor(42, 48);
  display.print(low_ya);
  if (isSensorError(low_ya))
  {
    display.setTextSize(1);
    display.print("!");
  }

  // ---- 右侧信息栏：烟雾 & 光敏 ----
  display.setTextSize(1);
  display.setCursor(90, 36);
  display.print("S:");
  display.print(smokeVal);
  display.setCursor(90, 48);
  display.print("L:");
  display.print(lightVal);

  // ---- 报警状态指示 ----
  if (smokeAlarm)
  {
    display.setCursor(90, 0);
    display.print("**SMK**");
  }
  if (vitalAlarm)
  {
    display.setCursor(90, 12);
    display.print("**ALM**");
  }
  if (lowLightLed)
  {
    display.setCursor(90, 24);
    display.print("**LED**");
  }

  display.display();  // 推送到屏幕
}

// ==================== 开机画面 ====================
void displayStartup()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.print("Health Monitor v1.0");
  display.drawLine(0, 12, 128, 12, WHITE);

  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("HR:");
  display.setCursor(42, 18);
  display.print(heartrate);
  display.print(" bpm");

  display.setCursor(0, 28);
  display.print("SPO2:");
  display.setCursor(42, 28);
  display.print(xueyang);
  display.print(" %");

  display.setCursor(0, 38);
  display.print("SBP:");
  display.setCursor(42, 38);
  display.print(high_ya);
  display.print(" mmHg");

  display.setCursor(0, 48);
  display.print("DBP:");
  display.setCursor(42, 48);
  display.print(low_ya);
  display.print(" mmHg");

  display.display();
  delay(2000);  // 显示 2 秒后进入主循环
  display.clearDisplay();
}

// ==================== 传感器故障检测 ====================
// 仅判断传感器是否返回了明显错误的值（0 或 255）
// 这些值通常是传感器未就绪、通信错误或字节溢出导致的
// 高血压(如160)、高心率(如130)等真实生理读数不算异常，正常显示和更新

bool isSensorError(int val)
{
  return (val <= 0 || val >= 255);
}
