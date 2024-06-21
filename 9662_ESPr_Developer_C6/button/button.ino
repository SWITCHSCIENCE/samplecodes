const int ledPin = 18;
const int btnPin = 9;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);
}

void loop() {
  digitalWrite(ledPin, !digitalRead(btnPin));
  delay(50);
}