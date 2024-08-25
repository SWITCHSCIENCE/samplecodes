#include <Arduino.h>

// PicossciNTSCライブラリ
#include <Picossci_NTSC.h>

#include <pico.h>
#include <stdlib.h>
#include <math.h>

// 8x8pixelのフォントデータ
#include "Font8x8C64.h"

/// このプログラムで出力されるNTSC信号の周波数は、RP2040のCPUクロックに依存して誤差が出ます。
/// CPUクロックがいくつであってもNTSC仕様の周波数に近い周波数が出せるように自動調整されますが、
/// NTSC仕様と完全に一致する出力ができる CPUクロックは 157.5MHz (またはその半分の78.75MHz)のみです。
/// これは、NTSC信号の出力に使用しているPIOの動作クロック分周比がCPUクロックに依存しているためです。

/// 以下はCPUクロックの設定行です。
/// NTSC信号出力の周波数仕様 3.579545 MHz との誤差が少ない順に並んでいます。
/// この中から希望するCPUクロックの行をコメント解除し有効化してご利用ください。
//                                        CPUクロック  NTSC出力周波数  NTSC仕様との誤差
#define CONFIG_CPU_FREQ_KHZ 157500   // 157.5 MHz   3579545.454 Hz  誤差   0.000 Hz
// #define CONFIG_CPU_FREQ_KHZ  78750   //  78.75MHz   3579545.454 Hz  誤差   0.000 Hz
// #define CONFIG_CPU_FREQ_KHZ 160800   // 160.8 MHz   3579547.826 Hz  誤差   2.371 Hz
// #define CONFIG_CPU_FREQ_KHZ 147600   // 147.6 MHz   3579537.703 Hz  誤差 - 7.750 Hz
// #define CONFIG_CPU_FREQ_KHZ 116000   // 116.0 MHz   3579556.412 Hz  誤差  10.958 Hz
// #define CONFIG_CPU_FREQ_KHZ 174000   // 174.0 MHz   3579556.412 Hz  誤差  10.958 Hz
// #define CONFIG_CPU_FREQ_KHZ 232000   // 232.0 MHz   3579556.412 Hz  誤差  10.958 Hz
// #define CONFIG_CPU_FREQ_KHZ 141000   // 141.0 MHz   3579531.931 Hz  誤差 -13.522 Hz
// #define CONFIG_CPU_FREQ_KHZ 282000   // 282.0 MHz   3579531.931 Hz  誤差 -13.522 Hz
// #define CONFIG_CPU_FREQ_KHZ 187200   // 187.2 MHz   3579563.788 Hz  誤差  18.333 Hz
// #define CONFIG_CPU_FREQ_KHZ  89600   //  89.6 MHz   3579525.593 Hz  誤差 -19.861 Hz
// #define CONFIG_CPU_FREQ_KHZ 134400   // 134.4 MHz   3579525.593 Hz  誤差 -19.861 Hz
// #define CONFIG_CPU_FREQ_KHZ 224000   // 224.0 MHz   3579525.593 Hz  誤差 -19.861 Hz
// #define CONFIG_CPU_FREQ_KHZ 268800   // 268.8 MHz   3579525.593 Hz  誤差 -19.861 Hz
// #define CONFIG_CPU_FREQ_KHZ 190500   // 190.5 MHz   3579565.472 Hz  誤差  20.018 Hz
// #define CONFIG_CPU_FREQ_KHZ 207000   // 207.0 MHz   3579573.088 Hz  誤差  27.633 Hz
// #define CONFIG_CPU_FREQ_KHZ 213600   // 213.6 MHz   3579575.805 Hz  誤差  30.350 Hz
// #define CONFIG_CPU_FREQ_KHZ  83000   //  83.0 MHz   3579514.824 Hz  誤差 -30.629 Hz
// #define CONFIG_CPU_FREQ_KHZ 124500   // 124.5 MHz   3579514.824 Hz  誤差 -30.629 Hz
// #define CONFIG_CPU_FREQ_KHZ 166000   // 166.0 MHz   3579514.824 Hz  誤差 -30.629 Hz
// #define CONFIG_CPU_FREQ_KHZ 249000   // 249.0 MHz   3579514.824 Hz  誤差 -30.629 Hz
// #define CONFIG_CPU_FREQ_KHZ 121200   // 121.2 MHz   3579510.844 Hz  誤差 -34.610 Hz
// #define CONFIG_CPU_FREQ_KHZ 242400   // 242.4 MHz   3579510.844 Hz  誤差 -34.610 Hz
// #define CONFIG_CPU_FREQ_KHZ 240000   // 240.0 MHz   3579585.178 Hz  誤差  39.723 Hz
// #define CONFIG_CPU_FREQ_KHZ 266400   // 266.4 MHz   3579592.693 Hz  誤差  47.239 Hz
// #define CONFIG_CPU_FREQ_KHZ  91000   //  91.0 MHz   3579594.345 Hz  誤差  48.890 Hz
// #define CONFIG_CPU_FREQ_KHZ 182000   // 182.0 MHz   3579594.345 Hz  誤差  48.890 Hz
// #define CONFIG_CPU_FREQ_KHZ 273000   // 273.0 MHz   3579594.345 Hz  誤差  48.890 Hz
// #define CONFIG_CPU_FREQ_KHZ 108000   // 108.0 MHz   3579492.490 Hz  誤差 -52.963 Hz
// #define CONFIG_CPU_FREQ_KHZ 216000   // 216.0 MHz   3579492.490 Hz  誤差 -52.963 Hz
// #define CONFIG_CPU_FREQ_KHZ  97600   //  97.6 MHz   3579598.853 Hz  誤差  53.399 Hz
// #define CONFIG_CPU_FREQ_KHZ 124000   // 124.0 MHz   3579612.088 Hz  誤差  66.633 Hz
// #define CONFIG_CPU_FREQ_KHZ 248000   // 248.0 MHz   3579612.088 Hz  誤差  66.633 Hz
// #define CONFIG_CPU_FREQ_KHZ 133000   // 133.0 MHz   3579478.553 Hz  誤差 -66.901 Hz
// #define CONFIG_CPU_FREQ_KHZ 199500   // 199.5 MHz   3579478.553 Hz  誤差 -66.901 Hz
// #define CONFIG_CPU_FREQ_KHZ 266000   // 266.0 MHz   3579478.553 Hz  誤差 -66.901 Hz
// #define CONFIG_CPU_FREQ_KHZ  94800   //  94.8 MHz   3579469.026 Hz  誤差 -76.427 Hz
// #define CONFIG_CPU_FREQ_KHZ 158000   // 158.0 MHz   3579469.026 Hz  誤差 -76.427 Hz
// #define CONFIG_CPU_FREQ_KHZ 189600   // 189.6 MHz   3579469.026 Hz  誤差 -76.427 Hz
// #define CONFIG_CPU_FREQ_KHZ  91500   //  91.5 MHz   3579462.102 Hz  誤差 -83.351 Hz
// #define CONFIG_CPU_FREQ_KHZ 183000   // 183.0 MHz   3579462.102 Hz  誤差 -83.351 Hz
// #define CONFIG_CPU_FREQ_KHZ 190000   // 190.0 MHz   3579629.084 Hz  誤差  83.629 Hz
// #define CONFIG_CPU_FREQ_KHZ 271200   // 271.2 MHz   3579459.682 Hz  誤差 -85.772 Hz
// #define CONFIG_CPU_FREQ_KHZ 208000   // 208.0 MHz   3579456.843 Hz  誤差 -88.611 Hz
// #define CONFIG_CPU_FREQ_KHZ 256000   // 256.0 MHz   3579637.317 Hz  誤差  91.862 Hz
// #define CONFIG_CPU_FREQ_KHZ 258000   // 258.0 MHz   3579449.382 Hz  誤差 -96.072 Hz

