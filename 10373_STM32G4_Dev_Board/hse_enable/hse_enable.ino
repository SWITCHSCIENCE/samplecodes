#include <Arduino.h>

// PWM 出力に使うピン（PB0）
// オシロスコープやロジアナで周波数・周期を測定して
// HSE クロックの精度を確認する用途
const int pwmPin = PB0;

void setup() {
  // analogWrite の分解能を 10bit に設定
  // duty は 0〜1023 の範囲になる
  analogWriteResolution(10);

  // PWM を開始
  // 512 は 10bit 分解能での 50% デューティ
  // ・デューティ比が正確に 50% になるか
  // ・PWM 周波数が理論値どおりか
  // を測定することで、HSE の
  // クロック精度・安定性を確認できる
  analogWrite(pwmPin, 512);
}

void loop() {
  // ループ処理は不要
  // setup() 実行後は PWM を出しっぱなしにして測定専用とする
}
