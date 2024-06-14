#ifndef PICOSSCI_NTSC_CVBS_GENERATE_HPP_
#define PICOSSCI_NTSC_CVBS_GENERATE_HPP_

#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>

//--------------------------------------------------------------------------------

/// CVBS映像信号を生成するクラス
class cvbs_generate_t
{
public:
  enum signal_type_t : uint8_t
  {
    NTSC,    // black = 7.5IRE
    NTSC_J,  // black = 0IRE (for Japan)
    signal_type_max,     // dummy, can't select this.
  };

  struct signal_spec_info_t
  {
    float subcarrier_freq;
    uint16_t total_scanlines;     // 走査線数(２フィールド、１フレーム);
    uint16_t scanline_width;      // 走査線内のサンプル数 (カラークロック数 x4);
    uint8_t hsync_equalizing;     // 等化パルス幅;
    uint8_t hsync_short;          // 水平同期期間のSYNC幅;
    uint16_t hsync_long;          // 垂直同期期間のSYNC幅;
    uint8_t burst_start;
    uint8_t burst_cycle;          // バースト信号の数;
    uint8_t active_start;
    uint8_t burst_shift_mask;
    uint16_t display_width;       // X方向 表示可能ピクセル数;
    uint16_t display_height;      // Y方向 表示可能ピクセル数;
    int16_t blanking_mv;          // ブランキング期間の電圧
    int16_t black_mv;             // 黒色部の電圧
    int16_t white_mv;             // 白色部の電圧
    uint8_t vsync_start;          // 垂直同期SYNCの開始位置
    uint8_t vsync_end;            // 垂直同期SYNCの終了位置
    uint8_t vsync_lines;          // 垂直同期期間(表示期間外)の走査線数(単フィールド分)
  };

  enum pixel_mode_t : uint8_t {
    pixel_palette = 0,
    pixel_rgb332 = 1,
    pixel_rgb565 = 2,
    pixel_mode_max = 3,
  };

  static uint8_t getPixelModeByteSize(pixel_mode_t mode) { return mode < 1 ? 1 : mode >= (pixel_mode_max-1) ? (pixel_mode_max-1) : mode; }

  /// イメージデータ要求コールバックで使用されるパラメータ構造体
  /// コールバック関数側で中身を更新する。
  struct line_image_info_t
  {
    /// 設定に書かれているコールバック関数に渡すパラメータ
    void* callback_param;
    /// 1ライン分の画像データのポインタ
    const void* image;
    /// 画像データのピクセル数 (X方向)
    uint16_t length;

    union {
      // 表示倍率 x256 (256 = 等倍 , 512 = 2倍 , 768 = 3倍)
      uint16_t scale;
      struct {
        // 表示倍率 (小数部分) (0=0.0 / 64=0.25 / 128=0.5)
        uint8_t scale_fractional;
        // 表示倍率 (整数部分) (1=1.0 / 2=2.0 / 3=3.0)
        uint8_t scale_number;
      };
    };
    union {
      int32_t x_offset;
      struct {
        int16_t offset_fractional;
        int16_t offset_number;    //表示開始位置(左座標)
      };
    };
    pixel_mode_t pixel_mode;      // 表示色モード
  };

  struct config_t {
    // イメージデータ要求コールバック関数
    // info内の各パラメータを更新して返す。
    void (*callback_requestImage)(line_image_info_t* info, uint16_t y);

    // コールバック関数の第一引数
    void* callback_param;

    // 出力信号の方式
    signal_type_t signal_type;

    // luminance_gain default:128  0=no signal
    uint8_t output_level = 128;

    // default:128  0=monochrome
    uint8_t chroma_level = 128;

    // DMA転送用の走査線バッファの数 (最低2)
    uint8_t dma_count = 4;

    // DMAバッファ作成タスクの優先度
    uint8_t task_priority = 10;
  };

  bool init(const config_t &config);

  bool enablePixelMode(pixel_mode_t);
  bool disablePixelMode(pixel_mode_t);
  void setPalette(uint8_t index, uint32_t rgb888);
  void setPalette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) { setPalette(index, r<<16|g<<8|b); }

  config_t getConfig(void) const { return _config; }
  uint16_t getScanline(void) const { return _scanline; }

  const signal_spec_info_t* getSignalSpec(void) const { return _spec_info; }

  /// DMA転送クラスに渡すコールバック
  static bool dma_callback(void* param, uintptr_t dma_buf, size_t dma_len);
protected:
  void make_linebuffer(uint8_t* buf, uint16_t scanline);

  /// DMAバッファ生成用のタスク
  static void task_dma_buffer(cvbs_generate_t* me);

  /// データ要求情報
  struct video_request_t {
    uint8_t* dma_buffer;
    volatile int16_t scanline;
    pixel_mode_t pixel_mode;
  };

  /// データ要求情報の配列
  video_request_t* _video_request = nullptr;

  /// DMA転送割り込み発生時の通知先タスク
  xTaskHandle _task_cvbs = nullptr;

  const signal_spec_info_t* _spec_info = nullptr;
  config_t _config;

  // コールバック関数に渡す情報構造体の実体
  line_image_info_t _line_image_info;

  struct internal_data_t {
    uint32_t burst_wave[2] = {0,0};   // バースト信号 odd+even (NTSCでは1個、PALでは2個使用する)
    uint32_t black_wave = 0;          // 黒レベル信号
    uint32_t blanking_wave = 0;       // ブランキングレベル信号
    uint32_t* palettes[pixel_mode_max] = {nullptr,}; // パレットのポインタ
  };
  internal_data_t _internal_data;
  static void setup_palette_ntsc_gray(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level);
  static void setup_palette_ntsc_332(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level);
  static void setup_palette_ntsc_565(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level);
  static void setup_palette_ntsc_888(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level);
  static void setup_palette_ntsc_base(internal_data_t* data, const signal_spec_info_t* spec, uint_fast8_t signal_level);

  /// 現在の走査線位置
  uint16_t _scanline = 0;
  uint8_t _dma_index = 0;
  /// 現在の映像要求リストの位置
  uint8_t _request_index = 0;
  uint8_t _request_max = 0;
  uint8_t _burst_shift = 0;
};

#endif
