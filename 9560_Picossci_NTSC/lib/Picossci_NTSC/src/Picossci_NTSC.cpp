#include "Picossci_NTSC.h"

#include <stdlib.h>
#include <malloc.h>

extern "C" void set_cpu_clock(uint32_t freq_khz);

namespace ns_picossci_ntsc
{
//--------------------------------------------------------------------------------

  void picossci_ntsc_t::setCpuClock(uint32_t freq_khz)
  {
    ::set_cpu_clock(freq_khz);
  }
//--------------------------------------------------------------------------------

  bool cvbs_t::init(config_t config)
  {
    if (_inited) { return true; }

    _inited = false;
    _config = config;

    if (config.dma_buf_count < 2) { return false; }

    auto cfg_cvbs = _cvbs_gen.getConfig();
    cfg_cvbs.output_level = config.output_level;
    cfg_cvbs.chroma_level = config.chroma_level;

    // 出力する信号の種類
    cfg_cvbs.signal_type = cvbs_generate_t::signal_type_t::NTSC_J;
    _cvbs_gen.init(cfg_cvbs);

    auto spec = _cvbs_gen.getSignalSpec();

    _video_request_index = 0;
    _video_request_max = config.dma_buf_count < 128 ? config.dma_buf_count * 2 : 254;

    auto rawbuf = (uint8_t*)malloc((sizeof(video_request_t) * _video_request_max));
    if (rawbuf == nullptr) { return false; }

    _video_request = (video_request_t*)rawbuf;

    for (int i = 0; i < _video_request_max; ++i) {
      _video_request[i].y = -1;
      _video_request[i].dma_buffer = 0;
    }


#if __has_include (<FreeRTOS.h>)
  xTaskCreate((TaskFunction_t)task_dma_buffer, "video_task", 4096, this, config.task_priority, &_task_cvbs);
#else
  this->_task_cvbs.start(mbed::callback(task_dma_buffer, this));
  this->_task_cvbs.set_priority((osPriority_t)_config.task_priority);
#endif

    auto cfg_pio = _out_pio.getConfig();
    // 出力周波数はサブキャリア周波数の4倍とする（１サンプルにつき4サイクルでサブキャリア周波数を生成するため）
    cfg_pio.freq_hz = spec->subcarrier_freq * 4;
    cfg_pio.pio_index = config.pio_index;
    cfg_pio.pin_num = config.pin_num;
    // 3bit = GPIO 8本使用
    cfg_pio.pin_bits = 3;
    if (_out_pio.init(cfg_pio)) {
      auto cfg_bus = _bus.getConfig();
      cfg_bus.output_pio = &_out_pio;
      cfg_bus.dma_buf_count = config.dma_buf_count;

      // 一回のDMAで2本分の走査線を処理するため、スキャンライン2本分のバッファを確保する
      cfg_bus.dma_buf_size = spec->scanline_width * 2;
      // DMA転送サイズ0は8bit単位を示す
      cfg_bus.dma_transfer_size = 0;

      cfg_bus.callback_param = this;
      cfg_bus.callback_function = bus_callback;

      // auto pin_count = 1u << cfg_bus.pin_bits;
      // for (uint pidx = 0; pidx < pin_count; pidx++) {
      //   gpio_set_drive_strength(pidx + config.pin_num, gpio_drive_strength::GPIO_DRIVE_STRENGTH_12MA);
      // }

      if (_bus.init(cfg_bus)) {
        _inited = true;
        return true;
      }
      _out_pio.deinit();
    }
    return false;
  }

  bool cvbs_t::start(void)
  {
    if (_inited) {
      _bus.start();
    }
    return _inited;
  }

  bool cvbs_t::stop(void)
  {
    if (_inited) {
      _bus.stop();
    }
    return _inited;
  }

