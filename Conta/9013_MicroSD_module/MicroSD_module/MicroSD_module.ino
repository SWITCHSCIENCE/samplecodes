/*
●SKU9013 Conta SD カードスロットモジュール 動作確認用サンプルコード

●対応ベースボード：
  ・SKU 3929 : SPRESENSE用 ベースボード

  ・SKU 7161 : Raspberry Pi Pico用 ベースボード

●ソフトウェア環境：
  ・フレームワークとしてArduinoを使用する。
  ・Raspberry Pi Pico の場合は earlephilhower 版のフレームワークを使用する。
    https://github.com/earlephilhower/arduino-pico

  ・ライブラリとして SdFat が必要。 ( https://github.com/greiman/SdFat )
*/

/// ContaベースボードのM2,M4を使う場合はこの定数を有効にする。
/// コメントアウトすると M1,M3を使用する。
//#define USE_CONTA_SOCKET_M2_M4

#include <Arduino.h>

#if !defined ( SS )
/// 定数 SS が設定されていない場合SdFatのヘッダがエラーを出すので仮の値を指定する。
#define SS 10
#endif

#include <SdFat.h>    /// https://github.com/greiman/SdFat

#if defined(ARDUINO_ARCH_RP2040)
/// for RP2040 earlephilhower's Arduino framework

 #if defined (USE_CONTA_SOCKET_M2_M4)
/// for M2, M4
  #define PERI_SPI spi0
  #define PIN_SPI_SCK   2
  #define PIN_SPI_MOSI  3
  #define PIN_SPI_MISO  4
  #define PIN_SPI_CS_SD 5
 #else
/// for M1,M3
  #define PERI_SPI spi1
  #define PIN_SPI_SCK   10
  #define PIN_SPI_MOSI  11
  #define PIN_SPI_MISO  12
  #define PIN_SPI_CS_SD 13
 #endif

 static SPIClassRP2040 hw_spi(PERI_SPI, PIN_SPI_MISO, -1, PIN_SPI_SCK, PIN_SPI_MOSI);
 static SdSpiConfig sdspicfg ( PIN_SPI_CS_SD, SHARED_SPI, 25000000, &hw_spi );

#else

 #if defined (ARDUINO_ARCH_SPRESENSE)
/// for SPRESENSE

  #if defined (USE_CONTA_SOCKET_M2_M4)
/// for M2, M4
   #define PIN_SPI_CS_SD 7
  #else
/// for M1, M3
   #define PIN_SPI_CS_SD 8
  #endif
  #define PIN_LED LED0

 #else

  #if defined (USE_CONTA_SOCKET_M2_M4)
/// for M2, M4
   #define PIN_SPI_CS_SD 9
  #else
/// for M1, M3
   #define PIN_SPI_CS_SD 10
  #endif

 #endif

 #define PERI_SPI SPI

 static SdSpiConfig sdspicfg ( PIN_SPI_CS_SD, SHARED_SPI, 25000000, &PERI_SPI );

#endif

static SdFat sd_fat;

#if defined ( PIN_LED )
static bool led;
#endif


/// 再帰的にディレクトリ内のファイルを開く関数。
static void openRecursive(FsFile& file)
{
#if defined ( PIN_LED )
  digitalWrite(PIN_LED, led);
  led = !led;
#endif

  int filesize = file.size();
  char strbuf[256];
  file.getName(strbuf, sizeof(strbuf));

/// ファイル名およびサイズをシリアルモニタに表示する。
  Serial.print(strbuf);
  Serial.print(" : ");
  Serial.println(filesize);

  if (file.isDirectory())
  {
/// ディレクトリの場合は、openRecursive再帰呼出しによってサブディレクトリを開く。
    FsFile sub;
    while (sub = file.openNextFile())
    {
      openRecursive(sub);
    }
  }
  else
  {
/// フォルダの場合は、先頭32バイトをシリアルモニタに表示する。
    uint8_t buffer[16];
    if (filesize)
    {
      if (filesize > 32) { filesize = 32; }
      do
      {
        int len = (filesize > 16) ? 16 : filesize;
        file.readBytes((char*)buffer, len);
        for (int i = 0; i < len; ++i)
        {
          snprintf(strbuf, sizeof(strbuf), "%02x ", buffer[i]);
          Serial.print(strbuf);
        }
        Serial.print(": ");
        for (int i = 0; i < len; ++i)
        {
          Serial.print((char)buffer[i]);
        }
        Serial.println();
        filesize -= len;
      } while (filesize);
    }
    file.close();
  }
}

void setup(void)
{
  Serial.begin(115200);
  delay(1024);
  Serial.println("sd begin");

#if defined ( PIN_LED )
  pinMode(PIN_LED, OUTPUT);
#endif

  while (!sd_fat.begin(sdspicfg))
  {
#if defined ( PIN_LED )
    digitalWrite(PIN_LED, led);
    led = !led;
#endif
    Serial.print(".");
    delay(512);
  }
  Serial.println("done.");
}

void loop(void)
{
  delay(1024);

  FsFile root = sd_fat.open("/");
  openRecursive(root);
}
