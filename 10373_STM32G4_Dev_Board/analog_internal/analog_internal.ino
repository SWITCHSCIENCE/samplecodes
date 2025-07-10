#define TS_CAL1 (*((uint16_t*)0x1FFF75A8))  // 3.0V  30℃時のADC値
#define TS_CAL2 (*((uint16_t*)0x1FFF75CA))  // 3.0V 130℃時のADC値
#define VREFINT (*((uint16_t*)0x1FFF75AA))  // 3.0V VREFINTのADC値

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
}

void loop() {
  uint32_t startTime = millis();
  uint32_t sum_temp = 0;
  uint32_t sum_vref = 0;
  uint16_t count = 0;

  while (millis() - startTime < 1000) {
    sum_temp += analogRead(ATEMP);
    sum_vref += analogRead(AVREF);
    count++;
  }

  uint16_t raw_temp = sum_temp / count;
  uint16_t raw_vref = sum_vref / count;

  // 実際のVDDAを算出（VREFINTは3.0V時の値）
  float vdda = 3.0 * ((float)VREFINT / (float)raw_vref);
  Serial.print("VDDA: ");
  Serial.print(vdda, 3);
  Serial.print("V ");

  // 補正後の温度値を算出（TS_CAL1/TS_CAL2の値は3.0V時の値）
  float temperature = ((float)((raw_temp * vdda) / 3.0) - TS_CAL1) * (130.0 - 30.0) / (TS_CAL2 - TS_CAL1) + 30.0;

  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");
}
