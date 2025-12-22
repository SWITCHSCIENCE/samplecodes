#include <STM32RTC.h>

STM32RTC& rtc = STM32RTC::getInstance();

void setup() {
  Serial.begin(9600);

  // 時計用の32768Hzクリスタルを使用
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin();

  // 時刻の設定（例：12時00分00秒）
  rtc.setTime(12, 0, 0);
  // 日付の設定（例：2025年12月31日水曜日）
  // 引数：週(1-7), 日, 月, 年(下2桁)
  rtc.setDate(RTC_WEEKDAY_WEDNESDAY, 31, 12, 25);
}

void loop() {
  // シリアルモニタに表示
  Serial.printf("20%02d/%02d/%02d ", rtc.getYear(), rtc.getMonth(), rtc.getDay());
  Serial.printf("%02d:%02d:%02d.%03d\n", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());
  delay(1000);
}