  // バスのISRから呼び出されるコールバック関数
  void cvbs_t::bus_callback(void* param, uintptr_t dma_buf, size_t dma_buf_len __attribute__((unused)))
  {
    auto me = (cvbs_t*)param;
    auto spec = me->_cvbs_gen.getSignalSpec();

    auto total_scanlines = spec->total_scanlines;
    auto request_index = me->_video_request_index;
    auto scanline = me->_scanline;

    // 一回のコールバックで走査線2本分を処理する
    for (int i = 0; i < 2; ++i) {
      int y = me->_cvbs_gen.make_sync((uint8_t*)dma_buf, scanline, me->_video_request_max, i);
      me->_video_request[request_index].dma_buffer = (uint8_t*)dma_buf;
      me->_video_request[request_index].y = y;
      if (y >= 0) {
        if (++request_index >= me->_video_request_max) { request_index = 0; }
       /// ここで次回分のリクエストを無効化しておく (データ生成タスクのリングバッファ周回をリクエストが追い越してしまった場合の対策)
        me->_video_request[request_index].y = -256;
      }
      dma_buf += spec->scanline_width;
      if (++scanline >= total_scanlines) { scanline -= total_scanlines; }
    }
    me->_video_request_index = request_index;
    me->_scanline = scanline;

#if __has_include (<FreeRTOS.h>)
    BaseType_t pxHigherPriorityTaskWoken = false;
    vTaskNotifyGiveFromISR(me->_task_cvbs, &pxHigherPriorityTaskWoken);
#else
    me->_event_flags.set(_FLAG_FROM_ISR);
#endif
  }

  void cvbs_t::task_dma_buffer(cvbs_t* me)
  {
    for (;;) {
#if __has_include (<FreeRTOS.h>)
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#else
      me->_event_flags.wait_all(_FLAG_FROM_ISR, osWaitForever);
#endif

      if ((me->_video_request[me->_write_index].y) >= 0) {
        auto func = me->_config.callback_function;
        if (func != nullptr) {
          func(me->_config.callback_param);
        }
      }
    }
  }

  int16_t cvbs_t::getCurrentY(void) const
  {
    return _video_request[_write_index].y;
  }

  void cvbs_t::writeScanLine(const void* buf, size_t pixel_count)
  {
    auto wi = _write_index;

    /// リクエスト情報からバッファのアドレスとY座標の値を得る
    int16_t y = _video_request[wi].y;
    if (y >= 0)
    {
      auto dst = _video_request[wi].dma_buffer;
      _video_request[wi].y = -256;
      _cvbs_gen.make_scanline(dst, buf, pixel_count);

      if (++wi >= _video_request_max) { wi = 0; }
      _write_index = wi;
    }
  }

//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
  bool i2s_audio_t::init(config_t config)
  {
    if (_inited) { return true; }

    _inited = false;

    if (config.dma_buf_count < 3) { return false; }

    auto cfg_audio = _audio_gen.getConfig();
    cfg_audio.dma_count = config.dma_buf_count;
    cfg_audio.callback_param = config.callback_param;
    cfg_audio.callback_function = config.callback_function;
    cfg_audio.freq_hz = config.freq_hz;
    cfg_audio.bps = 16;
    cfg_audio.stereo = true;

    _audio_gen.init(cfg_audio);
    auto cfg_pio = _out_pio.getConfig();
    cfg_pio.freq_hz = config.freq_hz;
    cfg_pio.pio_index = config.pio_index;
    cfg_pio.pin_bclk = config.pin_bclk;
    cfg_pio.pin_dout = config.pin_dout;

    if (_out_pio.init(cfg_pio)) {
      auto cfg_bus = _bus.getConfig();
      cfg_bus.output_pio = &_out_pio;
      cfg_bus.dma_buf_count = config.dma_buf_count;

      // dma_transfer_size: 0=1Byte, 1=2Byte, 2=4Byte
      cfg_bus.dma_transfer_size = cfg_audio.bps > 16 ? 2 : (cfg_audio.bps >> 4);
      cfg_bus.dma_buf_size = config.dma_buf_size;

      cfg_bus.callback_function = _audio_gen.bus_callback;
      cfg_bus.callback_param = &_audio_gen;

      if (_bus.init(cfg_bus)) {
        _inited = true;
        return true;
      }
      _out_pio.deinit();
    }
    return false;
  }

  bool i2s_audio_t::start(void)
  {
    if (_inited) {
      _bus.start();
    }
    return _inited;
  }

  bool i2s_audio_t::stop(void)
  {
    if (_inited) {
      _bus.stop();
    }
    return _inited;
  }

//--------------------------------------------------------------------------------
}
