// ホールセンサー スイッチ出力シミュレーション（S/N両極対応）
// S極 → val が CENTER より上昇
// N極 → val が CENTER より下降
// 中央値からの絶対偏差でしきい値判定する
// ドアスイッチ等に使用

const int SIG_PIN      = A0;
const int MAX_SAMPLES  = 16;

const int CENTER       = 512;  // 磁界ゼロ時の中央値（要キャリブレーション）
const int ON_DEVIATION =  10;  // この偏差を超えたら 1 を出力

int readAverage(int pin, int samples = MAX_SAMPLES) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
  }
  return sum / samples;
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  int val       = readAverage(SIG_PIN);
  int deviation = abs(val - CENTER);  // S/N どちらでも正の偏差に変換
  bool out      = (deviation > ON_DEVIATION);
  Serial.println(out ? 1 : 0);
  delay(10);
}
