#ifndef PICOSSCI_NTSC_HPP_
#define PICOSSCI_NTSC_HPP_

#include "picossci_ntsc/cvbs_generate.hpp"
#include "picossci_ntsc/audio_generate.hpp"
#include "picossci_ntsc/output_bus.hpp"

#if __has_include (<FreeRTOS.h>)
#include <FreeRTOS.h>
#include <task.h>
#else
#include <mbed.h>
#endif

namespace ns_picossci_ntsc
{
  /// CVBS映像信号を出力するクラス
  class cvbs_t
  {
  public:
    struct config_t {
      // バッファ空きが出来たことを知らせるコールバック関数
      void (*callback_function)(void* param) = nullptr;

      // コールバック関数の第一引数で受け取るポインタ
      void* callback_param = nullptr;

      // 使用するPIOの番号 (0 or 1)
      uint8_t pio_index = 0;

      // R2R-DAC出力先GPIOピン番号 (例:0を指定した場合はGPIO0～7の合計8本が使用される)
      uint8_t pin_num = 0;

      // 走査線単位のDMAバッファの数 (最低3。処理が間に合わず映像が乱れる場合は数を増やすことで解消できる場合がある。但しメモリ消費量が増える)
      uint8_t dma_buf_count = 4;

      // 映像信号が弱い場合はoutput_levelを高くすること対処できる場合があります。(初期値128)
      uint8_t output_level = 128;

      // 映像の彩度が弱い場合はchroma_levelを高くすること対処できる場合があります。(初期値128)
      uint8_t chroma_level = 128;

#if __has_include (<FreeRTOS.h>)
      // DMAバッファ作成タスクの優先度
      uint8_t task_priority = configMAX_PRIORITIES - 1;
#else
      // DMAバッファ作成タスクの優先度
      uint8_t task_priority = osPriority_t::osPriorityRealtime;
#endif
    };

    bool init(config_t config);
    bool start(void);
    bool stop(void);

    config_t getConfig(void) { return config_t(); }

    // 画像データを要求されている走査線のY座標値を取得する
    // マイナスの場合はバッファが空いていないか表示範囲外であることを示す
    int16_t getCurrentY(void) const;

    // 走査線の画像データを書き込む
    // getCurrentYで対象となるY座標を確認の後に呼び出すこと
    void writeScanLine(const void* buf, size_t pixel_count);

    // 横方向の表示倍率を設定する
    // 256が等倍表示、512が2倍表示、128が1/2倍表示
    void setScale(uint16_t x_scale) { _cvbs_gen.setScale(x_scale); }

    // 横方向の表示オフセットを設定する
    // プラスで右方向に、マイナスで左方向に表示をずらす
    void setOffset(int32_t x_offset) { _cvbs_gen.setOffset(x_offset); }

    // ピクセルモードを設定する
    void setPixelMode(cvbs_generate_t::pixel_mode_t mode) { _cvbs_gen.setPixelMode(mode); }

    // パレットを設定する
    void setPalette(uint8_t index, uint32_t rgb888) { _cvbs_gen.setPalette(index, rgb888); }

  protected:
    config_t _config;
    bool _inited = false;

    // busに渡すコールバック関数
    static void bus_callback(void* param, uintptr_t dma_buf, size_t dma_buf_len __attribute__((unused)));

    /// DMAバッファ生成用のタスク
    static void task_dma_buffer(cvbs_t* me);

    output_pio_simple_t _out_pio;
    output_bus_continuous_t _bus;
    cvbs_generate_t _cvbs_gen;

#if __has_include (<FreeRTOS.h>)
    /// DMA転送割り込み発生時の通知先タスク
    xTaskHandle _task_cvbs = nullptr;
#else
    static constexpr const uint32_t _FLAG_FROM_ISR = 3;
    rtos::Thread _task_cvbs;
    rtos::EventFlags _event_flags;
#endif

    /// データ要求情報
    struct video_request_t {
      uint8_t* dma_buffer;
      volatile int16_t y;
    };

    /// データ要求情報の配列
    video_request_t* _video_request = nullptr;

    uint16_t _write_index;

    /// 現在の走査線位置
    uint16_t _scanline = 0;

    /// 
    uint8_t _video_request_max = 0;
    /// 現在の映像要求リストの位置
    uint8_t _video_request_index = 0;

  };

  class i2s_audio_t
  {
  public:
    struct config_t {
      void* callback_param = nullptr;

      void (*callback_function)(void*);

      // オーディオの出力サンプリングレート
      float freq_hz = 48000;

      // 内部での出力は16bitステレオ固定

      // DMA転送バッファサイズ (バイト数)
      // 16ビットステレオ出力となるので 4の倍数であること。
      // 例: 192 を指定した場合 16bitステレオデータで48サンプル分のデータを保持できる。
      // つまり freq_hz が 48kHzのとき 1ms 分のデータを保持できる。
      uint16_t dma_buf_size = 192;

      // DMAバッファの数 (最低3)
      // dma_buf_size のサイズのバッファを何個分用意するかの設定。
      // dma_buf_size が 1msec の設定のとき、 dma_count を 4にすると 4msec分のデータが保持できる。
      uint8_t dma_buf_count = 4;

      // 使用するPIO (0 or 1)
      uint8_t pio_index;

      // 出力先GPIOピン番号
      int8_t pin_bclk = 16; // (and LRCK == BCLK+1 == 17)
      int8_t pin_dout = 18;
    };

    bool init(config_t config);
    bool start(void);
    bool stop(void);

    config_t getConfig(void) { return config_t(); }

    // 書き込めるバイト数を返す(サンプル数ではない点に注意。int16_t型なら1サンプルあたり2バイトになる)
    int availableForWrite(void) { return _audio_gen.availableForWrite(); }

    // データを書き込む (data_bytesはバイト数。サンプル数ではない点に注意)
    size_t write(const void* buf, size_t data_bytes) { return _audio_gen.write(buf, data_bytes); }

    // 左チャンネルと右チャンネルの int16_tの値を指定して 2サンプル (4バイト) 書き込む
    bool write(int16_t left, int16_t right)
    {
      int16_t data[2] = { left, right };
      return _audio_gen.write(data, 4);
    }

  protected:
    bool _inited = false;

    output_pio_i2s_t _out_pio;
    output_bus_continuous_t _bus;
    audio_generate_t _audio_gen;
  };

  class picossci_ntsc_t
  {
  public:
    ns_picossci_ntsc::cvbs_t video;
    ns_picossci_ntsc::i2s_audio_t audio;

    static void setCpuClock(uint32_t freq_khz);

    using cvbs_generate_t = ns_picossci_ntsc::cvbs_generate_t;
    using pixel_mode_t = picossci_ntsc_t::cvbs_generate_t::pixel_mode_t;
    using audio_generate_t = ns_picossci_ntsc::audio_generate_t;
  };
}

using Picossci_NTSC = ns_picossci_ntsc::picossci_ntsc_t;

#endif
