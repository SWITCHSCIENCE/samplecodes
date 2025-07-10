const int pinLED = PC4; // LED、LOWで点灯
const int pinBtn = PB8; // BOOTボタン、押すとHIGH

void setup() {
  pinMode(pinLED, OUTPUT);
  pinMode(pinBtn, INPUT);
}

void loop() {
  int rd = digitalRead(pinBtn);  // ボタンを押すとHIGH
  if (rd == HIGH) {
    digitalWrite(pinLED, LOW); // LOWで点灯
  } else {
    digitalWrite(pinLED, HIGH); // HIGHで消灯
  }
  delay(100);
}
