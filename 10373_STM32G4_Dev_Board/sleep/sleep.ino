/* STM32duino Low Powerライブラリをインストール */
#include "STM32LowPower.h"

void setup() {
  LowPower.begin();
}

void loop() {
  LowPower.deepSleep(5000);
  delay(5000);
}
