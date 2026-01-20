#ifndef PICOSSCI_AUDIO_HPP_
#define PICOSSCI_AUDIO_HPP_

#include "picossci_audio/audio_generate.hpp"
#include "picossci_audio/output_bus.hpp"
#include "picossci_audio/max9850.hpp"
#include "picossci_audio/switch_input.hpp"

#if __has_include (<FreeRTOS.h>)
#include <FreeRTOS.h>
#include <task.h>
#else
#include <mbed.h>
#endif

namespace ns_picossci_audio
{
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
      int8_t pin_bclk = 2; // (and LRCLK == BCLK+1 == 3)
      int8_t pin_dout = 1;
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

  class picossci_audio_t
  {
  public:
    using switch_state_t = ns_picossci_audio::switch_state_t;

    /// スイッチ数
    static constexpr uint8_t SW_COUNT = 3;

    /// スイッチインデックス
    static constexpr uint8_t SW_1 = 0;  // GPIO16 : SW1-1
    static constexpr uint8_t SW_T = 1;  // GPIO17 : SW1-T
    static constexpr uint8_t SW_2 = 2;  // GPIO18 : SW1-2

    /// スイッチ配列 (publicメンバ)
    switch_state_t sw[SW_COUNT];

    struct config_t {
      // オーディオの出力サンプリングレート
      float freq_hz = 48000;

      // DMA転送バッファサイズ (バイト数)
      uint16_t dma_buf_size = 192;

      // DMAバッファの数 (最低3)
      uint8_t dma_buf_count = 8;

      // 使用するPIO (0 or 1)
      uint8_t pio_index = 0;

      // I2S出力ピン
      int8_t pin_dout = 1;   // GPIO1 : I2S_DOUT
      int8_t pin_bclk = 2;   // GPIO2 : I2S_BCLK (LRCLK = BCLK+1 = GPIO3)

      // I2Cピン (コーデック制御用)
      int8_t pin_sda = 4;    // GPIO4 : I2C0_SDA
      int8_t pin_scl = 5;    // GPIO5 : I2C0_SCL

      // I2Cクロック周波数
      uint32_t i2c_clock = 100000;

      // 初期ボリューム (0x00=最大, 0x3F=最小)
      uint8_t volume = 0x20;

      // スイッチ入力ピン
      int8_t pin_sw1 = 16;   // GPIO16 : SW1-1
      int8_t pin_swt = 17;   // GPIO17 : SW1-T
      int8_t pin_sw2 = 18;   // GPIO18 : SW1-2

      // コールバック (オプション)
      void* callback_param = nullptr;
      void (*callback_function)(void*) = nullptr;
    };

    /// 初期化 (コーデックとI2S両方を初期化)
    bool init(const config_t& config);

    /// 初期化 (デフォルト設定)
    bool init(void) { return init(config_t()); }

    /// 開始
    bool start(void);

    /// 停止
    bool stop(void);

    /// 設定取得
    config_t getConfig(void) const { return _config; }

    /// 書き込めるバイト数を返す
    int availableForWrite(void) { return _audio.availableForWrite(); }

    /// データを書き込む (data_bytesはバイト数)
    size_t write(const void* buf, size_t data_bytes) { return _audio.write(buf, data_bytes); }

    /// 左右チャンネルの int16_t 値を書き込む
    bool write(int16_t left, int16_t right) { return _audio.write(left, right); }

    /// ボリューム設定 (0x00=最大, 0x3F=最小)
    bool setVolume(uint8_t volume) { return _codec.setVolume(volume); }

    /// ミュート設定
    bool setMute(bool mute) { return _codec.setMute(mute); }

    /// スイッチ状態を更新 (loop内で呼び出すこと)
    void updateSwitches(void) {
      for (uint8_t i = 0; i < SW_COUNT; ++i) {
        sw[i].update();
      }
    }

    static void setCpuClock(uint32_t freq_khz);

    using audio_generate_t = ns_picossci_audio::audio_generate_t;
    using max9850_t = ns_picossci_audio::max9850_t;

  protected:
    config_t _config;
    ns_picossci_audio::i2s_audio_t _audio;
    ns_picossci_audio::max9850_t _codec;
    bool _inited = false;
  };
}

using Picossci_Audio = ns_picossci_audio::picossci_audio_t;

#endif
