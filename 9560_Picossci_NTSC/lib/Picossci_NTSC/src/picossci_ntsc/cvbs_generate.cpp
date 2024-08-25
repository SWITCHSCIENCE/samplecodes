// #define GPIO_DEBUG_PIN 20
// #include <hardware/gpio.h>

#include "cvbs_generate.hpp"

#include <math.h>
#include <string.h>

static constexpr const uint8_t SYNC_LEVEL = 0;

namespace ns_picossci_ntsc
{
//--------------------------------------------------------------------------------
  __attribute((optimize("-O3")))
  static uint32_t setup_palette_ntsc_inner(uint32_t rgb, uint_fast8_t diff_level, uint_fast8_t base_level, uint_fast8_t chroma_level)
  {
    uint32_t r = (rgb >> 16) & 0xFF;
    uint32_t g = (rgb >> 8) & 0xFF;
    uint32_t b = rgb & 0xFF;

    uint_fast16_t y256 = r * (0.299f*256) + g * (0.587f*256) + b * (0.114f*256);
    int_fast16_t by = (b << 8) - y256;
    int_fast16_t ry = (r << 8) - y256;
    float i = (by * -0.2680f + ry * 0.7358f) / 256;
    float q = (by *  0.4127f + ry * 0.4778f) / 256;
    y256 = (base_level << 8) + ((y256 * diff_level) >> 8);

    float phase_offset = atan2f(i, q);
    float saturation = (sqrtf(i * i + q * q) * chroma_level) / 2;

    {
      uint8_t buf[4];
      uint8_t frac[4];
      int frac_total = 0;
      for (int j = 0; j < 4; j++)
      {
        int tmp = (int)(y256 + sinf(phase_offset + (float)M_PI / 2 * j) * saturation);
        frac[j] = tmp & 0xFF;
        frac_total += frac[j];
        tmp >>= 8;
        buf[j] = tmp < 0 ? 0 : tmp > 255 ? 255 : tmp;
      }
      // 切り捨てた端数分を補正する
      while (frac_total > 128)
      {
        frac_total -= 256;
        int target_idx = 0;
        const uint8_t idxtbl[] = { 0, 2, 1, 3 };
        for (int j = 1; j < 4; j++)
        {
          if (frac[idxtbl[j]] > frac[target_idx] || frac[target_idx] == 255) {
            target_idx = idxtbl[j];
          }
        }
        if (buf[target_idx] == 255) { break; }
        buf[target_idx]++;
        frac[target_idx] = 0;
      }
      return buf[0] << 0
           | buf[1] << 8
           | buf[2] << 16
           | buf[3] << 24
           ;
    }
  }

  void cvbs_generate_t::setup_palette_ntsc_gray(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level)
  {
    for (int level = 0; level < 256; ++level)
    {
      palette[level] = setup_palette_ntsc_inner(level * 0x010101u, diff_level, black_level, chroma_level);
    }
  }

  void cvbs_generate_t::setup_palette_ntsc_332(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level)
  {
    for (int rgb332 = 0; rgb332 < 256; ++rgb332)
    {
      int r = (( rgb332 >> 5)         * 0x49) >> 1;
      int g = (((rgb332 >> 2) & 0x07) * 0x49) >> 1;
      int b = (( rgb332       & 0x03) * 0x55);
      palette[rgb332] = setup_palette_ntsc_inner(r<<16|g<<8|b, diff_level, black_level, chroma_level);
    }
  }

  void cvbs_generate_t::setup_palette_ntsc_565(uint32_t* palette, uint32_t black_level, uint32_t diff_level, uint_fast8_t chroma_level)
  {
    // テーブル値は上位と下位を合算して出力するため、黒レベルを分割しておく。
    // 下位側のテーブルは青系の色で輝度値が低いため、黒レベルを多めに分配してテーブル値が0未満になるのを防ぐ
    uint32_t black_level0 = black_level >> 3;
    uint32_t black_level1 = black_level - black_level0;
    for (int idx = 0; idx < 256; ++idx)
    {
      { // RGB565の上位1Byteに対するテーブル
        int r = (idx >> 3);
        int g = (idx & 7) << 3;
        r = (r * 0x21) >> 2;
        g = (g * 0x41) >> 4;
        auto data = setup_palette_ntsc_inner(r<<16|g<<8, diff_level, black_level0, chroma_level);
        palette[idx + 256] = data;
      }
      { // RGB565の下位1Byteに対するテーブル
        int g = idx >> 5;
        int b = idx & 0x1F;
        b = (b * 0x21) >> 2;
        g = (g * 0x41) >> 4;
        auto data = setup_palette_ntsc_inner(g<<8|b, diff_level, black_level1, chroma_level);
        palette[idx] = data;
      }
    }
  }

