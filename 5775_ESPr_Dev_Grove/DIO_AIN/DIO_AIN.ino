void setup() {
  pinMode(14, OUTPUT);  // CN1 : set LED(D0) as an OUTPUT device
  pinMode(12, INPUT);   // CN2 : set SW(D2) as an INPUT device
  pinMode(A0, INPUT);   // CN3 : set Volume (A0) as an Analog INPUT device

  Serial.begin(115200);
  delay(1000);

  Serial.println(" ");
  Serial.println("Digital I/O, Analog Input Tset.");
  Serial.println(" ");
  delay(2000);
}

void loop() {
  if (digitalRead(12)) {
    digitalWrite(14, 1);
    Serial.print("1 : ");
  } else {
    digitalWrite(14, 0);
    Serial.print("0 : ");
  }
  Serial.println(analogRead(A0));
  delay(100);
}



