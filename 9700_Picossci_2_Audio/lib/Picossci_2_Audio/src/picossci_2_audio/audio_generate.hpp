#ifndef PICOSSCI_2_AUDIO_AUDIO_GENERATE_HPP_
#define PICOSSCI_2_AUDIO_AUDIO_GENERATE_HPP_

#if __has_include (<FreeRTOS.h>)
#include <FreeRTOS.h>
#include <task.h>
#else
#include <mbed.h>
#endif
#include <stdint.h>
#include <stddef.h>

namespace ns_picossci_2_audio
{
//--------------------------------------------------------------------------------

/// 音声データを生成するクラス
class audio_generate_t
{
public:

  struct config_t {
    // バッファ空きが出来たことを知らせるコールバック関数
    void (*callback_function)(void* param) = nullptr;

    // コールバック関数の第一引数
    void* callback_param = nullptr;

    // オーディオの出力サンプリングレート
    float freq_hz = 48000;

    // オーディオの出力ビット数
    uint8_t bps = 16; // 8/16/24/32

    // ステレオ or モノラル
    bool stereo = true;

    // DMA転送用のバッファの数 (最低2)
    uint8_t dma_count = 4;

#if __has_include (<FreeRTOS.h>)
    // DMAバッファ作成タスクの優先度
    uint8_t task_priority = configMAX_PRIORITIES - 1;
#else
    // DMAバッファ作成タスクの優先度
    uint8_t task_priority = osPriority_t::osPriorityRealtime;
#endif
  };

  bool init(const config_t &config);

  config_t getConfig(void) const { return _config; }

  int availableForWrite(void);
  size_t write(const void* buf, size_t data_bytes);

  /// DMA転送クラスに渡すコールバック
  static void bus_callback(void* param, uintptr_t buf, size_t size);
protected:

  static void task_callback(audio_generate_t* me);

#if __has_include (<FreeRTOS.h>)
  /// DMA転送割り込み発生時の通知先タスク
  xTaskHandle _task_audio = nullptr;
#else
  static constexpr const uint32_t _FLAG_FROM_ISR = 1;
  rtos::Thread _task_audio;
  rtos::EventFlags _event_flags;
#endif

  /// データ要求情報
  struct audio_request_t {
    uint8_t* dma_buffer;
    uint16_t length;
    bool data_requested;
  };

  /// データ要求情報の配列
  audio_request_t* _audio_request = nullptr;

  config_t _config;

  // 音声バッファへの書込み位置
  uint32_t _write_position = 0;
  uint8_t _write_index = 0;

  /// 現在の要求リストの位置
  uint8_t _request_index = 0;
  uint8_t _request_max = 0;

};
//--------------------------------------------------------------------------------
} // namespace ns_picossci_2_audio

#endif
