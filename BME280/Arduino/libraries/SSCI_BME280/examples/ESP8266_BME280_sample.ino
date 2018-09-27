extern "C" {
#include "user_interface.h"
}
#include <Wire.h>
#include <SSCI_BME280.h>

SSCI_BME280 bme280;
uint8_t i2c_addr = 0x76;        //I2C Address

void setup()
{
  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t bme280mode = 3;         //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable

  Serial.begin(115200);
  Wire.begin();
  bme280.setMode(i2c_addr, osrs_t, osrs_p, osrs_h, bme280mode, t_sb, filter, spi3w_en);

  bme280.readTrim();
}


void loop()
{
  double temp_act, press_act, hum_act; //最終的に表示される値を入れる変数

  bme280.readData(&temp_act, &press_act, &hum_act);

  Serial.print("TEMP : ");
  Serial.print(temp_act);
  Serial.print(" DegC  PRESS : ");
  Serial.print(press_act);
  Serial.print(" hPa  HUM : ");
  Serial.print(hum_act);
  Serial.print(" %  Light:");
  Serial.print(1024 - readLight());
  Serial.println(" ");

  delay(1000);
}

unsigned int readLight()
{
  unsigned int light = system_adc_read();
  return light;
}