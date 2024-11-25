#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

SFE_UBLOX_GNSS myGNSS;

#ifdef ARDUINO_ESP32C6_DEV
const int rxPin = 15;
const int txPin = 21;
const int ppsPin = 3;
#elif ARDUINO_ESP32S3_DEV
const int rxPin = 40;
const int txPin = 35;
const int ppsPin = 1;
#else  // ESP32_DEV
const int rxPin = 19;
const int txPin = 21;
const int ppsPin = 23;
#endif

unsigned long lastPPSTime = 0;
unsigned long ppsCount = 0;

void ppsInterrupt(void) {
  lastPPSTime = micros();
  ppsCount++;
}

void setup() {
  attachInterrupt(ppsPin, ppsInterrupt, RISING);
  Serial.begin(115200);
  while (!Serial)
    ;  //ターミナルを開くまでまつ
  Serial.println("SparkFun u-blox Example");

  Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);

  // myGNSS.enableDebugging(); // デバッグメッセージをSerialに出力する
  if (myGNSS.begin(Serial1) == false)  // Serial1を介してu-bloxモジュールに接続する
  {
    // ボーレートを変えてトライ
    Serial1.begin(38400, SERIAL_8N1, rxPin, txPin);
    if (myGNSS.begin(Serial1) == false) {
      Serial.println(F("u-blox GNSS not detected. Please check wiring. Freezing."));
      while (1)
        ;
    }
  }

  myGNSS.setUART1Output(COM_TYPE_UBX);  //UART1から出力をUBXのみに設定する
}

void loop() {
  if (myGNSS.getPVT()) {

    long latitude = myGNSS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    long longitude = myGNSS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    long altitude = myGNSS.getAltitude();
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    byte SIV = myGNSS.getSIV();
    Serial.print(F(" SIV: "));
    Serial.print(SIV);

    byte fixType = myGNSS.getFixType();
    Serial.print(F(" Fix: "));
    Serial.print(fixType);

    Serial.print(" ");
    Serial.print(myGNSS.getYear());
    Serial.print("-");
    Serial.print(myGNSS.getMonth());
    Serial.print("-");
    Serial.print(myGNSS.getDay());
    Serial.print(" ");
    Serial.print(myGNSS.getHour());
    Serial.print(":");
    Serial.print(myGNSS.getMinute());
    Serial.print(":");
    Serial.print(myGNSS.getSecond());
    Serial.print(" nano ");
    Serial.print(myGNSS.getNanosecond());  // Nanoseconds can be negative

    Serial.print(" dV ");
    Serial.print(myGNSS.getDateValid());
    Serial.print(" tV ");
    Serial.print(myGNSS.getTimeValid());

    unsigned long msec = micros() - lastPPSTime;
    Serial.print(" msec ");
    Serial.print(msec);
    Serial.print(" nPPS ");
    Serial.print(ppsCount);

    myGNSS.flushPVT();

    Serial.println();
  }
}