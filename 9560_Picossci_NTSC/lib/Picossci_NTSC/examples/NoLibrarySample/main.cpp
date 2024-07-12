#include <Arduino.h>
#include <I2S.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/dma.h>

// Picossci_NTSCのライブラリを使わずに、PIOを使ってNTSC信号と音を出力するサンプル
// このサンプルでは ボードマネージャは earlephilhower/arduino-pico を使用してください。


// PIO ASMで生成した PIO用のコード ---------------------------
#define piontsc_wrap_target 0
#define piontsc_wrap 2

static const uint16_t piontsc_program_instructions[] = {
            //     .wrap_target
    0x8880, //  0: pull   noblock         side 0 [8] 
    0xb027, //  1: mov    x, osr          side 1     
    0x6008, //  2: out    pins, 8         side 0     
            //     .wrap
};

static const struct pio_program piontsc_program = {
    .instructions = piontsc_program_instructions,
    .length = 3,
    .origin = -1,
};

static inline pio_sm_config piontsc_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + piontsc_wrap_target, offset + piontsc_wrap);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

static inline void piontsc_program_init(PIO pio, uint sm, uint offset, uint opin)
{
    pio_sm_config c = piontsc_program_get_default_config(offset);
    sm_config_set_out_pins(&c, opin, 8);
    sm_config_set_out_shift(&c, true, false, 0);
    for (uint pidx=opin; pidx < (opin + 8); pidx++) {
        pio_gpio_init(pio, pidx);
    }
    pio_sm_set_consecutive_pindirs(pio, sm, opin, 8, true);
    sm_config_set_clkdiv(&c, 1);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}
static inline void piontsc_program_stop(PIO pio, uint sm)
{
    pio_sm_set_enabled(pio, sm, false);
}
// ---------------------------


#define PIN_OUTPUT_CVBS  (0)
#define PIN_OUTPUT_DEBUG (21)

#define PIN_SW_1  (24)
#define PIN_SW_2  (23)
#define PIN_SW_3  (22)
#define PIN_LED_1  (25)
#define PIN_LED_2  (14)
#define PIN_LED_3  (15)
#define PIN_I2S_BCLK  (16)
#define PIN_I2S_LRCLK (17)
#define PIN_I2S_DOUT  (18)
const int32_t i2s_sampleRate = 48000;
I2S i2s(OUTPUT);

// フレームバッファ (RGB332形式。1ピクセルあたり1Byte)
#define FRAME_WIDTH 340
#define FRAME_HEIGHT 220
uint8_t frame_buffer[FRAME_WIDTH * FRAME_HEIGHT];

// NTSC出力 1ラインあたりのサンプル数
#define NUM_LINE_SAMPLES 910  // 227.5 * 4

// NTSC出力 走査線数
#define NUM_LINES 525  // 走査線525本

// DMAピンポンバッファ
uint32_t dma_buffer_raw[(NUM_LINE_SAMPLES+3)>>1];
uint8_t* dma_buffer[2] = { (uint8_t*)dma_buffer_raw, &((uint8_t*)dma_buffer_raw)[NUM_LINE_SAMPLES] };

// DMA 1ライン転送完了時に次回使用するバッファのアドレスを格納する変数
uintptr_t dma_looping_addr = (uintptr_t)dma_buffer_raw;

// RGB332 カラーパレット
uint32_t color_tbl[256];

// 映像データ転送用のDMAチャンネル
static int dma_chan_transfer;

// 映像1ライン転送完了時に次ラインに移行する処理用のためのDMAチャンネル
static int dma_chan_looping;

