/* STM32duino RTCライブラリをインストール */
#include <STM32RTC.h>

/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

bool setRTCFromSerial() {
  char input[20];
  int year, month, day, hours, minutes, seconds;

  Serial.setTimeout(15000);
  if (Serial.readBytesUntil('\n', input, sizeof(input)) > 0) {
    if (sscanf(input, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hours, &minutes, &seconds) == 6) {
      year -= 2000;  // RTC は 2000 年からのオフセットで管理
      rtc.setTime(hours, minutes, seconds);
      rtc.setDate(0, day, month, year);  // weekDay は 0 に設定
      Serial.println("RTC Updated Successfully!");
      return true;
    } else {
      Serial.println("Invalid format! Please enter: YYYY/MM/DD HH:MM:SS");
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Select RTC clock source: LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.
  // By default the LSI is selected as source.
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  // rtc.setClockSource(STM32RTC::HSE_CLOCK);

  rtc.begin();  // initialize RTC 24H format

  if (!rtc.isTimeSet()) {
    // 受信した日時情報を解析して RTC に設定
    do {
      Serial.println("Waiting for date and time input (YYYY/MM/DD HH:MM:SS)...");
    } while (!setRTCFromSerial());
  }
}

void loop() {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint32_t subseconds;
  rtc.getTime(&hours, &minutes, &seconds, &subseconds);

  uint8_t weekday;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  rtc.getDate(&weekday, &day, &month, &year);

  Serial.printf("%02d/%02d/%02d %02d:%02d:%02d.%03u\n", year, month, day, hours, minutes, seconds, subseconds);

  delay(1000 - subseconds + 10);
}