  void cvbs_generate_t::setup_palette_ntsc_base(internal_data_t* data, const signal_spec_info_t* spec, uint_fast8_t output_level)
  {
    uint32_t black_level = spec->black_mv * output_level / spec->white_mv;
    data->black_wave = setup_palette_ntsc_inner(0, 0, black_level, 0);
    uint32_t blanking_level = spec->blanking_mv * output_level / spec->white_mv;
    // 黄色を基準にバースト信号を生成する
    data->burst_wave[0] = setup_palette_ntsc_inner(0x707000, 0, blanking_level, output_level*3>>2);
    data->burst_wave[1] = data->burst_wave[0];
    data->blanking_wave = setup_palette_ntsc_inner(0, 0, blanking_level, 0);
  }

static constexpr const cvbs_generate_t::signal_spec_info_t signal_spec_info_list[]
{ // NTSC
  { 3579545.4545 // サブキャリア周波数3.579545MHz
  , 525         // 走査線525本;
  , 910         // 1走査線あたり 227.5 x4 sample
  , 32          // equalizing = 32 sample (2.3us)
  , 68          // hsync_short = 66 sample (4.7us)
  , 380         // hsync_long = 380 sample
  , 76          // burst start = 76 sample
  , 9           // burst cycle = 9 cycle
  , 148         // active_start = 148 sample (10.8us)
  , 2           // burst_shift_mask バースト信号反転動作;
  , 720         // display width max 720
  , 480         // display height max 480
  , 286         // blanking_mv 286mvを0IREとする
  , 340         // black_mv    黒色部の電圧
  , 1000        // white_mv    白色部の電圧
  , 6, 12       // VSYNCは 6 ~ 12 の位置
  , 22          // vsync_lines
  }
, // NTSC_J
  { 3579545.4545 // サブキャリア周波数3.579545MHz
  , 525         // 走査線525本;
  , 910         // 1走査線あたり 227.5 x4 sample
  , 32          // equalizing = 32 sample (2.3us)
  , 66          // hsync_short = 66 sample (4.7us)
  , 380         // hsync_long = 380 sample
  , 76          // burst start = 76 sample
  , 9           // burst cycle = 9 cycle
  , 148         // active_start = 148 sample (10.8us)
  , 2           // burst_shift_mask バースト信号反転動作;
  , 720         // display width max 720
  , 480         // display height max 480
  , 286         // blanking_mv ブランキング期間の電圧
  , 286         // black_mv    黒色部の電圧 日本仕様の黒はブランキング期間と同一となる
  , 1000        // white_mv    白色部の電圧
  , 6, 12       // VSYNCは 6 ~ 12 の位置
  , 22          // vsync_lines
  }
};


bool cvbs_generate_t::init(const config_t &config)
{
#if defined ( GPIO_DEBUG_PIN )
#if __has_include (<FreeRTOS.h>)
gpio_init(GPIO_DEBUG_PIN);
#else
_gpio_init(GPIO_DEBUG_PIN);
#endif
gpio_set_dir(GPIO_DEBUG_PIN, GPIO_OUT);
#endif

  _config = config;
  auto spec = &signal_spec_info_list[config.signal_type];
  _spec_info = spec;
  setup_palette_ntsc_base(&_internal_data, spec, config.output_level);

  return true;
}

uint8_t cvbs_generate_t::getPixelModeByteSize(pixel_mode_t mode)
{
  static constexpr const uint32_t pixel_mode_bytesize[] = {
    1, 1, 2, 2, 3,
  };
  return (mode < pixel_mode_max) ? pixel_mode_bytesize[mode] : 1;
}

bool cvbs_generate_t::enablePixelMode(pixel_mode_t pixel_mode)
{
  if (_spec_info == nullptr) { return false; }
  if (pixel_mode >= pixel_mode_max) { return false; }
  if (_internal_data.palettes[pixel_mode] == nullptr) {
    auto pixel_size = getPixelModeByteSize(pixel_mode);
    auto palette_size = (_spec_info->burst_shift_mask == 1) ? 512 : 256;

    _internal_data.palettes[pixel_mode] = (uint32_t*)(~3 & (uintptr_t)malloc(4 + sizeof(uint32_t) * pixel_size * palette_size));

    uint32_t black_level = _spec_info->black_mv * _config.output_level / _spec_info->white_mv;
    uint32_t diff_level = _config.output_level - black_level;
    void (*setup_palette_tbl[])(uint32_t*, uint32_t, uint32_t, uint_fast8_t) = {
      setup_palette_ntsc_gray,
      setup_palette_ntsc_332,
      setup_palette_ntsc_565,
      setup_palette_ntsc_565,
    };
    setup_palette_tbl[pixel_mode]( _internal_data.palettes[pixel_mode], black_level, diff_level, _config.chroma_level);
  }
  return true;
}

void cvbs_generate_t::setPalette(uint8_t index, uint32_t rgb888)
{
  if (_internal_data.palettes[pixel_mode_t::pixel_palette] == nullptr) {
    if (!enablePixelMode(pixel_mode_t::pixel_palette)) { return; }
  }
  uint32_t black_level = _spec_info->black_mv * _config.output_level / _spec_info->white_mv;
  uint32_t diff_level = _config.output_level - black_level;
  _internal_data.palettes[pixel_mode_t::pixel_palette][index] = setup_palette_ntsc_inner(rgb888, diff_level, black_level, _config.chroma_level);
}

bool cvbs_generate_t::disablePixelMode(pixel_mode_t pixel_mode)
{
  if (_spec_info == nullptr) { return false; }
  if (pixel_mode < pixel_mode_max && _internal_data.palettes[pixel_mode] != nullptr) {
    free(_internal_data.palettes[pixel_mode]);
    _internal_data.palettes[pixel_mode] = nullptr;
  }
  return true;
}

// アセンブラを呼び出すための関数宣言
extern "C" {
  void cvbs_generate_blit_8bit(uint8_t* d, const uint8_t* d_end, const uint8_t* s, const uint32_t* p, uint32_t scale, int32_t diff);
  void cvbs_generate_blit_16bit(uint8_t* d, const uint8_t* d_end, const uint16_t* s, const uint32_t* p, uint32_t scale, int32_t diff, bool swap);
};

// C版の関数をweakで定義しておく
__attribute((weak))
void cvbs_generate_blit_8bit(uint8_t* __restrict d, const uint8_t* __restrict d_end, const uint8_t* s, const uint32_t* p, uint32_t scale, int32_t diff)
{
  do
  {
    uint8_t pixel_c = s[0];
    // 色が変化する場所まで進める
    do {
      uint8_t pixel_next = s[1];
      ++s;
      diff += scale;
      if (pixel_next != pixel_c) { break; }
    } while (diff < 0);
    uint32_t color = p[pixel_c];

    // アラインメント端数分の処理
    uint32_t align = (((uint32_t)d) << 30) >> 30;
    if (align == 1) { goto LABEL_ALIGN1; }
    if (align < 1) { goto LABEL_ALIGN0; }
    if (align == 2) { goto LABEL_ALIGN2; }

    ((uint8_t*)d)[0] = color >> 24;
    d += 1;
    goto LABEL_ALIGN0;

LABEL_ALIGN1:
    ((uint8_t*)d)[0] = color >> 8;
    d += 1;
LABEL_ALIGN2:
    ((uint16_t*)d)[0] = color >> 16;
    d += 2;

LABEL_ALIGN0:
    auto d_target = d_end + (diff >> 8);
    if (d_target >= d_end) { d_target = d_end; }
    do {
      ((uint32_t*)d)[0] = color;
      ((uint32_t*)d)[1] = color;
      ((uint32_t*)d)[2] = color;
      d += 12;
    } while (d < d_target);
    d = (uint8_t*)d_target;
  } while (d < d_end);
}

__attribute((weak))
void cvbs_generate_blit_16bit(uint8_t* __restrict d, const uint8_t* __restrict d_end, const uint16_t* s, const uint32_t* p, uint32_t scale, int32_t diff, bool swap)
{
  auto pl = p;
  if (swap) { p += 256; }
  else { pl += 256; }
  do
  {
    uint16_t pixel_c = s[0];
    // 色が変化する場所まで進める
    do {
      uint16_t pixel_next = s[1];
      ++s;
      diff += scale;
      if (pixel_next != pixel_c) { break; }
    } while (diff < 0);
    uint32_t color = p[pixel_c & 255];
    pixel_c >>= 8;
    color += pl[pixel_c];

    // アラインメント端数分の処理
    uint32_t align = (((uint32_t)d) << 30) >> 30;
    if (align == 1) { goto LABEL_ALIGN1; }
    if (align < 1) { goto LABEL_ALIGN0; }
    if (align == 2) { goto LABEL_ALIGN2; }

    ((uint8_t*)d)[0] = color >> 24;
    d += 1;
    goto LABEL_ALIGN0;

LABEL_ALIGN1:
    ((uint8_t*)d)[0] = color >> 8;
    d += 1;
LABEL_ALIGN2:
    ((uint16_t*)d)[0] = color >> 16;
    d += 2;

LABEL_ALIGN0:
    auto d_target = d_end + (diff >> 8);
    if (d_target >= d_end) { d_target = d_end; }
    do {
      ((uint32_t*)d)[0] = color;
      ((uint32_t*)d)[1] = color;
      ((uint32_t*)d)[2] = color;
      d += 12;
    } while (d < d_target);
    d = (uint8_t*)d_target;
  } while (d < d_end);
}

int cvbs_generate_t::make_sync(uint8_t* buf, uint16_t scanline, uint16_t buffer_cycle, bool odd_even)
{
  // インターレース込みでの走査線位置を取得;
  int y = scanline << 1;
  // インターレースを外した走査線位置に変換する (奇数フィールドの場合に走査線位置が0基準になるように変換する)
  bool odd_field = y >= _spec_info->total_scanlines;
  if (odd_field) { y -= _spec_info->total_scanlines; }

  int sync_limit = _spec_info->vsync_start + _spec_info->vsync_end;

  if (y >= sync_limit)
  {
    /// DMAリングバッファの前の周回でデータ消去が済んでいなければ処理
    if ((y - sync_limit) <= (buffer_cycle << 1))
    {
      auto l = _spec_info->hsync_short;
      auto r = _spec_info->active_start;
      memset(&buf[l], _internal_data.blanking_wave, r - l);
      memset(buf, SYNC_LEVEL, _spec_info->hsync_short);
      l = _spec_info->active_start;
      memset(&buf[l], _internal_data.black_wave, _spec_info->display_width);

      /// バースト信号の付与
      uint32_t burst_data = _internal_data.burst_wave[odd_even];
      size_t burst_start = _spec_info->burst_start & ~3;
      auto b32 = (uint32_t*)(((uintptr_t)&buf[burst_start]) & ~3);
      for (int i = 0; i < _spec_info->burst_cycle; ++i)
      {
        b32[i] = burst_data;
      }
    }
  }
  else
  {
    auto half_width = _spec_info->scanline_width >> 1;
    for (int j = 0; j < 2; ++j)
    {
      int sink_index = y + j;
      bool long_sink = sink_index > _spec_info->vsync_start && sink_index <= _spec_info->vsync_end;
      int sink_len = long_sink ? _spec_info->hsync_long : _spec_info->hsync_equalizing;

      uint32_t bw = _internal_data.black_wave;

      memset(&buf[(j * half_width + sink_len)], bw, half_width - sink_len);
      auto b = &buf[j*half_width];
      memset(b, SYNC_LEVEL, sink_len);
    }
  }
  y -= (_spec_info->vsync_lines << 1);
  if (y >= _spec_info->display_height) { y -= _spec_info->total_scanlines; }
  return y;
}

void cvbs_generate_t::make_scanline(uint8_t* dst, const void* image, size_t pixel_count)
{
  auto b = &dst[_spec_info->active_start];
  if (image != nullptr && _x_scale != 0) {
    uint32_t initial = 0;
    int_fast32_t dst_right = (pixel_count * _x_scale) >> 8;
    int_fast16_t dst_left = _x_offset;
    if (dst_left) // 出力先のオフセットを反映する
    {
      dst_right += dst_left;
      if (dst_left < 0)
      { // マイナスオフセットを反映する
        initial = -dst_left;
        dst_left = 0;
      }
    }
    int limit = (int)_spec_info->display_width;
    if (dst_right > limit) { dst_right = limit; }
    if (dst_right > dst_left) {
      auto pixel_mode = _pixel_mode;
      auto palette = _internal_data.palettes[pixel_mode];
      if (palette == nullptr) {
        enablePixelMode(pixel_mode);
        palette = _internal_data.palettes[pixel_mode];
      }
      if (palette != nullptr) {

        int32_t diff = (dst_left - dst_right) << 8;
        auto scale = _x_scale;
        int sidx = 0;
        if (initial) {
          auto in8 = initial << 8;
          int s_add = in8 / scale;
          diff += s_add * scale - in8;
          sidx += s_add;
        }
#if defined ( GPIO_DEBUG_PIN )
gpio_put(GPIO_DEBUG_PIN, 1);
#endif
        if (getPixelModeByteSize(pixel_mode) == 1) {
          cvbs_generate_blit_8bit(&b[dst_left], &b[dst_right], &((const uint8_t*)image)[sidx], palette, scale, diff);
        } else
        if (getPixelModeByteSize(pixel_mode) == 2) {
          cvbs_generate_blit_16bit(&b[dst_left], &b[dst_right], &((const uint16_t*)image)[sidx], palette, scale, diff, pixel_mode != pixel_mode_t::pixel_rgb565);
        }
#if defined ( GPIO_DEBUG_PIN )
gpio_put(GPIO_DEBUG_PIN, 0);
#endif
      }
      memset(&b[limit], _internal_data.black_wave, 8);
      if (dst_left) {
        memset(b, _internal_data.black_wave, dst_left);
      }
    } else {
      dst_right = 0;
    }
    auto right_space = limit - dst_right;
    if (right_space > 0) {
      memset(&b[dst_right], _internal_data.black_wave, right_space);
    }
  } else {
    memset((uint16_t*)b, _internal_data.blanking_wave, _spec_info->display_width);
  }
}

//--------------------------------------------------------------------------------
} // namespace picossci_ntsc