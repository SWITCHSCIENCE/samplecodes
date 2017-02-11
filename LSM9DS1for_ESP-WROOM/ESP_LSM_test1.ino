//-----------------------------------------------------------------
//        ESP2866用　LSM9DS1　9軸センサーシールド
//　　　　　　　　　　　　　　　　　テストプログラム　
//
//　
//　　　　　　　　　　　　　　　　Ver.1
//
//　　参考URL:https://www.mgo-tec.com/blog-entry-ss-wroom-howto01.html
//-----------------------------------------------------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>                      //https://github.com/esp8266/Arduino
#include <Wire.h>                             //I2Cライブラリ
#include <SparkFunLSM9DS1.h>                  //https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library

#define ADAddr 0x48

//-------------------------------------------------------------------------
float gxVal = 0;                                //ジャイロｘ軸用データーレジスタ
float gyVal = 0;                                //ジャイロｙ軸用データーレジスタ
float gzVal = 0;                                //ジャイロｚ軸用データーレジスタ
float axVal = 0;                                //Axis ｘ用データーレジスタ
float ayVal = 0;                                //Axis ｙ用データーレジスタ
float azVal = 0;                                //Axis ｚ用データーレジスタ
float mxVal = 0;                                //Mag x 用データーレジスタ
float myVal = 0;                                //Mag ｙ 用データーレジスタ
float mzVal = 0;                                //Mag x 用データーレジスタ
float hedVal = 0;                               //Hedding 用データーレジスタ

//------------------------------------------------------------------------

LSM9DS1 imu;

#define LSM9DS1_M  0x1E // SPIアドレス設定 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // SPIアドレス設定 if SDO_AG is LOW

#define PRINT_CALCULATED
#define PRINT_SPEED 250 // 250 ms between prints
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

//ご自分のルーターのSSIDを入力してください
const char* ssid = "****";
//ご自分のルーターのパスワード
const char* password = "****";

boolean Ini_html_on = false;//ブラウザからの初回HTTPレスポンス完了したかどうかのフラグ

WiFiServer server(80);
WiFiClient client;
//----------------------　setup　---------------------------
void setup() {
  Serial.begin(115200);

  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());


  if (!imu.begin())                                     //センサ接続エラー時の表示

  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1)
      ;
  }

}

//-------------------------　メインループ　--------------------------------
void loop() {
  if (Ini_html_on == false) {
    Ini_HTTP_Response();
  } else if (client.available()) {
    Serial.print(client.read());
  }
  delay(1);//これは重要かも。これがないと動作かも。
}

//*****初回ブラウザからのGET要求によるHTMLタグ吐き出しHTTPレスポンス*******
void Ini_HTTP_Response()
{
  client = server.available();//クライアント生成
  delay(1);
  String req;

  while (client) {
    if (client.available()) {
      req = client.readStringUntil('\n');
      Serial.println(req);
      if (req.indexOf("GET / HTTP") != -1) { //ブラウザからリクエストを受信したらこの文字列を検知する
        Serial.println("-----from Browser FirstTime HTTP Request---------");
        Serial.println(req);
        //ブラウザからのリクエストで空行（\r\nが先頭になる）まで読み込む
        while (req.indexOf("\r") != 0) {
          req = client.readStringUntil('\n');//\nまで読み込むが\n自身は文字列に含まれず、捨てられる
          Serial.println(req);
        }
        req = "";
        delay(10);//10ms待ってレスポンスをブラウザに送信

        //メモリ節約のため、Fマクロで文字列を囲う
        //普通のHTTPレスポンスヘッダ
        client.print(F("HTTP/1.1 200 OK\r\n"));
        client.print(F("Content-Type:text/html\r\n"));
        client.print(F("Connection:close\r\n\r\n"));//１行空行が必要
        //ここからブラウザ表示のためのHTML JavaScript吐き出し

        //--------------------------------------------------------
        String str_gxVal;
        String str_gyVal;
        String str_gzVal;
        String str_axVal;
        String str_ayVal;
        String str_azVal;
        String str_mxVal;
        String str_myVal;
        String str_mzVal;

        str_gxVal = gxVal;
        str_gyVal = gyVal;
        str_gzVal = gzVal;
        str_axVal = axVal;
        str_ayVal = ayVal;
        str_azVal = azVal;
        str_mxVal = mxVal;
        str_myVal = myVal;
        str_mzVal = mzVal;
        //--------------------------------------------------------


        client.println(F("<!DOCTYPE html>"));
        client.println(F("<html>"));
        client.println(F("<font size=30>"));

        client.println(F("<br>"));
        client.println(F("LSM9DS1Motionsenser"));
        client.println(F("<br>"));
        client.print("G: " + str_gxVal + " " + str_gyVal + " " + str_gzVal + "deg/s");
        client.println(F("<br>"));
        client.print("A: " + str_axVal + " " + str_ayVal + " " + str_azVal + "g");
        client.println(F("<br>"));
        client.print("M: " + str_mxVal + " " + str_myVal + " " + str_mzVal + "gaus");
        client.println(F("<br>"));
        client.print(F("<meta http-equiv= refresh content=0.5>"));

        delay(1);//これが重要！これが無いと切断できないかもしれない。
        client.stop();//一旦ブラウザとコネクション切断する。

        Serial.println("\nGET HTTP client stop--------------------");
        req = "";
        Ini_html_on = false;  //初回HTTPレスポンス終わったらtrueにする。
      }
    }

  }

  printGyro();  // Print "G: gx, gy, gz"　　　シリアルモニタ表示用フォーマット
  printAccel(); // Print "A: ax, ay, az"
  printMag();   // Print "M: mx, my, mz"
  printAttitude(imu.ax, imu.ay, imu.az, -imu.my, -imu.mx, imu.mz);
  Serial.println();

  delay(PRINT_SPEED);
}

