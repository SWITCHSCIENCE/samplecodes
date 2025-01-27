const int pinIn = A2;

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println(analogRead(pinIn));
  delay(50);
}
