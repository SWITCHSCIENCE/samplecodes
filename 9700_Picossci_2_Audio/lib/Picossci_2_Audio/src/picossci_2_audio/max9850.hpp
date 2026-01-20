#ifndef PICOSSCI_2_AUDIO_MAX9850_HPP_
#define PICOSSCI_2_AUDIO_MAX9850_HPP_

#include <stdint.h>
#include <Wire.h>

namespace ns_picossci_2_audio
{
//--------------------------------------------------------------------------------

/// MAX9850 オーディオコーデック制御クラス
class max9850_t
{
public:
  // MAX9850 I2Cアドレス
  static constexpr uint8_t I2C_ADDR = 0x10;

  // MAX9850 レジスタマップ
  static constexpr uint8_t REG_STATUSA       = 0x00;
  static constexpr uint8_t REG_STATUSB       = 0x01;
  static constexpr uint8_t REG_VOLUME        = 0x02;
  static constexpr uint8_t REG_GENERAL_PURPOSE = 0x03;
  static constexpr uint8_t REG_INTERRUPT     = 0x04;
  static constexpr uint8_t REG_ENABLE        = 0x05;
  static constexpr uint8_t REG_CLOCK         = 0x06;
  static constexpr uint8_t REG_CHARGE_PUMP   = 0x07;
  static constexpr uint8_t REG_LRCLK_MSB     = 0x08;
  static constexpr uint8_t REG_LRCLK_LSB     = 0x09;
  static constexpr uint8_t REG_DIGITAL_AUDIO = 0x0A;

  // DIGITAL_AUDIO レジスタビット
  static constexpr uint8_t FMT_I2S_COMPATIBLE = 1 << 3;
  static constexpr uint8_t FMT_LEFT_JUSTIFIED = 0;

  struct config_t {
    // I2Cインスタンス (デフォルトはWire)
    TwoWire* wire = &Wire;

    // I2C SDAピン
    int8_t pin_sda = 4;

    // I2C SCLピン
    int8_t pin_scl = 5;

    // I2Cクロック周波数
    uint32_t i2c_clock = 100000;

    // 初期ボリューム値 (0x00～0x3F, 小さいほど大きい音)
    uint8_t volume = 0x20;
  };

  /// 初期化
  bool init(const config_t& config);

  /// デバイスが存在するかチェック
  bool isConnected(void);

  /// レジスタに書き込み
  bool writeRegister(uint8_t reg, uint8_t value);

  /// レジスタを読み込み
  bool readRegister(uint8_t reg, uint8_t& value);

  /// レジスタの特定ビットを更新
  bool updateBits(uint8_t reg, uint8_t mask, uint8_t value);

  /// ボリューム設定 (0x00～0x3F, 小さいほど大きい音)
  bool setVolume(uint8_t volume);

  /// ミュート設定
  bool setMute(bool mute);

  /// config取得
  const config_t& getConfig(void) const { return _config; }

protected:
  config_t _config;
  bool _inited = false;
};

//--------------------------------------------------------------------------------
} // namespace ns_picossci_2_audio

#endif
