#include <SPI.h>

void writeReg(uint8_t reg, uint8_t value) {
  uint8_t data[] = { 0x08, reg, value };
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(data, 3);
  SPI.endTransaction();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);

  SPI.begin(18, 38, 23, 25);
  SPI.setHwCs(true);

  // OPMODE_CFG CONV_MODE = 0 Manual Mode
  writeReg(0x04, 0x00);

  // SEQUENCE_CFG SEQ_MODE = 2 On-the-fly sequence mode
  writeReg(0x10, 0x02);

  // DATA_CFG APPEND_STATUS = 1 4-bit channel ID is appended to ADC data.
  writeReg(0x02, 0x10);

  // 転送開始
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  // ダミーリード
  SPI.transfer16(0x8000);
}

void loop() {
#define BUFLEN 16
  uint16_t buf[BUFLEN];
  for (int i = 0; i < BUFLEN; i++) {
    buf[i] = SPI.transfer16(0x8000);  // AIN0
  }
  for (int i = 0; i < BUFLEN; i++) {
    Serial.println(buf[i] >> 4);
  }
}