// NTSC 1ライン分の映像データを生成する関数
static void makeDmaBuffer(uint8_t* buf, size_t line_num, uint8_t odd_burst)
{
if (line_num == 0)
  { gpio_put(PIN_OUTPUT_DEBUG, 0); }

  odd_burst = odd_burst ? 2 : 0;

  int y = line_num * 2;
  bool odd_frame = y > NUM_LINES;
  if (odd_frame) {
    y = y - NUM_LINES;
  }

  if (y < 20)
  {
    for (int i = 0; i < 2; ++i)
    {
      int sink_index = y + i;
      if (sink_index < 2) continue;
      bool long_sink = sink_index >= 8 && sink_index <= 13;
      auto b = &buf[i * (NUM_LINE_SAMPLES / 2)];
      int sink_len = 0;
      if (sink_index < 20) {
        sink_len = long_sink ? 380 : 32;
        memset(b, 0, sink_len);
      }
      auto black = 2 << 5;
      memset(&b[sink_len], black, (NUM_LINE_SAMPLES >> 1) - sink_len);
    }
  }
  else
  {
    constexpr const int margin = 20 + ((NUM_LINES - (FRAME_HEIGHT<<1)) >> 1);
    y = y - margin;
    if (y >= 0 && y < (FRAME_HEIGHT<<1)) {
      constexpr const int active_start = (56 + ((NUM_LINE_SAMPLES - (FRAME_WIDTH << 1)) >> 1)) & ~3u;
      auto f = &frame_buffer[(y>>1) * FRAME_WIDTH];
      auto b = (uint16_t*)&buf[active_start];
      int count = (FRAME_WIDTH) >> 2;
      if (odd_burst) {
        do
        {
          b[0] = color_tbl[f[0]] >> 16;
          b[1] = color_tbl[f[1]];
          b[2] = color_tbl[f[2]] >> 16;
          b[3] = color_tbl[f[3]];
          f += 4;
          b += 4;
        } while (--count);
      } else {
        do
        {
          b[0] = color_tbl[f[0]];
          b[1] = color_tbl[f[1]] >> 16;
          b[2] = color_tbl[f[2]];
          b[3] = color_tbl[f[3]] >> 16;
          f += 4;
          b += 4;
        } while (--count);
      }
    }
    else
    { // フレームバッファ範囲外の処理
      int burst_start = 76;
      int burst_end = burst_start + 4 * 9;
      auto black = 2 << 5;
      memset(&buf[burst_end], black, NUM_LINE_SAMPLES - burst_end);
      int sink_len = 66;
      memset(buf, 0, sink_len);
      for (int i = burst_start; i < burst_end; ++i)
      {
        buf[i] = (1 + (((1 + i + odd_burst) & 2))) << 5;
      }
    }
  }
  gpio_put(PIN_OUTPUT_DEBUG, 1);
}

static uint32_t setup_palette_ntsc_inner(uint32_t rgb, uint32_t diff_level, uint32_t base_level, float satuation_base, float chroma_scale)
{
  static constexpr float BASE_RAD = (M_PI * 192) / 180; // 2.932153;
  uint8_t buf[4];

  uint32_t r = rgb >> 16;
  uint32_t g = (rgb >> 8) & 0xFF;
  uint32_t b = rgb & 0xFF;

  float y = r * 0.299f + g * 0.587f + b * 0.114f;
  float i = (b - y) * -0.2680f + (r - y) * 0.7358f;
  float q = (b - y) *  0.4127f + (r - y) * 0.4778f;
  y = y * diff_level / 256 + base_level;

  float phase_offset = atan2f(i, q) + BASE_RAD;
  float saturation = sqrtf(i * i + q * q) * chroma_scale;
  saturation = saturation * satuation_base;
  for (int j = 0; j < 4; j++)
  {
    int tmp = ((int)(128.5f + y + sinf(phase_offset + (float)M_PI / 2 * j) * saturation)) >> 8;
    buf[j] = tmp < 0 ? 0 : (tmp > 255 ? 255 : tmp);
  }

  return buf[3] << 24
        | buf[2] << 16
        | buf[1] <<  8
        | buf[0] <<  0
        ;
}

