#include "switch_input.hpp"

#include <hardware/gpio.h>

namespace ns_picossci_2_audio
{
//--------------------------------------------------------------------------------

bool switch_state_t::init(int8_t pin, uint8_t flags)
{
  if (_flags & FLAG_INITED) { return true; }

  _pin = pin;
  // 設定フラグを保存 (FLAG_PULLUP, FLAG_ACTIVE_LOW)
  _flags = flags & (FLAG_PULLUP | FLAG_ACTIVE_LOW);

  if (pin >= 0) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    if (flags & FLAG_PULLUP) {
      gpio_pull_up(pin);
    }
  }

  _flags |= FLAG_INITED;
  return true;
}

void switch_state_t::update(void)
{
  if (!(_flags & FLAG_INITED)) { return; }

  // 前回の状態を保存
  if (_flags & FLAG_CURRENT) {
    _flags |= FLAG_PREVIOUS;
  } else {
    _flags &= ~FLAG_PREVIOUS;
  }

  // 現在の状態を取得
  if (_pin >= 0) {
    bool raw = gpio_get(_pin);
    // active_low の場合、LOWで押されている
    bool pressed = (_flags & FLAG_ACTIVE_LOW) ? !raw : raw;
    if (pressed) {
      _flags |= FLAG_CURRENT;
    } else {
      _flags &= ~FLAG_CURRENT;
    }
  } else {
    _flags &= ~FLAG_CURRENT;
  }
}

//--------------------------------------------------------------------------------
} // namespace ns_picossci_2_audio
