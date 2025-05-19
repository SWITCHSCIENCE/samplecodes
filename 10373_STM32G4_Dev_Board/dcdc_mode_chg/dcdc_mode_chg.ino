const int ModePin = PC10;

void setup() {
  pinMode(ModePin, OUTPUT);
}

void loop() {
  // PWM mode
  digitalWrite(ModePin, HIGH);
  delay(5000);
  // PFM/PWM mode
  digitalWrite(ModePin, LOW);
  delay(5000);
}
