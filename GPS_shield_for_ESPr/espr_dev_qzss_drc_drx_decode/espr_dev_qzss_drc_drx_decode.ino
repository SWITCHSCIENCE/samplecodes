#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <QZQSM.h>
#include "QZSSDCX.h"

SFE_UBLOX_GNSS myGNSS;

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

#define DBG_PRINT_SFRBX 0
#define DBG_PRINT_SAT 0
#define DBG_PRINT_PVT 0
#define DBG_PRINT_DCX_ALL 0

byte l1s_msg_buf[32];  // MAX 250 BITS
QZQSM dc_report;
DCXDecoder dcx_decoder;

// dwrdを16進数文字列に変換して出力する関数
const char *dwrd_to_str(uint32_t value) {
  static const char hex_chars[] = "0123456789ABCDEF";  // 16進数文字
  static char buffer[9];                               // 8桁 + 終端文字
  // リトルエンディアンなので入れ替える
  buffer[8] = '\0';
  buffer[7] = hex_chars[value & 0xF];
  buffer[6] = hex_chars[value >> 4 & 0xF];
  buffer[5] = hex_chars[value >> 8 & 0xF];
  buffer[4] = hex_chars[value >> 12 & 0xF];
  buffer[3] = hex_chars[value >> 16 & 0xF];
  buffer[2] = hex_chars[value >> 20 & 0xF];
  buffer[1] = hex_chars[value >> 24 & 0xF];
  buffer[0] = hex_chars[value >> 28 & 0xF];
  return buffer;
}

void newSFRBX(UBX_RXM_SFRBX_data_t *data) {
#if DBG_PRINT_SFRBX
  Serial.print("SFRBX gnssId: ");
  Serial.print(data->gnssId);
  Serial.print(" svId: ");
  Serial.print(data->svId);
  Serial.print(" freqId: ");
  Serial.print(data->freqId);
  Serial.print(" numWords: ");
  Serial.print(data->numWords);
  Serial.print(" version: ");
  Serial.print(data->version);
  Serial.print(" ");
  for (int i = 0; i < data->numWords; i++) {
    Serial.print(dwrd_to_str(data->dwrd[i]));
  }
  Serial.println();
#endif

  // QZSS L1Sメッセージ解析
  if (data->gnssId == 5) {

    // SFRBXのdwrdはリトルエンディアンなので入れ替える
    for (int i = 0; i < min(int(data->numWords), 8); i++) {
      l1s_msg_buf[(i << 2) + 0] = (data->dwrd[i] >> 24) & 0xff;
      l1s_msg_buf[(i << 2) + 1] = (data->dwrd[i] >> 16) & 0xff;
      l1s_msg_buf[(i << 2) + 2] = (data->dwrd[i] >> 8) & 0xff;
      l1s_msg_buf[(i << 2) + 3] = (data->dwrd[i]) & 0xff;
    }

    byte pab = l1s_msg_buf[0];
    byte mt = l1s_msg_buf[1] >> 2;

    if (pab == 0x53 || pab == 0x9A || pab == 0xC6) {
      // Message Typeを表示
      struct {
        byte mt;
        const char *desc;
      } MTTable[] = {
        { 0, "Test Mode" },
        { 43, "DC Report" },
        { 44, "DCX message" },
        { 47, "Monitoring Station Information" },
        { 48, "PRN Mask" },
        { 49, "Data Issue Number" },
        { 50, "DGPS Correction" },
        { 51, "Satellite Health" },
        { 63, "Null message" },
      };
      for (int i = 0; i < sizeof(MTTable) / sizeof(MTTable[0]); i++) {
        if (MTTable[i].mt == mt) {
          Serial.print(mt);
          Serial.print(" ");
          Serial.println(MTTable[i].desc);
          break;
        }
      }
      // 災害・危機管理通報サービス（DC Report）のメッセージ内容を表示
      if (mt == 43) {
        dc_report.SetYear(2024);  // todo
        dc_report.Decode(l1s_msg_buf);
        Serial.println(dc_report.GetReport());
      }
      // 災害・危機管理通報サービス（拡張）（DCX）のメッセージ内容を表示
      else if (mt == 44) {
        dcx_decoder.decode(l1s_msg_buf);
        dcx_decoder.printSummary(Serial, dcx_decoder.r);
#if DBG_PRINT_DCX_ALL
        dcx_decoder.printAll(Serial, dcx_decoder.r);
#endif
      }
    }
  }
}

