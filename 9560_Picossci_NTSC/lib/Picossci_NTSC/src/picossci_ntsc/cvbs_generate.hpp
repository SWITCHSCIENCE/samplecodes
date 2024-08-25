#ifndef PICOSSCI_NTSC_CVBS_GENERATE_HPP_
#define PICOSSCI_NTSC_CVBS_GENERATE_HPP_

#include <stdint.h>
#include <stddef.h>

namespace ns_picossci_ntsc
{
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
    pixel_swap565 = 3,
    pixel_mode_max = 4,
  };

  static uint8_t getPixelModeByteSize(pixel_mode_t mode);

  struct config_t {
    // 出力信号の方式
    signal_type_t signal_type = signal_type_t::NTSC_J;

    // luminance_gain default:128  0=no signal
    uint8_t output_level = 128;

    // default:128  0=monochrome
    uint8_t chroma_level = 128;
  };

  bool init(const config_t &config);

  bool enablePixelMode(pixel_mode_t);
  bool disablePixelMode(pixel_mode_t);
  void setPalette(uint8_t index, uint32_t rgb888);
  void setPalette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) { setPalette(index, r<<16|g<<8|b); }

  config_t getConfig(void) const { return _config; }

  const signal_spec_info_t* getSignalSpec(void) const { return _spec_info; }

  void setScale(uint16_t x_scale) { _x_scale = x_scale; }
  void setOffset(int32_t x_offset) { _x_offset = x_offset; }
  void setPixelMode(pixel_mode_t mode) { _pixel_mode = mode; }

  // syncパルスをバッファに生成する。戻り値で映像表示位置を返す。
  int make_sync(uint8_t* buf, uint16_t scanline, uint16_t buffer_cycle, bool odd_even);
  void make_scanline(uint8_t* dst, const void* image, size_t pixel_count);
protected:
  // void make_linebuffer(uint8_t* buf, uint16_t scanline);
  // void write_buffer(uint8_t* buf, const line_image_info_t* info);

  const signal_spec_info_t* _spec_info = nullptr;
  config_t _config;

  // 表示倍率 x256 (256 = 等倍 , 512 = 2倍 , 768 = 3倍)
  int32_t _x_offset;
  uint16_t _x_scale;
  pixel_mode_t _pixel_mode;      // 表示色モード

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
//static void setup_palette_ntsc_888(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level);
  static void setup_palette_ntsc_base(internal_data_t* data, const signal_spec_info_t* spec, uint_fast8_t signal_level);
};
//--------------------------------------------------------------------------------
} // namespace picossci_ntsc

#endif
