#include <SPI.h>

uint8_t spi_mode = SPI_MODE0;

void writeReg(uint8_t reg, uint8_t value) {
  uint8_t data[] = { 0x08, reg, value };
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, spi_mode));
  SPI.transfer(data, 3);
  SPI.endTransaction();
}

uint16_t channel = 0;

void setup() {

  // put your setup code here, to run once:
  Serial.begin(2000000);

#ifdef ARDUINO_m5stack_core2
  SPI.begin(27, 26, 19, 25);  // SCK MISO MOSI SS
#else
  SPI.begin(12, 13, 11, 10);
#endif
  SPI.setHwCs(true);

  // OPMODE_CFG CONV_MODE = 0 Manual Mode
  writeReg(0x04, 0x00);

  // SEQUENCE_CFG SEQ_MODE = 2 On-the-fly sequence mode
  writeReg(0x10, 0x02);

  // DATA_CFG
  // APPEND_STATUS = 1b 4-bit channel ID is appended to ADC data.
  // CPOL_CPHA = 11b = CPOL = 1, CPHA = 1
  writeReg(0x02, 0x13);
  spi_mode = SPI_MODE3;

  // 転送開始
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, spi_mode));

  // 空読みと次のチャンネル設定
  SPI.transfer16(0x8000 | (channel << 11));
}

void loop() {
  uint16_t rd = SPI.transfer16(0x8000 | (channel << 11));  // AD値の読みと次のチャンネル設定
  Serial.println(rd >> 4);
}
