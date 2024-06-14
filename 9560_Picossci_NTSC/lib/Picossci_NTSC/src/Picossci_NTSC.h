#ifndef PICOSSCI_NTSC_HPP_
#define PICOSSCI_NTSC_HPP_

#include "picossci_ntsc/cvbs_generate.hpp"
#include "picossci_ntsc/output_bus.hpp"

/// CVBS映像信号を出力するクラス
class picossci_ntsc_t
{
public:
  struct config_t {
    void* callback_param = nullptr;

    void (*callback_requestImage)(cvbs_generate_t::line_image_info_t* info, uint16_t y);

    // 使用するPIOの番号 (0 or 1)
    uint8_t pio_index = 0;

    // R2R-DAC出力先GPIOピン番号 (例:0を指定した場合はGPIO0～7の合計8本が使用される)
    uint8_t pin_num = 0;

    // 走査線単位のDMAバッファの数 (最低3。処理が間に合わず映像が乱れる場合は数を増やすことで解消できる場合がある。但しメモリ消費量が増える)
    uint8_t dma_count = 3;

    // 映像信号が弱い場合はoutput_levelを高くすること対処できる場合があります。(初期値128)
    uint8_t output_level = 128;

    // 映像の彩度が弱い場合はchroma_levelを高くすること対処できる場合があります。(初期値128)
    uint8_t chroma_level = 128;
  };

  bool init(config_t config)
  {
    if (_inited) { return true; }

    _inited = false;

    if (config.dma_count < 3) { return false; }

    auto cfg_cvbs = _cvbs_out.getConfig();
    cfg_cvbs.dma_count = config.dma_count;
    cfg_cvbs.output_level = config.output_level;
    cfg_cvbs.chroma_level = config.chroma_level;
    cfg_cvbs.callback_param = config.callback_param;
    cfg_cvbs.callback_requestImage = config.callback_requestImage;
    cfg_cvbs.signal_type = cvbs_generate_t::signal_type_t::NTSC; // 出力する信号の種類
    _cvbs_out.init(cfg_cvbs);

    auto spec = _cvbs_out.getSignalSpec();

    auto cfg_bus = _bus.getConfig();
    cfg_bus.dma_count = config.dma_count;
    cfg_bus.dma_length = spec->scanline_width * 2;

    // 出力周波数はサブキャリア周波数の4倍とする（１サンプルにつき4サイクルでサブキャリア周波数を生成するため）
    cfg_bus.freq_hz = spec->subcarrier_freq * 4;

    // 3bit = GPIO 8本使用
    cfg_bus.pin_bits = 3;
    cfg_bus.pin_num = config.pin_num;
    cfg_bus.pio_index = config.pio_index;
    cfg_bus.callback_function = _cvbs_out.dma_callback;
    cfg_bus.callback_param = &_cvbs_out;

    auto pin_count = 1u << cfg_bus.pin_bits;
    for (uint pidx = 0; pidx < pin_count; pidx++) {
      gpio_set_drive_strength(pidx + config.pin_num, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    }

    if (_bus.init(cfg_bus)) {
      _inited = true;
      return true;
    }
    return false;
  }

  bool start(void)
  {
    if (_inited) {
      _bus.start();
    }
    return _inited;
  }

  config_t getConfig(void) { return config_t(); }
  uint16_t getScanline(void) const { return _cvbs_out.getScanline(); }

  void setPalette(uint8_t index, uint32_t rgb888) { _cvbs_out.setPalette(index, rgb888); }

protected:
  bool _inited = false;

  continuous_output_bus_t _bus;
  cvbs_generate_t _cvbs_out;
};

#endif
