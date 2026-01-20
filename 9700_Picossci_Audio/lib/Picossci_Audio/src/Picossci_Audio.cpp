#include "Picossci_Audio.h"

#include <stdlib.h>
#include <malloc.h>

extern "C" void set_cpu_clock(uint32_t freq_khz);

namespace ns_picossci_audio
{
//--------------------------------------------------------------------------------

  void picossci_audio_t::setCpuClock(uint32_t freq_khz)
  {
    ::set_cpu_clock(freq_khz);
  }

  bool picossci_audio_t::init(const config_t& config)
  {
    if (_inited) { return true; }

    _config = config;

    // コーデック (MAX9850) の初期化
    max9850_t::config_t codec_cfg;
    codec_cfg.pin_sda = config.pin_sda;
    codec_cfg.pin_scl = config.pin_scl;
    codec_cfg.i2c_clock = config.i2c_clock;
    codec_cfg.volume = config.volume;

    bool codec_ok = _codec.init(codec_cfg);
    // コーデック初期化失敗時も I2S 出力は継続する

    // スイッチ入力の初期化
    sw[SW_1].init(config.pin_sw1);
    sw[SW_T].init(config.pin_swt);
    sw[SW_2].init(config.pin_sw2);

    // I2S オーディオの初期化
    i2s_audio_t::config_t audio_cfg;
    audio_cfg.freq_hz = config.freq_hz;
    audio_cfg.dma_buf_size = config.dma_buf_size;
    audio_cfg.dma_buf_count = config.dma_buf_count;
    audio_cfg.pio_index = config.pio_index;
    audio_cfg.pin_dout = config.pin_dout;
    audio_cfg.pin_bclk = config.pin_bclk;
    audio_cfg.callback_param = config.callback_param;
    audio_cfg.callback_function = config.callback_function;

    if (!_audio.init(audio_cfg)) {
      return false;
    }

    _inited = true;
    return codec_ok;
  }

  bool picossci_audio_t::start(void)
  {
    if (!_inited) { return false; }
    return _audio.start();
  }

  bool picossci_audio_t::stop(void)
  {
    if (!_inited) { return false; }
    return _audio.stop();
  }

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