void newNAVSAT(UBX_NAV_SAT_data_t *data) {

#define NUM_GNSS 7
  int nGNSS[NUM_GNSS] = { 0 };
  for (uint16_t block = 0; block < data->header.numSvs; block++) {
    if (data->blocks[block].gnssId < NUM_GNSS) {
      nGNSS[data->blocks[block].gnssId]++;
    }
  }
  Serial.print(F("Satellites: "));
  Serial.print(data->header.numSvs);
  const char *gnssName[] = { "GPS", "SBAS", "Galileo", "BeiDou", "IMES", "QZSS", "GLONASS" };
  for (uint16_t i = 0; i < NUM_GNSS; i++) {
    if (nGNSS[i]) {
      Serial.print(" ");
      Serial.print(gnssName[i]);
      Serial.print(": ");
      Serial.print(nGNSS[i]);
    }
  }
  Serial.println();

#if DBG_PRINT_SAT
  // 衛星の種類を表示
  for (uint16_t block = 0; block < data->header.numSvs; block++) {
    switch (data->blocks[block].gnssId) {
      case 0:
        Serial.print(F("GPS     "));
        break;
      case 1:
        Serial.print(F("SBAS    "));
        break;
      case 2:
        Serial.print(F("Galileo "));
        break;
      case 3:
        Serial.print(F("BeiDou  "));
        break;
      case 4:
        Serial.print(F("IMES    "));
        break;
      case 5:
        Serial.print(F("QZSS    "));
        break;
      case 6:
        Serial.print(F("GLONASS "));
        break;
      default:
        Serial.print(F("UNKNOWN "));
        break;
    }

    // 衛星番号を表示
    Serial.print(data->blocks[block].svId);

    if (data->blocks[block].svId < 10) Serial.print(F("   "));
    else if (data->blocks[block].svId < 100) Serial.print(F("  "));
    else Serial.print(F(" "));

    // 信号の強さを表示
    for (uint8_t cno = 0; cno < data->blocks[block].cno; cno++)
      Serial.print(F("|"));

    Serial.println();
  }
#endif
}

void newNAVPVT(UBX_NAV_PVT_data_t *data) {
#if DBG_PRINT_PVT
  // 時刻表示
  Serial.print(F("Time: "));
  uint8_t hms = data->hour;
  if (hms < 10) Serial.print(F("0"));
  Serial.print(hms);
  Serial.print(F(":"));
  hms = data->min;
  if (hms < 10) Serial.print(F("0"));
  Serial.print(hms);
  Serial.print(F(":"));
  hms = data->sec;
  if (hms < 10) Serial.print(F("0"));
  Serial.print(hms);
  Serial.print(F("."));
  unsigned long millisecs = data->iTOW % 1000;
  if (millisecs < 100) Serial.print(F("0"));
  if (millisecs < 10) Serial.print(F("0"));
  Serial.print(millisecs);

  // 経度・緯度・高度表示
  long latitude = data->lat;
  Serial.print(F(" Lat: "));
  Serial.print(latitude);

  long longitude = data->lon;
  Serial.print(F(" Long: "));
  Serial.print(longitude);
  Serial.print(F(" (degrees * 10^-7)"));

  long altitude = data->hMSL;
  Serial.print(F(" Height above MSL: "));
  Serial.print(altitude);
  Serial.print(F(" (mm)"));

  Serial.print(F(" SIV: "));
  Serial.print(data->numSV);

  Serial.print(F(" Fix: "));
  Serial.print(data->fixType);

  Serial.println();
#endif
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  //ターミナルを開くまでまつ

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

  myGNSS.setUART1Output(COM_TYPE_UBX);           // UART1から出力をUBXのみに設定する
  enableQZSSL1S();                               // QZSS L1S信号の受信を有効にする
  myGNSS.setAutoRXMSFRBXcallbackPtr(&newSFRBX);  // UBX-RXM-SFRBXメッセージ受信コールバック関数を登録
  myGNSS.setAutoNAVSATcallbackPtr(&newNAVSAT);   // UBX-NAV-SATメッセージ受信コールバック関数を登録
  myGNSS.setAutoPVTcallbackPtr(&newNAVPVT);      // UBX-NAV-PVTメッセージ受信コールバック関数を登録
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

void loop() {
  myGNSS.checkUblox();
  myGNSS.checkCallbacks();
}
