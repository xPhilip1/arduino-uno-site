void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT); // 配置引脚13为输出模式
}

void loop() {
  int smokeVal = analogRead(A0);
  int lightVal = analogRead(A1);
  lightVal = 1023 - lightVal; // 建议加上这行，让光敏数值更符合直觉

  // 烟雾报警逻辑：>500 报警（引脚13输出低电平）
  if(smokeVal > 500) {
    digitalWrite(13, LOW);
  } else {
    digitalWrite(13, HIGH);
  }

  // 控制台打印
  Serial.print("烟雾传感器值：");
  Serial.print(smokeVal);
  Serial.print("  |  光敏传感器值：");
  Serial.println(lightVal);

  delay(500);
}