static Picossci_NTSC picossci_ntsc;

static constexpr const size_t frame_buf_width = 480;
static constexpr const size_t frame_buf_height = 320;
static uint8_t frame_buffer[frame_buf_width * frame_buf_height];

static uint8_t scene_index = 0;


// サンプルコード内で三角関数を利用したラスタースクロールのために、
// テーブルを使った独自のsin関数を用意しています。
static int fast_sin(int i) {
  static constexpr const uint16_t fast_sin_tbl[32] = {
      0x0000,0x0C9C,0x1931,0x25B5,0x3223,0x3E71,0x4A98,0x5691,
      0x6254,0x6DDA,0x791C,0x8413,0x8EB7,0x9903,0xA2EF,0xAC77,
      0xB593,0xBE3E,0xC673,0xCE2D,0xD566,0xDC1B,0xE247,0xE7E6,
      0xECF6,0xF172,0xF558,0xF8A5,0xFB58,0xFD6E,0xFEE7,0xFFC2,
  };
  i = (i & 127);
  if (i >= 64) return -fast_sin(i - 64);
  if (i >= 32) i = (63 - i);
  return fast_sin_tbl[i];
}

// バッファに空きが出来た時に呼び出されるコールバック関数
// この関数はバッファに空きが出来た時に呼び出されます。
static void callback_makeImage(void* arg)
{
  // テスト画像の作成バッファ
  static uint32_t line_buffer[180];

  // frame_count は走査線が先頭に戻るたびにインクリメントして使用しています。
  static uint8_t frame_count = 0;

  for (;;) {
    // 要求されているY座標を取得します。
    int y = picossci_ntsc.video.getCurrentY();

    // getCurrentYが負の値の場合は画面外またはバッファの空きがない状況です。
    if (y < 0) { return; }

    // Yの値が正の値の場合、１ライン分の画像を用意して writeScanLine関数を呼び出すことで画面に表示できます。

    // writeScanLine関数で画像の書込みを行う度に、yの値は2進みます。
    // 具体的には 0, 2, 4, 6 …の順で進み、478の次は 1, 3, 5, 7 …の順で進みます。479の次は0に戻ります。
    // 偶数フィールドが一巡したあと奇数フィールドを一巡します。1秒間に60回のフィールド更新が行われます。

    // 走査線ひとつあたりの処理時間は、30usec以内に収めることを推奨します。
    // これより実行時間が長すぎる場合、バッファの作成が間に合わず、映像が乱れることがあります。

    if (y < 2) {
      // Yの値が 0 の場合は偶数フィールドの先頭位置、 1 の場合は奇数フィールドの先頭位置です。
      // この例では先頭位置に戻る度にフレームカウントを加算します。
      frame_count++;
      // フレームカウントが256に達してオーバーフローし 0に戻った際にシーンサンプルを切り替えます。
      if (frame_count == 0) {
        scene_index = (scene_index + 1) % 10;
      }
    }

    // scene_sample によって表示する画像を切り替えます。
    switch (scene_index) {

  //----------------------------------------------------------------
    case 0: // カラーバー テストパターン

      if (y < 2)
      {
        // RGB332モードを設定
        picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_rgb332);

        // 表示倍率の設定。
        // 例 : 256==等倍表示 , 512==2倍表示 , 384== 1.5倍表示
        // ここでは720ピクセルの表示範囲に8ピクセルを拡大表示するので倍率は90倍となります。
        picossci_ntsc.video.setScale(256 * 720 / 8);

        // X方向の表示位置オフセットの設定。
        // ここでは0を指定し、横方向のオフセットなしとします。
        // (表示位置をずらしたい場合は、プラスで右、マイナスで左に位置がずれます)
        picossci_ntsc.video.setOffset(0);

        // 7ピクセル分のカラーバーを作成する
        auto buf = (uint8_t*)line_buffer;
        for (int j = 0; j < 8; ++j) {
          int r = (j & 2) ? 0 : 255;
          int g = (j & 4) ? 0 : 255;
          int b = (j & 1) ? 0 : 255;
          uint8_t color = (r >> 5) << 5 | (g >> 5) << 2 | b >> 6;
          buf[j] = color;
        }
      }
      // 走査線画像データを書き込みます。
      // 先の設定により90倍に引き延ばして表示されるため、画面全体にカラーバーが表示されます。
      picossci_ntsc.video.writeScanLine(line_buffer, 8);
      break;

  //----------------------------------------------------------------
    case 1: // 表示オフセット変更
      // case 0 の時の画像をそのまま使い、表示位置を変更してみます。
      if (y < 2)
      {
        // オフセットをサイン波で変化させてみます。
        // このサンプル内にあるfast_sin関数の戻り値は -65535 ～ +65535 の範囲です。
        // 128倍したあと16ビット右シフトして、-128 ～ +128 の範囲に変換しています。
        picossci_ntsc.video.setOffset(fast_sin(frame_count << 1) * 128 >> 16);
      }
      picossci_ntsc.video.writeScanLine(line_buffer, 8);
      break;

  //----------------------------------------------------------------
    case 2: // 表示倍率変更
      // case 0 の時の画像をそのまま使い、表示倍率を変更してみます。
      if (y < 2)
      {
        // オフセットは0とします。
        picossci_ntsc.video.setOffset(0);

        // 表示倍率をサイン波で変化させてみます。
        picossci_ntsc.video.setScale(256 * (90 + (fast_sin(frame_count << 1) * 80 >> 16)));
      }
      picossci_ntsc.video.writeScanLine(line_buffer, 8);
      break;

  //----------------------------------------------------------------
    case 3: // 走査線単位での表示倍率と表示オフセット変更
      // case 0 の時の画像をそのまま使い、走査線単位で表示倍率と表示オフセットを変更してみます。
      {
        int scale = (90 << 8) + (((y - 240) * (fast_sin(frame_count << 1) * 80 >> 16)));
        picossci_ntsc.video.setScale(scale);
        picossci_ntsc.video.setOffset(360 - (scale * 4 >> 8));
        picossci_ntsc.video.writeScanLine(line_buffer, 8);
      }
      break;

  //----------------------------------------------------------------
    case 4: // RGB332で表示可能な256色のカラーマップ表示
      {
        int y_idx = y / 30;
        int fraction = y - (y_idx * 30);
        if (fraction < 2) {
          if (y < 2)
          {
            // RGB332モードを設定
            picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_rgb332);

            // info->x_offset = 0;
            picossci_ntsc.video.setOffset(0);

            // info->scale = 720 * 256 / 16;
            picossci_ntsc.video.setScale(720 * 256 / 16);

            // データサイズは16pixelに設定
            // info->length = 16;
          }
        // RGB332カラーマップを作成する
          auto buf = (uint8_t*)line_buffer;
          for (int x = 0; x < 16; ++x) {
            int r = x & 7;
            int g = y_idx & 7;
            int b = x >> 3 | (y_idx >> 3) << 1;
            uint8_t color = r << 5 | g << 2 | b;
            buf[x] = color;
          }
        }
        picossci_ntsc.video.writeScanLine(line_buffer, 16);
      }
      break;

  //----------------------------------------------------------------
    case 5: // RGB565表示
      if (y < 2)
      {
        // RGB565モードを設定
        picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_rgb565);

        picossci_ntsc.video.setScale(720 * 256 / 256);
        picossci_ntsc.video.setOffset(0);
      }

      // 256pixel分のデータを作成して書き込む
      {
        auto buf = (uint16_t*)line_buffer;
        int b = (y<<8) / 480;
        for (int x = 0; x < 256; ++x) {
          int r = x >> 3;
          int g = (((x + b) >> 3) + frame_count) & 0x3F;
          buf[x] = r << 11 | g << 5 | b >> 3;
        }
      }
      picossci_ntsc.video.writeScanLine(line_buffer, 256);

      break;

  //----------------------------------------------------------------
    case 6: // 256パレット表示
    // なお loop関数側でパレット値の変更処理を行っています。
    // ここではパレット値の変更処理は行いません。
      {
        int y_idx = y / 30;
        int fraction = y - (y_idx * 30);
        if (fraction < 2) {
          if (y < 2)
          {
            // パレットモードを設定
            picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_palette);
            picossci_ntsc.video.setOffset(0);
            picossci_ntsc.video.setScale(720 * 256 / 16);
          }
        // 16x16=256色マップ表示を作成する
          auto buf = (uint8_t*)line_buffer;
          for (int x = 0; x < 16; ++x) {
            int palette_index = (x << 4) + y_idx;
            buf[x] = palette_index;
          }
        }
        picossci_ntsc.video.writeScanLine(line_buffer, 16);
      }
      break;

  //----------------------------------------------------------------
    case 7: // 走査線の位置に応じてモードを切り替える例
      {
        size_t idx = y / 160;
        size_t fraction = y - (idx * 160);

        if (idx == 0) {
        // 上 1/3 は RGB332 を表示
          if (fraction < 2) {
            picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_rgb332);
            picossci_ntsc.video.setScale(720 * 256 / 7);
          }
          auto buf = (uint8_t*)line_buffer;
          auto mag = fraction * 256 / 160;
          for (int j = 0; j < 8; ++j) {
            int r = (j & 2) ? 0 : mag;
            int g = (j & 4) ? 0 : mag;
            int b = (j & 1) ? 0 : mag;
            uint8_t color = (r >> 5) << 5 | (g >> 5) << 2 | b >> 6;
            buf[j] = color;
          }
          picossci_ntsc.video.writeScanLine(line_buffer, 7);
        }
        else if (idx == 1) {
        // 中央 1/3 は RGB565 を表示
          if (fraction < 2) {
            picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_rgb565);
            picossci_ntsc.video.setScale(720 * 256 / 7);
          }
          auto buf = (uint16_t*)line_buffer;
          auto mag = fraction * 256 / 160;
          for (int j = 0; j < 8; ++j) {
            int r = (j & 2) ? 0 : mag;
            int g = (j & 4) ? 0 : mag;
            int b = (j & 1) ? 0 : mag;
            uint16_t color = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3;
            buf[j] = color;
          }
          picossci_ntsc.video.writeScanLine(line_buffer, 7);
        }
        else {
        // 下 1/3 は パレットモード を表示
          if (fraction < 2) {
            picossci_ntsc.video.setPixelMode(Picossci_NTSC::pixel_mode_t::pixel_palette);
            picossci_ntsc.video.setScale(720 * 256 / 8);
          }
          auto mag = (fraction * 256 / 160) >> 3;
          auto buf = (uint8_t*)line_buffer;
          for (int j = 0; j < 8; ++j) {
            buf[j] = mag + (j << 5);
          }
          picossci_ntsc.video.writeScanLine(line_buffer, 8);
        }
      }
      break;

  //----------------------------------------------------------------
    default: // フレームバッファの画像をそのまま表示する例
      if (y < 2) {
        picossci_ntsc.video.setPixelMode(sizeof(frame_buffer[0]) == 1
                        ? Picossci_NTSC::pixel_mode_t::pixel_rgb332
                        : Picossci_NTSC::pixel_mode_t::pixel_rgb565);
        picossci_ntsc.video.setScale((720 * 256) / frame_buf_width);
      }

      // くねくね動くようにオフセットを設定
      picossci_ntsc.video.setOffset(fast_sin(y+frame_count) * 8 >> 16);

      // フレームバッファへはloop関数内で作画処理を行う。
      // コールバック関数内では、走査線位置に応じたフレームバッファのポインタを設定するだけでよい。
      picossci_ntsc.video.writeScanLine(&frame_buffer[frame_buf_width * (y * frame_buf_height / 480)], frame_buf_width);

      break;
    }
  }
}


