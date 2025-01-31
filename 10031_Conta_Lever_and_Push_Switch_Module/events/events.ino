#include "LeverAndPushSw.h"

LeverAndPushSw leverPushSw(A2);

void setup() {
  Serial.begin(115200);
}

void loop() {
  leverPushSw.update();
  LeverAndPushSw::Event event = leverPushSw.getEvent();

  if (event != LeverAndPushSw::NONE) {
    LeverAndPushSw::State state = leverPushSw.getState();
    switch (state) {
      case LeverAndPushSw::PUSH:
        Serial.print("PUSH ");
        break;
      case LeverAndPushSw::LEVER_CCW:
        Serial.print("CCW ");
        break;
      case LeverAndPushSw::LEVER_CW:
        Serial.print("CW ");
        break;
      default:
        break;
    }
    Serial.print("Event: ");
    switch (event) {
      case LeverAndPushSw::PRESSED:
        Serial.println("Pressed");
        break;
      case LeverAndPushSw::RELEASED:
        Serial.println("Released");
        break;
      case LeverAndPushSw::HELD:
        Serial.println("Held");
        break;
      default:
        break;
    }
  }

  delay(20);
}
