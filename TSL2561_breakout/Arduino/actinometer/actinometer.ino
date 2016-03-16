#include <Wire.h>
#include <SPI.h>
//tsl2561 設定
#include "TSL2561.h"
#define TSL2561_VDD A2
#define TSL2561_GND A3
TSL2561 tsl(TSL2561_ADDR_FLOAT);
// aqm1248 設定
#include "Adafruit_GFX.h"
#include "AQM1248A.h"
#define PWR 8
#define RS  9
#define CS  10
AQM1248A aqm = AQM1248A(CS,RS,PWR);



void setup(void) {
    Serial.begin(115200);
    //while (!Serial);

    tsl_setup();
    aqm.begin();

    aqm.showPic();
    Serial.println("SWITCH SCIENCE Actinomator");
    delay(1000);
    aqm.fillScreen(0);
}

void loop(void)
{

    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full,data;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    data = tsl.calculateLux(full, ir);
    Serial.println("Lux: "); Serial.println(data);

    // 前の描画を消す
    aqm.fillScreen(0);
    // 最新のデータで描画する
    aqm.setCursor(0,0);aqm.setTextColor(1);aqm.setTextSize(1);
    aqm.println("Lux: ");aqm.print(data);aqm.println("lux");

    aqm.drawRect(0,23,1 + data/32,20,1); // 測定MAX約4000/128pix = 31
    aqm.showPic();

}



void tsl_setup(void)
{
      /* TSL2561 を基板に直付するときのピン設定 */
      pinMode(TSL2561_VDD, OUTPUT);
      pinMode(TSL2561_GND, OUTPUT);
      digitalWrite(TSL2561_VDD, HIGH);
      digitalWrite(TSL2561_GND, LOW);

      if (tsl.begin()) {
        Serial.println("Found sensor");
      } else {
        Serial.println("No sensor?");
        while (1);
      }

      // 周囲の明暗状況に合わせて適切にゲインを変更してください
      //tsl.setGain(TSL2561_GAIN_0X);         // ゲインなし:周囲が明るい場合
      tsl.setGain(TSL2561_GAIN_16X);      // ゲインx16:周囲が暗い場合

      // 積算時間を変更することで、光の測定時間を変更することができます
      // 長時間測定すると、データの取得は遅くなりますが、低光度環境での測定性能が向上します
      tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // 短時間測定:明るい環境
      //tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // 中時間測定:中程度の明るさ
      //tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // 最長時間測定:暗い環境
}
