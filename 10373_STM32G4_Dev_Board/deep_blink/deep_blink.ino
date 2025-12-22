#include "STM32LowPower.h"

const int LEDPin = PC4;

void setup() {
  pinMode(LEDPin, OUTPUT);
  
  // 省電力機能の初期化
  LowPower.begin(); 
}

void loop() {
  // 1. 起きている状態：LEDを1秒点灯（アクティブロー）
  digitalWrite(LEDPin, LOW);
  delay(5000); 
  
  // 2. LEDを消す
  digitalWrite(LEDPin, HIGH);

  // 3. ディープスリープへ移行
  // この間、マイコンは動作を停止し、消費電流が激減します
  LowPower.deepSleep(5000); 
}
