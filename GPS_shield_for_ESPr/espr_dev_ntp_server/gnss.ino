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

bool ppsValid = false;
unsigned long ppsCount = 0;
unsigned long lastPPSTime = 0;
unsigned long unixEpochTime = 0;

void ppsInterrupt(void) {
  lastPPSTime = micros() - 2;  // PPS信号立ち上がりからこの処理までの遅延が2us
  unixEpochTime += 1;          // 1秒進める
  ppsValid = true;
  ppsCount += 1;
}

void initGNSS() {
  // PPS信号の立ち上がり割込み処理
  attachInterrupt(ppsPin, ppsInterrupt, RISING);

  // myGNSSを初期化して、ボーレートを9600から38400に変更する
  do {
    Serial.println("GNSS: trying 38400 baud");
    Serial1.end();
    Serial1.begin(38400, SERIAL_8N1, rxPin, txPin);
    if (myGNSS.begin(Serial1) == true) break;

    delay(100);
    Serial.println("GNSS: trying 9600 baud");
    Serial1.end();
    Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);
    if (myGNSS.begin(Serial1) == true) {
      Serial.println("GNSS: connected at 9600 baud, switching to 38400");
      myGNSS.setSerialRate(38400);
      delay(100);
    } else {
      //myGNSS.factoryReset();
      delay(2000);
    }
  } while (1);

  myGNSS.setUART1Output(COM_TYPE_UBX);  // UART1ポートからの出力をUBXのみにする(NMEAと止める)
  myGNSS.setAutoPVTcallbackPtr(&processPVTdata);
  enableQZSSL1S();  // QZSS L1Sを有効に
}

void processPVTdata(UBX_NAV_PVT_data_t *data) {
  if (myGNSS.getTimeFullyResolved()) {
    unsigned long new_epoch = myGNSS.getUnixEpoch();
    if (!ppsValid) {
      if (ppsCount > 0) {
        lastPPSTime += (new_epoch - unixEpochTime) * 1000000;
      } else {
        lastPPSTime = micros();
      }
    }
    ppsValid = false;
    unixEpochTime = new_epoch;
  }
  debugPrintPVT(*data);
}

void processGNSS(void) {
  myGNSS.checkUblox();
  myGNSS.checkCallbacks();
}

// QZSSのL1S信号を受信するよう設定する
bool enableQZSSL1S(void) {
  uint8_t customPayload[MAX_PAYLOAD_SIZE];
  ubxPacket customCfg = { 0, 0, 0, 0, 0, customPayload, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED };

  customCfg.cls = UBX_CLASS_CFG;
  customCfg.id = UBX_CFG_GNSS;
  customCfg.len = 0;
  customCfg.startingSpot = 0;

  if (myGNSS.sendCommand(&customCfg) != SFE_UBLOX_STATUS_DATA_RECEIVED)
    return (false);

  int numConfigBlocks = customPayload[3];
  for (int block = 0; block < numConfigBlocks; block++) {
    if (customPayload[(block * 8) + 4] == (uint8_t)SFE_UBLOX_GNSS_ID_QZSS) {
      customPayload[(block * 8) + 8] |= 0x01;      // set enable bit
      customPayload[(block * 8) + 8 + 2] |= 0x05;  // set 0x01 QZSS L1C/A 0x04 = QZSS L1S
    }
  }

  return (myGNSS.sendCommand(&customCfg) == SFE_UBLOX_STATUS_DATA_SENT);
}

// GNSSの時刻情報から計算したUnixエポックタイムとPPS信号からの経過マイクロ秒を返す
bool getUnixEpoch(uint32_t *seconds, uint32_t *microseconds) {
  bool ret = false;
  if (unixEpochTime && ppsCount > 0) {
    uint32_t sec = unixEpochTime;
    uint32_t msec = micros() - lastPPSTime;
    // PPS信号が入ってきていない時にunixEpochTimeが1秒遅れてしまうので補正する
    uint32_t ovf = msec / 1000000;
    *seconds = sec + ovf;
    *microseconds = msec - ovf * 1000000;
    ret = true;
  }
  return ret;
}

void debugPrintPVT(UBX_NAV_PVT_data_t &data) {
  Serial.print(F("Lat: "));
  Serial.print(data.lat);

  Serial.print(F(" Long: "));
  Serial.print(data.lon);
  Serial.print(F(" (degrees * 10^-7)"));

  Serial.print(F(" Alt: "));
  Serial.print(data.height);
  Serial.print(F(" (mm)"));

  Serial.print(F(" SIV: "));
  Serial.print(data.numSV);

  Serial.print(F(" Fix: "));
  Serial.print(data.fixType);

  Serial.print(" ");
  Serial.print(data.year);
  Serial.print("-");
  Serial.print(data.month);
  Serial.print("-");
  Serial.print(data.day);
  Serial.print(" ");
  Serial.print(data.hour);
  Serial.print(":");
  Serial.print(data.min);
  Serial.print(":");
  Serial.print(data.sec);
  Serial.print(" nano ");
  Serial.print(data.nano);  // Nanoseconds can be negative

  Serial.print(" vD ");
  Serial.print(data.valid.bits.validDate);
  Serial.print(" vT ");
  Serial.print(data.valid.bits.validTime);
  Serial.print(" fR ");
  Serial.print(data.valid.bits.fullyResolved);

  uint32_t microseconds = micros() - lastPPSTime;
  Serial.print(" Epoch ");
  Serial.print(unixEpochTime);
  Serial.print(" micro ");
  Serial.print(microseconds);
  Serial.print(" nPPS ");
  Serial.print(ppsCount);

  Serial.println();
}
