// #define GPIO_DEBUG_PIN 19

#include "audio_generate.hpp"

#include <hardware/gpio.h>
#include <math.h>
#include <string.h>

namespace ns_picossci_ntsc
{
//--------------------------------------------------------------------------------

bool audio_generate_t::init(const config_t &config)
{
  if (config.dma_count < 2) { return false; }

#if defined ( GPIO_DEBUG_PIN )
#if __has_include (<FreeRTOS.h>)
gpio_init(GPIO_DEBUG_PIN);
#else
_gpio_init(GPIO_DEBUG_PIN);
#endif
gpio_set_dir(GPIO_DEBUG_PIN, GPIO_OUT);
#endif

  _config = config;

  _request_index = 0;
  auto dma_count = config.dma_count;

  auto rawbuf = (uint8_t*)malloc((sizeof(audio_request_t) * dma_count));
  if (rawbuf == nullptr) { return false; }

  _audio_request = (audio_request_t*)rawbuf;

  for (int i = 0; i < dma_count; ++i) {
    _audio_request[i].dma_buffer = 0;
    _audio_request[i].data_requested = false;
  }

#if __has_include (<FreeRTOS.h>)
  xTaskCreate((TaskFunction_t)task_callback, "audio_dma", 4096, this, _config.task_priority, &_task_audio);
#else
  this->_task_audio.start(mbed::callback(task_callback, this));
  this->_task_audio.set_priority((osPriority_t)_config.task_priority);
#endif

  return true;
}

/// DMA転送ISRから呼び出されるコールバック関数
void audio_generate_t::bus_callback(void* param, uintptr_t dma_buf, size_t dma_buf_len __attribute__((unused)))
{
#if defined ( GPIO_DEBUG_PIN )
gpio_put(GPIO_DEBUG_PIN, 1);
#endif
  auto me = (audio_generate_t*)param;
  auto request_index = me->_request_index;

  auto &req = me->_audio_request[request_index];

  if (++request_index >= me->_config.dma_count) { request_index = 0; }
  // /// ここで次回分のリクエストを無効化しておく (データ生成タスクのリングバッファ周回をリクエストが追い越してしまった場合の対策)
  me->_audio_request[request_index].data_requested = false;
  me->_request_index = request_index;

  req.length = dma_buf_len;
  req.dma_buffer = (uint8_t*)dma_buf;
  req.data_requested = true;

#if __has_include (<FreeRTOS.h>)
  BaseType_t pxHigherPriorityTaskWoken = false;
  vTaskNotifyGiveFromISR(me->_task_audio, &pxHigherPriorityTaskWoken);
#else
  me->_event_flags.set(_FLAG_FROM_ISR);
#endif

#if defined ( GPIO_DEBUG_PIN )
gpio_put(GPIO_DEBUG_PIN, 0);
#endif
}

void audio_generate_t::task_callback(audio_generate_t* me)
{
  for (;;) {
#if __has_include (<FreeRTOS.h>)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#else
    me->_event_flags.wait_all(_FLAG_FROM_ISR, osWaitForever);
#endif

    auto func = me->_config.callback_function;
    if (func != nullptr) {
      func(me->_config.callback_param);
    }
  }
}

int audio_generate_t::availableForWrite(void)
{
  int result = 0;

  auto wr_idx = _write_index;
  if (_audio_request[wr_idx].data_requested) {
    result = _audio_request[wr_idx].length - _write_position;
    if (++wr_idx >= _config.dma_count) { wr_idx = 0; }

    while (_audio_request[wr_idx].data_requested) {
      result += _audio_request[wr_idx].length;
      if (++wr_idx >= _config.dma_count) { wr_idx = 0; }
      if (_write_index == wr_idx) { break; }
    }
  }
  return result;
}

size_t audio_generate_t::write(const void* buf, size_t data_bytes)
{
  auto wr_idx = _write_index;
  size_t result = 0;
  if (data_bytes == 0 || !_audio_request[wr_idx].data_requested) {
    return result;
  }
  auto wp = _write_position;

  do {
    auto dst = _audio_request[wr_idx].dma_buffer;
    auto len = (int)_audio_request[wr_idx].length;
    int wr_len = len - wp;
    if (wr_len > (int)data_bytes) { wr_len = data_bytes; }
    memcpy(dst + wp, buf, wr_len);

    buf = (const uint8_t*)buf + wr_len;
    data_bytes -= wr_len;
    result += wr_len;
    wp += wr_len;
    if (wp >= len) {
      wp = 0;
      _audio_request[wr_idx].data_requested = false;
      if (++wr_idx >= _config.dma_count) { wr_idx = 0; }
      if (!_audio_request[wr_idx].data_requested) {
        break;
      }
    }
  } while (data_bytes);
  _write_position = wp;
  _write_index = wr_idx;
  return result;
}

//--------------------------------------------------------------------------------
} // namespace picossci_ntsc
