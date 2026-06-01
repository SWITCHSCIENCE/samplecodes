// ホールセンサー ラッチ出力シミュレーション
// アナログ出力型センサーでラッチ動作をソフトウェアエミュレート
// 回転動作の検出等に使用

const int SIG_PIN = A0;
const int MAX_SAMPLES = 16;

// ヒステリシスを設けてチャタリングを防ぐ
const int ON_THRESHOLD = 560;   // S極が近づいてこの値を超えたら ON にラッチ
const int OFF_THRESHOLD = 464;  // N極が近づいてこの値を下回ったら OFF にラッチ

bool state = false;  // false = 0(OFF), true = 1(ON)

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
  int val = readAverage(SIG_PIN);

  if (!state) {
    // 現在 OFF → ONしきい値を超えたらラッチON
    if (val > ON_THRESHOLD) {
      state = true;
      Serial.println(1);
    }
  } else {
    // 現在 ON → OFFしきい値を下回ったらラッチOFF
    if (val < OFF_THRESHOLD) {
      state = false;
      Serial.println(0);
    }
  }

  delay(10);
}