//******************************　Gyro DATA ****************************
void printGyro()
{

  imu.readGyro();

  Serial.print("G: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcGyro(imu.gx), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gy), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");

  //------------　測位データ　Gyro/x,y,z　-----------

  gxVal = (imu.calcGyro(imu.gx));
  gyVal = (imu.calcGyro(imu.gy));
  gzVal = (imu.calcGyro(imu.gz));

#elif defined PRINT_RAW
  Serial.print(imu.gx);
  Serial.print(", ");
  Serial.print(imu.gy);
  Serial.print(", ");
  Serial.println(imu.gz);
#endif

}
//-------------------　Accel DATA ----------------------
void printAccel()
{
  // To read from the accelerometer, you must first call the
  // readAccel() function. When this exits, it'll update the
  // ax, ay, and az variables with the most current data.
  imu.readAccel();

  Serial.print("A: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcAccel(imu.ax), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.ay), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.az), 2);
  Serial.println(" g");
//------------　測位データ　Gyro/x,y,z　-----------

  axVal = (imu.calcGyro(imu.ax));
  ayVal = (imu.calcGyro(imu.ay));
  azVal = (imu.calcGyro(imu.az));

#elif defined PRINT_RAW
  Serial.print(imu.ax);
  Serial.print(", ");
  Serial.print(imu.ay);
  Serial.print(", ");
  Serial.println(imu.az);
#endif

}
//--------------　Mag DATA ------------------
void printMag()
{

  imu.readMag();
  Serial.print("M: ");
#ifdef PRINT_CALCULATED

  Serial.print(imu.calcMag(imu.mx), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.my), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.mz), 2);
  Serial.println(" gauss");
//------------　測位データ　Gyro/x,y,z　-----------

  mxVal = (imu.calcGyro(imu.mx));
  myVal = (imu.calcGyro(imu.my));
  mzVal = (imu.calcGyro(imu.mz));

#elif defined PRINT_RAW
  Serial.print(imu.mx);
  Serial.print(", ");
  Serial.print(imu.my);
  Serial.print(", ");
  Serial.println(imu.mz);

#endif

}
//-----------------------------------------------------------------------------
void printAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
  float roll = atan2(ay, az);
  float pitch = atan2(-ax, sqrt(ay * ay + az * az));
  float heading;
  if (my == 0)
    heading = (mx < 0) ? 180.0 : 0;
  else
    heading = atan2(mx, my);

  heading -= DECLINATION * PI / 180;

  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);
  else if (heading < 0) heading += 2 * PI;

  // Convert everything from radians to degrees:
  heading *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll  *= 180.0 / PI;

  Serial.print("Pitch, Roll: ");
  Serial.print(pitch, 2);
  Serial.print(", ");
  Serial.println(roll, 2);
  Serial.print("Heading: ");
  Serial.println(heading, 2);

}


