#include <M5Core2.h>

#define ADS7142_ADDRESS 0x18

bool writeReg(uint8_t reg, uint8_t value) {
  uint8_t data[3] = { 0x08, reg, value };
  Wire.beginTransmission(ADS7142_ADDRESS);
  if (!Wire.write(data, 3))
    return (false);
  return (Wire.endTransmission() == 0);
}

bool readAdc(uint16_t *buf, uint16_t buflen) {
  Wire.requestFrom(ADS7142_ADDRESS, buflen * 2);
  if (!Wire.available())
    return (false);
  for (uint16_t i = 0; i < buflen; i++) {
    uint16_t dat = Wire.read() << 4;  // read upper byte
    dat += (Wire.read() >> 4) & 0xf;  // read lower byte
    *buf = dat;
    buf++;
  }
  return true;
}

// Wireライブラリのリードバッファを使わずに読み込む
uint16_t readAdc2(uint16_t *buf, uint16_t buflen) {
  size_t rxLength = 0;
  esp_err_t err = i2cRead(0, ADS7142_ADDRESS, (uint8_t *)buf, buflen * 2, 5000, &rxLength);
  rxLength /= 2;
  if (err) {
    log_e("i2cRead returned Error %d", err);
  } else {
    for (size_t i = 0; i < rxLength; i++) {
      buf[i] = ((buf[i] & 0xf000) >> 12) | ((buf[i] & 0xff) << 4);
    }
  }
  return rxLength;
}

void setup() {
  M5.begin(true, true, true, true);
  Serial.end();
  Serial.begin(2000000);
  Wire.setClock(1000000);

  // AUTO_SEQ_CHEN Channel 0/1 is selected
  writeReg(0x20, 0x03);
  // OPMODE_SEL Manual Mode with AUTO Sequencing enabled
  writeReg(0x1C, 0x04);
  // START_SEQUENCE start
  writeReg(0x1E, 0x01);
}

#define BUFLEN 2
uint16_t buffer[BUFLEN];

void loop() {
  // START_SEQUENCE start
  // writeReg(0x1E, 0x01);
  // ADC
  // readAdc(buffer, BUFLEN);
  readAdc2(buffer, BUFLEN);
  // ABORT_SEQUENCE aborts
  // writeReg(0x1F, 0x01);

  for (int i = 0; i < BUFLEN; i += 2) {
    Serial.printf("%d,%d\n", buffer[i], buffer[i + 1]);
  }
}
