#include <Wire.h>
#include <SparkFun_u-blox_GNSS_v3.h>  //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
#include <QZQSM.h> //https://github.com/baggio63446333/QZQSM
#include "QZSSDCX.h" //https://github.com/SWITCHSCIENCE/QZSSDCX

SFE_UBLOX_GNSS myGNSS;

#define DBG_PRINT_SFRBX 0
#define DBG_PRINT_SAT 0
#define DBG_PRINT_PVT 1
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
        dcx_decode0.printAll(Serial, dcx_decoder.r);
#endif
      }
    }
  }
}

void printWithDigits(long number, int digits = 1) {
  // 数値の桁数を計算
  int numDigits = 0;
  long temp = number;
  if (temp == 0) {
    numDigits = 1;  // 数値が0の場合は1桁とする
  } else {
    while (temp != 0) {
      temp /= 10;
      numDigits++;
    }
  }

  // ゼロ埋めのための先頭のスペースを計算して出力
  for (int i = 0; i < digits - numDigits; i++) {
    Serial.print(' ');
  }

  // 数値を出力
  Serial.print(number);
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
    printWithDigits(data->blocks[block].svId, 3);

    // 方位角
    Serial.print(" Az ");
    printWithDigits(data->blocks[block].azim, 3);

    // 信号クオリティ
    Serial.print(" Ql ");
    Serial.print(data->blocks[block].flags.bits.qualityInd);

    // ヘルスを表示
    Serial.print(" Hl ");
    Serial.print(data->blocks[block].flags.bits.health);

    // エフェメリス
    Serial.print(" eph ");
    Serial.print(data->blocks[block].flags.bits.ephAvail);

    // アルマナック
    Serial.print(" alm ");
    Serial.print(data->blocks[block].flags.bits.almAvail);

    Serial.print(" ");

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

  Serial.print(F(" DGNSS: "));
  Serial.print(data->flags.bits.diffSoln);

  Serial.println();
#endif
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  //ターミナルを開くまでまつ

  Wire.setPins(0, 1);  // ESPr Developer C6用のI2Cピン設定
  Wire.begin();

  // myGNSS.enableDebugging(); // デバッグメッセージを出力する
  if (myGNSS.begin() == false)  //Wireポート経由でu-bloxモジュールに接続
  {
    Serial.println(F("デフォルトのI2Cアドレスでu-blox GNSSが検出されません。配線を確認してください。停止します。"));
    while (1)
      ;
  }

  myGNSS.setI2COutput(COM_TYPE_UBX);             // I2Cから出力をUBXのみに設定する
  myGNSS.setAutoRXMSFRBXcallbackPtr(&newSFRBX);  // UBX-RXM-SFRBXメッセージ受信コールバック関数を登録
  myGNSS.setAutoNAVSATcallbackPtr(&newNAVSAT);   // UBX-NAV-SATメッセージ受信コールバック関数を登録
  myGNSS.setAutoPVTcallbackPtr(&newNAVPVT);      // UBX-NAV-PVTメッセージ受信コールバック関数を登録
}

void loop() {
  myGNSS.checkUblox();
  myGNSS.checkCallbacks();
  delay(100);
}