//----------------------------------------------------------------

void setup(void)
{
  // CPUクロックの変更処理
#if defined (CONFIG_CPU_FREQ_KHZ)
  picossci_ntsc.setCpuClock(CONFIG_CPU_FREQ_KHZ);
#endif

  // 映像用の設定
  auto video_cfg = picossci_ntsc.video.getConfig();

  /// 使用するPIOの番号 (0または1 を指定。初期値は0)
  // video_cfg.pio_index = 0;

  /// 画像バッファに空きが出来た時に呼び出されるコールバック関数を指定します。
  video_cfg.callback_function = callback_makeImage;

  /// コールバック関数を呼び出す際に何らかのデータを受け取りたい場合、callback_paramに指定できます。
  // video_cfg.callback_param = nullptr;

  // 作画処理が間に合わず映像が乱れる場合はdma_countを大きくすることで解消する場合があります。
  // (初期値4, 設定値に比例してメモリ消費が増えます)
  video_cfg.dma_buf_count = 8;

  // 信号の出力電圧を上げたい場合はoutput_levelを高く設定します。(0~255 , 初期値128)
  // video_cfg.output_level = 192;

  // 映像の彩度が弱い場合はchroma_levelを高く設定します。(0~255 , 初期値128)
  // video_cfg.chroma_level = 192;

  // ※ output_levelとchroma_levelを共に極端に高い値に設定した場合、出力が飽和して色がおかしくなる可能性があります。

  // 映像出力を開始します。
  picossci_ntsc.video.init(video_cfg);
  picossci_ntsc.video.start();


  // 音声用の設定
  auto audio_cfg = picossci_ntsc.audio.getConfig();

  // DMA転送バッファサイズ (バイト数)
  // 16ビットステレオでの出力となるので 4の倍数を設定します。
  // 例: 192 を指定した場合 16bitステレオデータで48サンプル分のデータを保持できます。
  // つまり freq_hz が 48kHzのとき、48サンプルあれば 1msec 分のデータを保持できることになります。
  audio_cfg.dma_buf_size = 192;

  // DMA転送バッファの数 (上記のバッファサイズ * バッファ数 が DMA転送バッファの総容量となる)
  // 16を指定した場合、上記のバッファサイズが1msecであれば、合計 16msec分のデータをDMA用に保持できることになります。
  audio_cfg.dma_buf_count = 16;

  // 上記の設定値が大きいほど音切れが起きにくくなりますが、音声の遅延は大きくなり、メモリ消費も増えます。

  // 音声出力を開始します。
  picossci_ntsc.audio.init(audio_cfg);
  picossci_ntsc.audio.start();
}

