// use SimpleKalmanFilter

#include <SimpleKalmanFilter.h>
#include <SPI.h>

const int MA735_CS = SS;
SPISettings settings = SPISettings(10000000, MSBFIRST, SPI_MODE0);

// e_mea: 測定の不確かさ - 測定値がどれくらい変動するかを表します（ほとんどの場合、標準偏差の値をいれとけばOK）。
// e_est: 推定の不確かさ - カルマンフィルタがこの値を調整するため、初期値として e_mea と同じ値を使用できます。
// q: プロセスの分散 - 通常は 0.001 から 1 の間の小さな値で、測定値がどれくらい早く変動するかを表します。
//     推奨値は 0.01 です。用途に応じて調整する必要があります（動きが早い場合は大きく、遅い場合は小さくしてください）。

SimpleKalmanFilter kF(21 /*e_mea*/, 21/*e_est*/, 0.1/*q*/);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPI.begin();
  pinMode(MA735_CS, OUTPUT);
  digitalWrite(MA735_CS, HIGH);
}

void loop() {
  uint16_t rd = readAngle();
  if (rd >= 32768) rd = 32768 - (rd - 32768);  // 連続値に変換
  uint16_t kd = kF.updateEstimate(rd); // カルマンフィルターを通す
  Serial.print(micros());
  Serial.print(',');
  Serial.print(kd);
  Serial.print(',');
  Serial.println(rd);
  delay(20);
}

uint16_t readAngle() {
  SPI.beginTransaction(settings);
  digitalWrite(MA735_CS, LOW);
  uint16_t rd = SPI.transfer16(0);  // 角度の読み込み
  digitalWrite(MA735_CS, HIGH);
  SPI.endTransaction();
  return rd;
}
