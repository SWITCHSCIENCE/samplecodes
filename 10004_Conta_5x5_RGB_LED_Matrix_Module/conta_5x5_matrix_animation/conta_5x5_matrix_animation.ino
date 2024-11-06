// Raspberry Pi PicoとRaspberry Pi Pico用 Conta™ベースボード（ESP-WROOM-02搭載）を利用
// MatrixモジュールはM1ソケットに接続
// このサンプルはAdafruit_NeoPixelライブラリのサンプルコード strandtest_wheel.ino をベースにしています。

#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 25
#define PIN 18

// パラメータ1 = ストリップ内のピクセル数
// パラメータ2 = Arduinoのピン番号
// パラメータ3 = ピクセルの種類と設定
//   NEO_KHZ800  800KHzのビットストリーム（ほとんどのNeoPixel製品）
//   NEO_KHZ400  400KHz（古いFLORAやWS2811ドライバなど）
//   NEO_GRB     GRB配列（ほとんどのNeoPixel製品）
//   NEO_RGB     RGB配列（古いFLORAピクセル）
//   NEO_RGBW    RGBW配列（NeoPixel RGBW製品用）
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// 注意：NeoPixelの寿命を延ばすため、電源リードに1000uFのコンデンサを追加し、
// 最初のピクセルのデータ入力に300 - 500オームの抵抗を追加し、
// Arduinoと最初のピクセル間の距離を最小限に抑える。ライブ回路への接続を避け、接続する場合はGNDから接続。

void setup() {
  pixels.begin();            // NeoPixelの初期化
  pixels.setBrightness(50);  // 明るさを50に設定
  pixels.show();             // すべてのピクセルをオフに初期化
}

void loop() {
  // 以下はピクセルに表示する例の手順
  colorWipe(pixels.Color(255, 0, 0), 50);  // 赤
  colorWipe(pixels.Color(0, 255, 0), 50);  // 緑
  colorWipe(pixels.Color(0, 0, 255), 50);  // 青
  //colorWipe(pixels.Color(0, 0, 0, 255), 50); // 白 (RGBW用)

  // シアターライト風の追尾効果
  theaterChase(pixels.Color(127, 127, 127), 50);  // 白
  theaterChase(pixels.Color(127, 0, 0), 50);      // 赤
  theaterChase(pixels.Color(0, 0, 127), 50);      // 青

  rainbow(20);              // レインボー効果
  rainbowCycle(20);         // レインボーサイクル効果
  theaterChaseRainbow(50);  // レインボーのシアター追尾効果
}

// 1つずつドットを色で埋めていく
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {  // 全ピクセルを順に処理
    pixels.setPixelColor(i, c);                        // i番目のピクセルに指定の色を設定
    pixels.show();                                     // ピクセルの色をハードウェアに送信
    delay(wait);                                      // 次のピクセルの点灯まで待機
  }
}

// レインボー効果
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {  // 色相を変化させてループ
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 255));  // 各ピクセルに異なる色を設定
    }
    pixels.show();
    delay(wait);  // 次の色の表示まで待機
  }
}

// レインボー効果を均等に分布させる
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) {  // 全色のホイールを5回サイクル
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));  // 各ピクセルに色を設定
    }
    pixels.show();
    delay(wait);
  }
}

// シアタースタイルのクロールライト
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) {  // 10回繰り返す
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, c);  // 3つごとにピクセルを点灯
      }
      pixels.show();

      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, 0);  // 3つごとにピクセルを消灯
      }
    }
  }
}

// レインボー効果のシアタークロールライト
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {  // ホイールの全256色をサイクル
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, Wheel((i + j) % 255));  // 3つごとにピクセルを点灯
      }
      pixels.show();

      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, 0);  // 3つごとにピクセルを消灯
      }
    }
  }
}

// 0～255の値を入力し、色を取得する
// 赤-緑-青-赤に戻る色の遷移
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
