const int SIG_PIN = A0;
const int MAX_SAMPLES = 16;

int readAverage(int pin, int samples = MAX_SAMPLES) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
  }
  return sum / samples;
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  int val = readAverage(SIG_PIN);
  Serial.println(val);
  delay(10);
}
