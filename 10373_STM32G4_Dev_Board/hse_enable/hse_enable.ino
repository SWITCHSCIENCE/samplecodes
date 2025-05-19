#include <Arduino.h>

const int pwmPin = PB15;
HardwareTimer *MyTim;

void setup() {

  // Automatically retrieve TIM instance and channel associated to pin
  // This is used to be compatible with all STM32 series automatically.
  TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(pwmPin), PinMap_PWM);
  uint32_t channel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(pwmPin), PinMap_PWM));

  // Instantiate HardwareTimer object. Thanks to 'new' instantiation, HardwareTimer is not destructed when setup() function is finished.
  MyTim = new HardwareTimer(Instance);

  // Configure and start PWM
  MyTim->setPWM(channel, pwmPin, 1000, 50);  // 1kHz, 50% dutycycle
}

void loop() {
}