// 音データサンプル (左チャンネル)
static constexpr const uint16_t wave0[32] = {
  0x0000, 0x1000, 0x1800, 0x1A00, 0x1800, 0x1000, 0x0000, 0xF000,
  0xE800, 0xE600, 0xE800, 0xF000, 0x0000, 0x1000, 0x2000, 0x3000,
  0x3800, 0x3A00, 0x3800, 0x3000, 0x2000, 0x1000, 0x0000, 0xF000,
  0xE000, 0xD800, 0xDA00, 0xD800, 0xD000, 0xE000, 0xF000, 0xF800,
};

// 音データサンプル (右チャンネル)
static constexpr const uint16_t wave1[32] = {
  0x0000, 0x0200, 0x0400, 0x0600, 0x0800, 0x0A00, 0x0C00, 0x0E00,
  0x1000, 0x1200, 0x1400, 0x1600, 0x1800, 0x1A00, 0x1C00, 0x1E00,
  0xE000, 0xE200, 0xE400, 0xE600, 0xE800, 0xEA00, 0xEC00, 0xEE00,
  0xF000, 0xF200, 0xF400, 0xF600, 0xF800, 0xFA00, 0xFC00, 0xFE00,
};

void loop(void)
{
  // 音データの書込み可能なバッファサイズを取得
  // バイト数が返ってくるので、int16_t型のサンプルリングデータの数に変換するため半分にする
  // ステレオで書き込むため、奇数の端数分は除去する
  int remain = (picossci_ntsc.audio.availableForWrite() >> 1) & ~1;

  // 音データのバッファにある程度の空きが出来て入れば音データを書き込む
  if (remain > 64) {
    static uint16_t left_step;
    static uint16_t right_step;
    static uint16_t left_add = 17;
    static uint16_t right_add = 256;

    if (remain > 256) {
      remain = 256;
    }
    int16_t buffer[256];
    // 音データを作成
    for (int i = 0; i < remain; i += 2) {
      // 左チャンネル
      buffer[i + 0] = wave0[(left_step >> 8) & 31];

      left_step += left_add;
      if (left_step < left_add) {
        if (++left_add > 512) {
          left_add -= 512;
        }
      }

      // 右チャンネル
      buffer[i + 1] = wave1[(right_step >> 8) & 31];

      right_step += right_add;
      if (right_step < right_add) {
        if (--right_add < 16) {
          right_add += 512;
        }
      }
    }
    // 音データを書き込み
    // サンプル数をバイト数に変換するため remain 2倍する
    picossci_ntsc.audio.write(buffer, remain << 1);
  }
  else
  {
    switch (scene_index) {
    default:
      delay(1);
      break;

    case 6:
    case 7:
      // カラーパレットを動的に変更する例
      {
        static uint8_t palette_index;
        static uint32_t counter;
        int rv = 256 - abs((int)((counter +  85)&255) - 128);
        int gv = 256 - abs((int)((counter + 170)&255) - 128);
        int bv = 256 - abs((int) (counter       &255) - 128);
        rv = rv < 0 ? 0 : rv > 256 ? 256 : rv;
        gv = gv < 0 ? 0 : gv > 256 ? 256 : gv;
        bv = bv < 0 ? 0 : bv > 256 ? 256 : bv;
        do {
          uint8_t r = palette_index * rv >> 8;
          uint8_t g = palette_index * gv >> 8;
          uint8_t b = palette_index * bv >> 8;
          picossci_ntsc.video.setPalette(palette_index, r<<16|g<<8|b);
          if (++palette_index == 0) {
            counter += 3;
            break;
          }
          // 音データのバッファサイズに空きが出来るまでループする。
          // (setPaletteがそこそこ重い処理のため、音途切れを生じさせないためバッファの空き状況を確認している)
        } while (picossci_ntsc.audio.availableForWrite() < 128);
      }
      break;

    case 8:
      // フレームバッファに矩形を描画する例
      for (int k = 0; k < 8; ++k) {
        uint32_t r = rand();
        int x = (r >> 15) % (frame_buf_width - 8);
        int y = (r >> 22) % (frame_buf_height - 8);
        size_t idx = y * frame_buf_width + x;
        for (int i = 0; i < 8; ++i) {
          for (int j = 0; j < 8; ++j) {
            frame_buffer[idx + i * frame_buf_width + j] = r;
          }
        }
      }
      break;

    case 9:
      // フレームバッファにランダムにフォントを描画する例
      for (int k = 0; k < 8; ++k) {
        uint32_t r = rand();
        int x = r % (frame_buf_width >> 3);
        int y = (r >> 8) % (frame_buf_height >> 3);
        int c = (r >> 16) % 112;

        uint16_t colors[2];
        colors[0] = 0;
        colors[1] = r >> 16;
        auto charnametbl = &font8x8_c64[c*8];
        for (int i = 0; i < 8; ++i) {
          auto ptn = charnametbl[i];
          for (int j = 0; j < 8; ++j) {
            frame_buffer[(y * 8 + j) * frame_buf_width + x * 8 + i] = colors[ptn & 1];
            ptn >>= 1;
          }
        }
      }
      break;
    }
  }
}
