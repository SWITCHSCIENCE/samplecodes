#include <math.h>

const int pinDAC1 = PA4;  // DAC1_CH1
const int pinDAC2 = PA5;  // DAC1_CH2

const int numSamples = 100;      // 波形の分解能
const float amplitude = 2047.5;  // 12bit DACの最大値(4095)の半分
const float offset = 2047.5;     // 中心値

void setup() {
  analogWriteResolution(12);
}

void loop() {
  for (int i = 0; i < numSamples; i++) {
    float angle = (2 * M_PI * i) / numSamples;          // 0 〜 2π の範囲で分割
    int sineValue = offset + amplitude * sin(angle);    // サイン波
    int cosineValue = offset + amplitude * cos(angle);  // コサイン波
    analogWrite(pinDAC1, sineValue);
    analogWrite(pinDAC2, cosineValue);
  }
}