static void setup_palette_ntsc_332(uint32_t* palette, uint_fast16_t white_level, uint_fast16_t black_level, uint_fast8_t chroma_level)
{
  float chroma_scale = chroma_level / 7168.0f;
  float satuation_base = black_level / 2;
  uint32_t diff_level = white_level - black_level;

  for (int rgb332 = 0; rgb332 < 256; ++rgb332)
  {
    int r = (( rgb332 >> 5)         * 0x49) >> 1;
    int g = (((rgb332 >> 2) & 0x07) * 0x49) >> 1;
    int b = (( rgb332       & 0x03) * 0x55);

    palette[rgb332] = setup_palette_ntsc_inner(r<<16|g<<8|b, diff_level, black_level, satuation_base, chroma_scale);
  }
}

static void irq_handler(void) {
  static bool flip = false;
  // dma_channel_set_read_addr(dma_chan, dma_buffer[flip], true);
  dma_looping_addr = (uintptr_t)dma_buffer[flip];
  flip = !flip;

  static size_t scanline = 0;
  if (++scanline >= NUM_LINES) { scanline = 0; }
  makeDmaBuffer((uint8_t*)dma_buffer[flip], scanline, flip);

  dma_hw->ints0 = 1u << dma_chan_transfer;
}

void ntsc_task(void)
{
  for (;;) {
    for (size_t scanline = 0; scanline < NUM_LINES; ++scanline) {
      bool flip = multicore_fifo_pop_blocking();
      makeDmaBuffer((uint8_t*)dma_buffer[flip], scanline, flip);
    }
  }
}

// I2Sオーディオ出力データを生成する
void audio_task(void)
{
  int32_t idx = 16384;
  int add = 1;
  int32_t lidx = 0;
  int32_t ridx = 0;
  for (;;)
  {
    if (!gpio_get(PIN_SW_1)) {
      idx ++;
    } else
    if (!gpio_get(PIN_SW_3)) {
      idx --;
    }
    lidx += (idx * 7) >> 16;
    ridx += (idx * 11) >> 8;
    int l = (abs(128 - (lidx & 255)) - 64) << 8;
    int r = sinf(ridx * M_PI / 65536.0f) * 16384;
    if (!gpio_get(PIN_SW_2)) {
      std::swap(l,r);
    }

    uint32_t o = l << 16 | (r & 0xFFFF);
    while (!i2s.write(o, false)) {}
  }
}

void loop(void)
{
  static uint32_t counter = 0;
  ++counter;

  gpio_put(PIN_LED_1, counter & 0b0010);
  gpio_put(PIN_LED_2, counter & 0b0100);
  gpio_put(PIN_LED_3, counter & 0b1000);

  // 画面に表示するフレームバッファの中身を更新する
  for (size_t y = 72; y < FRAME_HEIGHT; ++y)
  {
    for (size_t x = 0; x < FRAME_WIDTH; ++x)
    {
      uint8_t r = (x-counter)<<1;
      uint8_t g = (y+counter)<<1;
      uint8_t b = (((x+counter) & 0x80) + ((y-counter) & 0x100)) >> 1;
      frame_buffer[x + y * FRAME_WIDTH] = (r & 0xE0) + ((g >> 3) & 0x1C) + (b >> 6);
    }
  }
}

