#include <Wire.h>
#include "AQM1248A.h"

// AQM1248A SETTINGS
#define AQM_GND 2
#define SCK 0
#define MISO 19
#define MOSI 16
#define SS 16
#define PWR 18
#define RS  5
#define CS  17
AQM1248A aqm = AQM1248A(CS,RS,PWR);

void setup() {
  Serial.begin(9600);
  
  /* AQM1248A Settings*/
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  SPI.begin(SCK,MISO,MOSI,SS);
  pinMode(AQM_GND,OUTPUT);
  digitalWrite(AQM_GND,LOW);
  aqm.begin(); // 初期化します
  // 初期画像
  aqm.showPic(); // バッファの内容を液晶に反映します(必須)
  delay(1000);
  aqm.fillScreen(0);
  aqm.showPic();
}

void loop() {
  aqm.fillScreen(0);
  aqm.setCursor(0,0);
  aqm.setTextColor(1);
  aqm.setTextSize(1);
    aqm.print("ESPr Developer 32\n");
    aqm.print("SPI DEMO with IO MUX\n");
    aqm.print("ssci.to/3210");
  aqm.showPic();
  
  delay(1500);
  
}