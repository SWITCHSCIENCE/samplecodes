void setup() {
  pinMode(25, OUTPUT);  // set LED(D0) as an OUTPUT device
  pinMode(26, INPUT);   // set SW(D2) as an INPUT device
  pinMode(4, INPUT);    // set Volume (A0) as an Analog INPUT device

  Serial.begin(115200);
  delay(1000);

  Serial.println(" ");
  Serial.println("Digital I/O, Analog Input Tset.");
  Serial.println(" ");
  delay(2000);
}

void loop() {
  if (digitalRead(26)) {
    digitalWrite(25, 1);
    Serial.print("1 : ");
  } else {
    digitalWrite(25, 0);
    Serial.print("0 : ");
  }
  Serial.println(analogRead(4));
  delay(100);
}


