void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogWriteFrequency(500);
  analogWrite(PA8, 128);
}

void loop() {
  // PA0-PA7 PB0 PB1 PB2 PB11 PB12 PB14 PB15
  int achs[] = { A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14};

  int achs_num = sizeof(achs) / sizeof(achs[0]);
  int ain[achs_num] = { 0 };
  for (int i = 0; i < achs_num; i++) {
    ain[i] = analogRead(achs[i]);
  }
  Serial.print(ain[0]);
  for (int i = 1; i < achs_num; i++) {
    Serial.print(' ');
    Serial.print(ain[i]);
  }
  Serial.println();
}
