const int servoPin = PA10;  // PWM対応ピンを指定

void setup() {
  pinMode(servoPin, OUTPUT);
  analogWriteFrequency(50);  // サーボ用のPWM周波数（50Hz）に設定
  analogWriteResolution(16);  // 16ビット分解能（0〜65535）で指定
}

void loop() {
  setServoAngle(0);  // 0度
  delay(1000);
  setServoAngle(90);  // 90度
  delay(1000);
  setServoAngle(180);  // 180度
  delay(1000);
  setServoAngle(90);  // 90度
  delay(1000);
}

// サーボ角度を0〜180度で指定する関数
void setServoAngle(int angle) {
  // 0.5ms〜2.5msに相当するPWMデューティ比を計算（分解能に合わせる）
  int minPulse = 1638;  // 0.5ms / 20ms * 65535 ≒ 1638
  int maxPulse = 8191;  // 2.5ms / 20ms * 65535 ≒ 8191

  int duty = map(angle, 0, 180, minPulse, maxPulse);
  analogWrite(servoPin, duty);
}
