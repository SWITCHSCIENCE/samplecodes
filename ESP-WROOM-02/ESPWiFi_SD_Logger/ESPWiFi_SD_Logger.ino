/*
  ESPWiFi_SD_Logger.ino
  BME280のセンサの値を10分ごとにSDカードに記録する
  記録したデータはHTTPを立てて外部からブラウザ経由で見れる
  http://mag.switch-science.com/

  SDcardディレクトリの中身をESP-WROOM-02に接続するSDカードのルートディレクトリにコピーしてください
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Time.h>
#include <TimeLib.h>

#include "NTP.h"
#include "SDlogger.h"
#include "WebServer.h"
#include "BME280.h"

#define UTC_TOKYO +9


char ssid[] = "***********";  //  your network SSID (name)
char pass[] = "***********";       // your network password

unsigned long whenCountStarted = 0;

double temp,press,hum;

char date_ym[8]; // yyyy-nn0
char date_mdhm[13];  // mm/dd-hh:mmn0

const int period = 10*60*1000; // ログを取る間隔:10分間 (ms)
int ntpAccsessTimes = 0;

unsigned long unixtime;

NTPClient ntp;
BME280 bme280(0x76);
SDlogger sd;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  setSDStatus(sd.init());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ntp.udpSetup();
  WifiServerinit();
  bme280.init();
}

void loop()
{
  handleClient();
  // 約50日ごとにおこるオーバーフロー時はmillsが0になるので引き算の結果はマイナス
  // とりうる値の範囲を考慮して割り算した結果同士を比較 (4,294,967,295)
  if((signed long)(millis()/1000000 - whenCountStarted/1000000) < 0){whenCountStarted = millis();}
  if((millis() - whenCountStarted) > period)
  {
    whenCountStarted = millis();

    // 時間を取得
    if((ntpAccsessTimes==0) && (ntp.getTime(&unixtime)))
    {
      ntpAccsessTimes++;
      if(ntpAccsessTimes >= 48){ ntpAccsessTimes = 0;} // 48回に1回のアクセスとする: 8時間に1回
    }else{
      //時刻取得に失敗 or 取得タイミングでなかったら前回の値から測定間隔分だけ秒数を足す
      unixtime += period/1000;
    }
    time_t t = unixtime + (UTC_TOKYO * 60 * 60); // 日本標準時に調整
    sprintf(date_ym, "%04d-%02d", year(t), month(t));
    sprintf(date_mdhm, "%2d/%02d-%02d:%02d", month(t), day(t),hour(t), minute(t));
    Serial.println(date_ym);
    Serial.println(date_mdhm);

    // センサからデータを取得
    bme280.getSensorData(&temp,&press,&hum);

    // SDカードにデータを記録
    // データを記録するファイル名の指定
    // ファイル名8文字+拡張子3文字までなので注意
    String fileName = "";
    fileName += String(date_ym);
    fileName +=  ".csv";
    // 記録するデータの生成
    String dataStream = "";
    dataStream += String(date_mdhm);
    dataStream += String(',');
    dataStream += String(temp);
    dataStream += String(',');
    dataStream += String(press);
    dataStream += String(',');
    dataStream += String(hum);
    sd.recordData(fileName,dataStream);
  }
}
