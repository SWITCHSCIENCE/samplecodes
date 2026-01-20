#include "max9850.hpp"

namespace ns_picossci_audio
{
//--------------------------------------------------------------------------------

bool max9850_t::init(const config_t& config)
{
  if (_inited) { return true; }

  _config = config;

  // I2Cピンの設定
  _config.wire->setSDA(_config.pin_sda);
  _config.wire->setSCL(_config.pin_scl);
  _config.wire->begin();
  _config.wire->setClock(_config.i2c_clock);

  // デバイスの存在確認
  if (!isConnected()) {
    return false;
  }

  bool ok = true;

  // I2Sフォーマット設定、コーデックはスレーブモード
// ok &= writeRegister(REG_DIGITAL_AUDIO, FMT_I2S_COMPATIBLE);
  ok &= writeRegister(REG_DIGITAL_AUDIO, FMT_LEFT_JUSTIFIED);

  // 基本クロック設定
  ok &= writeRegister(REG_CLOCK, 0x00);

  // ゼロ検出とスルーレート制御を有効化
  ok &= updateBits(REG_GENERAL_PURPOSE, 0x01, 0x01);
  ok &= updateBits(REG_VOLUME, 0x40, 0x40);

  // スルーレート 125ms
  ok &= writeRegister(REG_CHARGE_PUMP, 0xC0);

  // メインブロックの電源ON: SHDN, MCLK, チャージポンプ, ミキサー, HP, DAC
  ok &= updateBits(REG_ENABLE, 0xFD, 0xFD);

  // ボリューム設定とヘッドフォンスイッチのミュート解除
  ok &= updateBits(REG_VOLUME, 0x3F, _config.volume);
  ok &= updateBits(REG_VOLUME, 0x80, 0x00);

  _inited = ok;
  return ok;
}

bool max9850_t::isConnected(void)
{
  _config.wire->beginTransmission(I2C_ADDR);
  return (0 == _config.wire->endTransmission(true));
}

bool max9850_t::writeRegister(uint8_t reg, uint8_t value)
{
  _config.wire->beginTransmission(I2C_ADDR);
  _config.wire->write(reg);
  _config.wire->write(value);
  return (0 == _config.wire->endTransmission(true));
}

bool max9850_t::readRegister(uint8_t reg, uint8_t& value)
{
  _config.wire->beginTransmission(I2C_ADDR);
  _config.wire->write(reg);
  if (0 != _config.wire->endTransmission(false)) {
    return false;
  }
  if (1 != _config.wire->requestFrom(I2C_ADDR, (uint8_t)1, (uint8_t)true)) {
    return false;
  }
  value = static_cast<uint8_t>(_config.wire->read());
  return true;
}

bool max9850_t::updateBits(uint8_t reg, uint8_t mask, uint8_t value)
{
  uint8_t current = 0;
  if (!readRegister(reg, current)) {
    return false;
  }
  uint8_t next = static_cast<uint8_t>((current & ~mask) | (value & mask));
  return writeRegister(reg, next);
}

bool max9850_t::setVolume(uint8_t volume)
{
  if (!_inited) { return false; }
  return updateBits(REG_VOLUME, 0x3F, volume & 0x3F);
}

bool max9850_t::setMute(bool mute)
{
  if (!_inited) { return false; }
  return updateBits(REG_VOLUME, 0x80, mute ? 0x80 : 0x00);
}

//--------------------------------------------------------------------------------
} // namespace ns_picossci_audio
