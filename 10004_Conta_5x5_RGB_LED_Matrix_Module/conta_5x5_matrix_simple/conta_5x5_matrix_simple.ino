// Raspberry Pi PicoとRaspberry Pi Pico用 Conta™ベースボード（ESP-WROOM-02搭載）を利用
// MatrixモジュールはM1ソケットに接続
// このサンプルはAdafruit_NeoPixelライブラリのサンプルコード simple.ino をベースにしています。

#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 25
#define PIN 18

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup(void) {
  pixels.begin();
}

void loop(void) {
  pixels.clear();  // すべてのピクセルをオフにする

  // 最初のNeoPixelは#0、次は1というように、ピクセルの数-1まで繰り返す
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color()はRGB値を設定。0,0,0から255,255,255までの範囲
    // ここでは適度に明るい緑色を使用
    pixels.setPixelColor(i, pixels.Color(0, 16, 0));

    pixels.show();  // 更新されたピクセルの色をハードウェアに送信する

    delay(500);  // 次のループまで500ミリ秒待機
  }
}
