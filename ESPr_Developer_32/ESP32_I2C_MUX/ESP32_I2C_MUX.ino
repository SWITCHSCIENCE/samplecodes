#include <Wire.h>
#include "SSCI_BME280.h"

// BME280 SETTINGS
#define BME_VIO 25
#define BME_VCORE 26
#define BME_GND 15
#define BME_CS0 14
#define BME_SDI 12
#define BME_SCK 13
#define BME_SDO 34 // ダミー
SSCI_BME280 bme280;
uint8_t i2c_addr = 0x76;        //I2C Address

void setup()
{
  Serial.begin(115200);

  /* BME280 Settings */
  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t bme280mode = 3;         //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable
  pinMode(BME_GND,OUTPUT);
    digitalWrite(BME_GND,LOW);
  pinMode(BME_VIO,OUTPUT);
    digitalWrite(BME_VIO,HIGH);
  pinMode(BME_VCORE,OUTPUT);
    digitalWrite(BME_VCORE,HIGH);
  pinMode(BME_CS0,INPUT);
  pinMode(BME_SDO,OUTPUT);
    digitalWrite(BME_SDO,LOW); // ダミー

  Wire.begin(BME_SDI,BME_SCK);
  bme280.setMode(i2c_addr, osrs_t, osrs_p, osrs_h, bme280mode, t_sb, filter, spi3w_en);

  bme280.readTrim();
}

void loop() {
  double temp_act, press_act, hum_act; //最終的に表示される値を入れる変数

  bme280.readData(&temp_act, &press_act, &hum_act);

    Serial.print("/////\nTEMP : ");
    Serial.print(temp_act);
    Serial.print(" DegC \nPRESS : ");
    Serial.print(press_act);
    Serial.print(" hPa  \nHUM : ");
    Serial.println(hum_act);
  
  delay(1000);
}