const int pinSense = A0;

float ref;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  // 起動時に読み取った電圧の平均値を基準電圧とし、基準電圧との差分を電流値に変換する
  ref = 0;
  for (int i = 0; i < 1000; i++) {
    ref += analogRead(pinSense);
    delay(1);
  }
  ref /= 1000;
}

// analogReadの値をアンペアに変換するための定数
#define VA (3.245 / 4096)
// #define VA (0.001)

void loop() {
  // INA181出力を取得
  int ain = analogRead(pinSense);
  // 1秒間の移動平均値を計算
  static float ma = 0;
  ma = (float)ain * 0.05 + ma * 0.95;
  // シリアルに出力（読み取った値 電流換算値mA）
  Serial.printf("%d %f\n", ain, ((ma - ref)*VA*1000));
  // 待機
  delay(2);
}
