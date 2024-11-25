#ifdef ARDUINO_ESP32C6_DEV
const int rxPin = 15;
const int txPin = 21;
#elif ARDUINO_ESP32S3_DEV
const int rxPin = 40;
const int txPin = 35;
#else  // ESP32_DEV
const int rxPin = 19;
const int txPin = 21;
#endif

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);
}

void loop() {
  if (Serial.available()) {        // If anything comes in Serial (USB),
    Serial1.write(Serial.read());  // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial1.available()) {       // If anything comes in Serial1 (pins 0 & 1)
    Serial.write(Serial1.read());  // read it and send it out Serial (USB)
  }
}