void setup(void)
{
  gpio_init(PIN_OUTPUT_DEBUG);
  gpio_set_dir(PIN_OUTPUT_DEBUG, GPIO_OUT);
  gpio_put(PIN_OUTPUT_DEBUG, 1);

  gpio_init(PIN_LED_1);
  gpio_set_dir(PIN_LED_1, GPIO_OUT);
  gpio_put(PIN_LED_1, 1);

  gpio_init(PIN_LED_2);
  gpio_set_dir(PIN_LED_2, GPIO_OUT);
  gpio_put(PIN_LED_2, 1);

  gpio_init(PIN_LED_3);
  gpio_set_dir(PIN_LED_3, GPIO_OUT);
  gpio_put(PIN_LED_3, 1);

  setup_palette_ntsc_332(color_tbl, 960*64, 286*64, 128);

  // CPUを157.5MHzで動作させる。
  uint32_t freq_khz = 157500;

  // ※ NTSCのカラー信号を1周期4サンプルで出力する。
  // PIOのコードを調整して、1サンプルあたり11サイクルで行うようにしておく。
  // すると 157.5 [MHz] / (11 * 4) = 3.579545 [Hz] となり、 NTSCのカラーバースト周期を正確に再現できる。

  set_sys_clock_khz(freq_khz, false);

  PIO pio = pio0;
  uint sm = pio_claim_unused_sm(pio, true);

  pio_sm_restart(pio, sm);
  for (int i = PIN_OUTPUT_CVBS; i<(PIN_OUTPUT_CVBS+8); ++i) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_OUT);
  }
  uint offset = pio_add_program(pio, &piontsc_program);
  piontsc_program_init(pio, sm, offset, PIN_OUTPUT_CVBS);

  dma_chan_transfer = dma_claim_unused_channel(true);
  dma_chan_looping  = dma_claim_unused_channel(true);

  { // NTSC信号を出力するためのDMAチャンネル設定
    auto dma_chan_config = dma_channel_get_default_config(dma_chan_transfer);
    channel_config_set_transfer_data_size(&dma_chan_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_chan_config, true);
    channel_config_set_write_increment(&dma_chan_config, false);
    channel_config_set_dreq(&dma_chan_config, DREQ_PIO0_TX0 + sm);
    channel_config_set_chain_to(&dma_chan_config, dma_chan_looping);
    dma_channel_configure(
        dma_chan_transfer,
        &dma_chan_config,
        &pio->txf[sm],
        dma_buffer[0],
        NUM_LINE_SAMPLES, // NTSC 1ライン分
        false // ここでは転送を開始しない
    );
  }

  { // NTSC映像信号を1ライン分送信を終えた時に、次のラインに切り替えるためのDMAチャンネル設定
    auto dma_chan_config = dma_channel_get_default_config(dma_chan_looping);
    channel_config_set_transfer_data_size(&dma_chan_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_chan_config, false);
    channel_config_set_write_increment(&dma_chan_config, false);
    channel_config_set_chain_to(&dma_chan_config, dma_chan_transfer);
    dma_channel_configure(
        dma_chan_looping,
        &dma_chan_config,
        &(dma_hw->ch[dma_chan_transfer].read_addr),
        &dma_looping_addr,
        1,
        true // 転送を開始する
    );
    dma_channel_set_irq1_enabled(dma_chan_transfer, true);
    irq_set_exclusive_handler(DMA_IRQ_1, irq_handler);
    irq_set_priority(DMA_IRQ_1, 1);
    irq_set_enabled(DMA_IRQ_1, true);
  }

  // カラーバー描画
  for (size_t y = 0; y < FRAME_HEIGHT; ++y) {
    size_t x = 100;
    static constexpr const uint8_t color_tbl[] = {
      0xFFu, 0xFCu, 0x1Fu, 0x1Cu, 0xE3u, 0xE0u, 0x03u, 0x00u
    };
    int bar_idx = 0;
    uint8_t c = color_tbl[0];
    for (int i = 0; i < FRAME_WIDTH; ++i)
    {
      int bi = i * sizeof(color_tbl) / FRAME_WIDTH;
      if (bar_idx != bi) {
        bar_idx = bi;

        c = color_tbl[bar_idx & 7];
      }
      frame_buffer[i + y * FRAME_WIDTH] = c;
    }
  }

  // i2sオーディオ設定
  i2s.setBCLK(PIN_I2S_BCLK);
  i2s.setDATA(PIN_I2S_DOUT);
  i2s.setBitsPerSample(16);
  i2s.setLSBJFormat();
  i2s.begin(i2s_sampleRate);

  // オーディオデータ生成用のタスクをcore1で動作させる
  multicore_launch_core1(audio_task);
}