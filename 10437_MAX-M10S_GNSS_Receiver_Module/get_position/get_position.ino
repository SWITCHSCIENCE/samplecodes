#include <Wire.h>  //GNSSとのI2C通信に必要
  
#include <SparkFun_u-blox_GNSS_v3.h>  //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
SFE_UBLOX_GNSS myGNSS;

long lastTime = 0;  //シンプルなローカルタイマー。u-bloxモジュールへのI2C通信頻度を制限するため

void setup() {
  Serial.begin(115200);
  Serial.println("u-blox MAX-M10S テストプログラム");

  Wire.setPins(0, 1);  // ESPr Developer C6用のI2Cピン設定
  Wire.begin();

  //myGNSS.enableDebugging(); // デバッグメッセージをシリアルに出力する場合はこの行のコメントを外す

  if (myGNSS.begin() == false)  //Wireポート経由でu-bloxモジュールに接続
  {
    Serial.println(F("デフォルトのI2Cアドレスでu-blox GNSSが検出されません。配線を確認してください。停止します。"));
    while (1)
      ;
  }

  myGNSS.setI2COutput(COM_TYPE_UBX);  //I2CポートをUBX出力のみに設定（NMEA出力を無効化）
  // myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //通信ポート設定のみをフラッシュとBBRに保存
}

void loop() {
  //1秒ごとにモジュールにクエリを送信。それ以上頻繁にするとI2Cトラフィックが増えるだけ。
  //位置情報が更新されたときのみモジュールは応答する
  if (millis() - lastTime > 1000) {
    lastTime = millis();  //タイマーを更新

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

    byte fixType = myGNSS.getFixType();
    Serial.print(F(" 測位状態: "));
    if (fixType == 0) Serial.print(F("No fix"));
    else if (fixType == 1) Serial.print(F("Dead reckoning"));
    else if (fixType == 2) Serial.print(F("2D"));
    else if (fixType == 3) Serial.print(F("3D"));
    else if (fixType == 4) Serial.print(F("GNSS + Dead reckoning"));
    else if (fixType == 5) Serial.print(F("Time only"));

    long latitude = myGNSS.getLatitude();
    Serial.print(F(" 緯度: "));
    Serial.print((double)latitude/10000000.0, 7);

    long longitude = myGNSS.getLongitude();
    Serial.print(F(" 経度: "));
    Serial.print((double)longitude/10000000.0, 7);

    long altitude = myGNSS.getAltitude();
    Serial.print(F(" 高度: "));
    Serial.print((double)altitude/1000, 3);
    Serial.print(F(" m"));

    byte SIV = myGNSS.getSIV();
    Serial.print(F(" 可視衛星数: "));
    Serial.print(SIV);

    Serial.println();
  }
}