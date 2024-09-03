#include <SPI.h>

const int MA735_CS = SS;
SPISettings settings = SPISettings(10000000, MSBFIRST, SPI_MODE0);

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
