#include <Arduino.h>

// PicossciNTSCライブラリ
#include <Picossci_NTSC.h>

#include <pico/stdlib.h>
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

static picossci_ntsc_t picossci_ntsc;

static constexpr const size_t frame_buf_width = 480;
static constexpr const size_t frame_buf_height = 320;
static uint8_t frame_buffer[frame_buf_width * frame_buf_height];

static uint8_t scene_index = 0;

// 走査線単位で呼び出されるコールバック関数 (走査線毎に表示したい画像データをこの関数内で呼び出し元に渡す)
static void callback_makeImage(cvbs_generate_t::line_image_info_t* info, uint16_t y)
{
// この関数は走査線単位で呼び出されます。
// NTSCの走査線は合計525本ですが、画面外の走査線に対しては呼び出されず、480本分の走査線を対象とします。
// 呼び出し毎にyの値は2進みます。つまり 0, 2, 4, 6 …の順で進み、478の次は 1, 3, 5, 7 …の順で進みます。479の次は0に戻ります。
// 偶数フィールドが一巡したあと奇数フィールドを一巡します。合計で480ライン分の呼び出しが秒間30回行われます。

// コールバック一回あたりの処理時間は、30us以内に収めることを推奨します。
// コールバック関数の実行時間が長すぎる場合、映像が乱れることがあります。
// このサンプルプログラムでは、コールバック関数内にさまざまな処理を記述していますが、
// 実際の運用上では、コールバック関数の外で画像を作成しておき、
// コールバック関数内では作成済みの画像データを渡すだけに留めることを推奨します。

  // テスト画像の作成バッファ
  static uint32_t line_buffer[180];

  // frame_countは走査線が先頭に戻るたびにインクリメントされます。
  static uint8_t frame_count = 0;
  if (y < 2) {
    // Yの値が0または1のとき、つまり偶数と奇数フィールドの最初の走査線のときにフレームカウントを加算します。
    frame_count++;
    // フレームカウントが256に達してオーバーフローし0に戻った際にシーンサンプルを切り替えます。
    if (frame_count == 0) {
      scene_index = (scene_index + 1) % 10;
    }
  }

  // コールバックの呼び出し元に渡す画像データのポインタを設定します。
  // この例では、line_bufferに画像を作成してそれを渡すようにしています。
  info->image = line_buffer;

  // scene_sample によって表示する画像を切り替えます。
  switch (scene_index) {

//----------------------------------------------------------------
  case 0: // カラーバー テストパターン

    if (y < 2)
    { // ※ line_image_info_t構造体の設定値は前回のコールバック呼び出し時のものが引き継がれる。
      //    ここでは Y==0のとき毎回設定しているが、最初に一度設定すれば以後は設定しなくてもよい。

      // RGB332モードを設定
      info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_rgb332;

      // データサイズは8pixelに設定。 (倍率設定にて引き延ば表示します)
      info->length = 8;

      // 拡大倍率の整数部分を設定。 720ピクセルの表示範囲に8ピクセルを拡大表示するので倍率は90倍となる
      info->scale_number = 720 / 8;

      // 拡大倍率の小数部分を設定。0でよい。
      info->scale_fractional = 0;

      // 拡大倍率は unionで定義されており、上記の代わりに info->scale で設定することもできます。
      // scale で設定する場合、倍率の256倍の値を設定します。
      // 例 : 256==等倍表示 , 512==2倍表示 , 384== 1.5倍表示
      // info->scale = 90 * 256;

      // X方向の表示位置オフセットは0
      // (表示位置をずらしたい場合は変更する。プラスで右、マイナスで左に位置がずれる)
      info->x_offset = 0;

      // 8ピクセル分のカラーバーを作成する
      auto buf = (uint8_t*)line_buffer;
      for (int j = 0; j < 8; ++j) {
        int r = (j & 2) ? 0 : 255;
        int g = (j & 4) ? 0 : 255;
        int b = (j & 1) ? 0 : 255;
        uint8_t color = (r >> 5) << 5 | (g >> 5) << 2 | b >> 6;
        buf[j] = color;
      }
    }
    break;

//----------------------------------------------------------------
  case 1: // 表示オフセット変更
    // case 0 の時の画像をそのまま使い、表示位置を変更してみます。
    if (y < 2)
    {
      info->x_offset = sinf(frame_count * 3.1415f / 32) * 128;
    }
    break;

//----------------------------------------------------------------
  case 2: // 表示倍率変更
    // case 0 の時の画像をそのまま使い、表示倍率を変更してみます。
    if (y < 2)
    {
      info->x_offset = 0;
      info->scale = 256 * (90 + sinf(frame_count * 3.1415f / 32) * 80);
    }
    break;

//----------------------------------------------------------------
  case 3: // 走査線単位での表示倍率と表示オフセット変更
    // case 0 の時の画像をそのまま使い、走査線単位で表示倍率と表示オフセットを変更してみます。
    // 画面の下の方を拡大、上の方を縮小表示するように設定してみます。
    info->scale = (90 << 8) + (((y - 240) * sinf(frame_count * 3.1415f / 32) * 80));

    // 画面の中央が中心になるよう、倍率に応じてオフセットを変更してみます。
    // 画像は8ピクセルなので、半分の4の位置を中心に表示されるように設定してみます。
    info->x_offset = 360 - (info->scale * 4 >> 8);
    break;

//----------------------------------------------------------------
  case 4: // RGB332で表示可能な256色のカラーマップ表示
    {
      int y_idx = y / 30;
      int flaction = y - (y_idx * 30);
      if (flaction < 2) {
        if (y < 2)
        {
          // RGB332モードを設定
          info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_rgb332;

          info->x_offset = 0;

          // データサイズは16pixelに設定
          info->length = 16;

          info->scale = 720 * 256 / 16;
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
    }
    break;

//----------------------------------------------------------------
  case 5: // RGB565表示
    if (y < 2)
    {
      // RGB565モードを設定
      info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_rgb565;

      // データサイズは256pixelに設定
      info->length = 256;

      info->scale = 720 * 256 / 256;

      info->x_offset = 0;
    }
    {
      auto buf = (uint16_t*)line_buffer;
      int b = (y<<8) / 480;
      for (int x = 0; x < 256; ++x) {
        int r = x >> 3;
        int g = (x + b) >> 3;
        buf[x] = r << 11 | g << 5 | b >> 3;
      }
    }
    break;

//----------------------------------------------------------------
  case 6: // 256パレット表示
    {
      int y_idx = y / 30;
      int flaction = y - (y_idx * 30);
      if (flaction < 2) {
        if (y < 2)
        {
          // RGB565モードを設定
          info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_palette;

          info->x_offset = 0;

          // データサイズは16pixelに設定
          info->length = 16;

          info->scale = 720 * 256 / 16;

        }
      // 16x16=256色マップ表示を作成する
        auto buf = (uint8_t*)line_buffer;
        for (int x = 0; x < 16; ++x) {
          int palette_index = (x << 4) + y_idx;
          buf[x] = palette_index;
        }
      }

      // カラーパレット色設定
      // ※ setPalette関数は比較的重い処理なので、走査線単位で変更するような使い方は推奨されません。
      //   この例では説明のために走査線毎にパレットを変更していますが、コールバック関数外で行うことを推奨します。
      // この例では処理負荷を時間分散させるため、1走査線あたり1色のみパレットを変更します。
      {
        int rv = 256 - abs((int)((frame_count +  85)&255) - 128);
        int gv = 256 - abs((int)((frame_count + 170)&255) - 128);
        int bv = 256 - abs((int)  frame_count             - 128);
        rv = rv < 0 ? 0 : rv > 256 ? 256 : rv;
        gv = gv < 0 ? 0 : gv > 256 ? 256 : gv;
        bv = bv < 0 ? 0 : bv > 256 ? 256 : bv;
        int i = y*256 / 480;
        uint8_t r = i*rv >> 8;
        uint8_t g = i*gv >> 8;
        uint8_t b = i*bv >> 8;
        picossci_ntsc.setPalette(i, r<<16|g<<8|b);
      }
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
          info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_rgb332;
          info->length = 7;
          info->scale = 720 * 256 / 7;
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
      }
      else if (idx == 1) {
      // 中央 1/3 は RGB565 を表示
        if (fraction < 2) {
          info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_rgb565;
          info->length = 7;
          info->scale = 720 * 256 / 7;
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
      }
      else {
      // 下 1/3 は パレットモード を表示
        if (fraction < 2) {
          info->pixel_mode = cvbs_generate_t::pixel_mode_t::pixel_palette;
          info->length = 8;
          info->scale = 720 * 256 / 8;
        }
        auto mag = (fraction * 256 / 160) >> 3;
        auto buf = (uint8_t*)line_buffer;
        for (int j = 0; j < 8; ++j) {
          buf[j] = mag + (j << 5);
        }
      }
    }
    break;

//----------------------------------------------------------------
  default: // フレームバッファの画像をそのまま表示する例
    if (y < 2) {
      info->pixel_mode = sizeof(frame_buffer[0]) == 1
                       ? cvbs_generate_t::pixel_mode_t::pixel_rgb332
                       : cvbs_generate_t::pixel_mode_t::pixel_rgb565;
      info->scale = (720 * 256) / frame_buf_width;
      info->length = frame_buf_width;
      info->x_offset = 0;
    }
    // フレームバッファへはloop関数内で作画処理を行う。
    // コールバック関数内では、走査線位置に応じたフレームバッファのポインタを設定するだけでよい。
    info->image = &frame_buffer[frame_buf_width * (y * frame_buf_height / 480)];

    // くねくね動くようにオフセットを設定
    info->x_offset = sinf((y+frame_count) * 3.1415f / 64) * 8;

    break;
  }
}


//----------------------------------------------------------------

void setup(void)
{
#if defined (CONFIG_CPU_FREQ_KHZ)
  set_sys_clock_khz(CONFIG_CPU_FREQ_KHZ, false);
#endif
  auto cfg = picossci_ntsc.getConfig();

  /// 使用するPIOの番号 (0または1 を指定する)
  cfg.pio_index = 0;

  /// 信号出力するGPIO番号(8本の先頭の番号を指定する。0を指定した場合はGPIO0～7の合計8本が使用される)
  // cfg.pin_num = 0;

  /// コールバック関数を呼び出す際に何らかのポインタを受け取りたい場合は指定する
  // cfg.callback_param = nullptr;

  /// 走査線単位で呼び出されるコールバック関数を指定する
  cfg.callback_requestImage = callback_makeImage;

  // 作画処理が間に合わず映像が乱れる場合はdma_countを大きくすることで解消する場合があります。
  // (初期値3, 設定値に比例してメモリ消費が増えます)
  cfg.dma_count = 8;

  // 信号の出力電圧を上げたい場合はoutput_levelを高く設定します。(0~255 , 初期値128)
  // cfg.output_level = 192;

  // 映像の彩度が弱い場合はchroma_levelを高く設定します。(0~255 , 初期値128)
  // cfg.chroma_level = 192;

  // ※ output_levelとchroma_levelを共に極端に高い値に設定した場合、出力が飽和して色がおかしくなる可能性があります。

  picossci_ntsc.init(cfg);
  picossci_ntsc.start();
}

void loop(void)
{
  vTaskDelay(1);
  switch (scene_index) {
  default:
    break;

  case 8:
    // フレームバッファに矩形を描画する例
    for (int k = 0; k < 16; ++k) {
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
    for (int k = 0; k < 16; ++k) {
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
