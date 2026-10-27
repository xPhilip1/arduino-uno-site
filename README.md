# Arduino UNO 传感器实验合集（健康监测 + 噪声监测）

> 一块 Arduino UNO，两个方向的小实验：**指夹式健康监测**（心率 / 血氧 / 血压 + 烟雾 + 光敏）与
> **噪声监测**（分贝值 + TFT 彩屏 + RGB 灯）。

这个目录收集了课程实验过程中逐步迭代出来的 7 个 sketch，从"只读传感器原始数据"到"综合显示 + 报警"，
每个版本都能独立烧录运行。

## 功能一览

### 血压血氧（`血压血氧/`）

| Sketch | 说明 |
| --- | --- |
| `SmokeLight_SensorTest` | 烟雾（MQ-2，A0）与光敏（A1）阈值测试，超过 500 点亮 D13 |
| `HealthModule_SerialTest` | 健康监测模组的串口裸数据测试 |
| `Health_Monitoring` | 硬件串口读心率 / 血氧 / 高压 / 低压，OLED 显示，软串口回传电脑 |
| `HealthMonitor_OLED_Smoke_Sensor` | 精简版：OLED + 健康模组 + 烟雾 + 光敏，无串口打印 |
| `HealthMonitor_Comprehensive` | 综合版：生理数据 + 烟雾报警（D12 蜂鸣器）+ 弱光照明（D13）+ 数据滤波 |

### 噪声检测（`噪声检测/`）

| Sketch | 说明 |
| --- | --- |
| `NoiseMonitor_TFT_RGB_Basic` | 分贝传感器 + ST7735 彩屏，最小可用版本 |
| `NoiseMonitor_TFT_RGB` | 增加分贝柱状图与中文位图字模 |
| `NoiseMonitor_TFT_RGB_Optimized` | 优化刷新逻辑，减少闪屏与重复绘制 |

另外附带 `血压血氧/设计参考资料/`（模组资料、电路图）。

## 硬件与接线

**健康监测综合版**（注释里也有完整接线表）：

```
4 针 OLED      VCC→5V   GND→GND   SCL→A5   SDA→A4      (I2C)
健康监测模组    VCC→3.3V RX→D0    TX→D1    GND→GND     (硬件串口, 38400)
USB-TTL 调试   VCC→5V   RX→D7    TX→D6    GND→GND     (软串口, 9600)
MQ-2 烟雾      AO→A0
光敏电阻       AO→A1
有源蜂鸣器      D12（低电平触发）
板载 LED        D13
```

**噪声监测**：

```
分贝传感器  TX→D5  RX→D6   (SoftwareSerial)
ST7735      CS→D10 RST→D8  DC→D9  BL→D7
RGB 灯      R→D12 G→D4     B→D2
```

## 上手前准备

| 项 | 说明 |
| --- | --- |
| **Arduino IDE** | 1.8+ 或 2.x 均可 |
| **三个库** | 库管理器装 `Adafruit GFX Library`、`Adafruit SSD1306`、`Adafruit ST7735 and ST7789 Library`（不装会直接编译报错） |
| **开发板与端口** | Board 选 `Arduino UNO`，Port 选实际串口（插上板子才出现） |
| **接线** | 按每个 sketch 头部的接线表接；健康监测模组**必须 3.3V 供电**，接 5V 可能烧模块 |
| **阈值标定** | 烟雾阈值 500、光敏 `1023 - val`、分贝换算都是写死的经验值，换传感器/换环境要自己重新标 |
| **串口波特率** | 健康模组 38400，调试口 9600，不要搞混 |

## 怎么运行

1. 安装 Arduino IDE 1.8+ 或 2.x；
2. 库管理器安装 `Adafruit GFX Library`、`Adafruit SSD1306`、`Adafruit ST7735 and ST7789 Library`；
3. 打开对应 `.ino` 文件（**sketch 必须放在同名文件夹里**，本仓库已按此组织）；
4. 选开发板 `Arduino UNO`、对应串口，上传。

> 健康监测模组需要 3.3V 供电，接 5V 可能烧模块；接线请先断电。

## 目录说明

```
arduino-uno-site/          # 目录名为历史遗留，与网站无关
├── 血压血氧/
│   ├── HealthModule_SerialTest/
│   ├── HealthMonitor_Comprehensive/
│   ├── HealthMonitor_OLED_Smoke_Sensor/
│   ├── SmokeLight_SensorTest/
│   └── 设计参考资料/        # 模组手册、电路图、参考代码
├── 噪声检测/
│   ├── NoiseMonitor_TFT_RGB/
│   ├── NoiseMonitor_TFT_RGB_Basic/
│   └── NoiseMonitor_TFT_RGB_Optimized/
```

## 已知问题

- 健康模组输出的血压值精度有限，仅作演示，不能用于医疗判断；
- 各 sketch 的引脚定义是硬编码的，换板子需要改 `#define`；
- `HealthMonitor_Comprehensive` 里的数据滤波是经验阈值，传感器个体差异需要重新标定。

## License

课程实验代码，仅供学习参考。
