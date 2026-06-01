#ifndef PICOSSCI_2_AUDIO_SWITCH_INPUT_HPP_
#define PICOSSCI_2_AUDIO_SWITCH_INPUT_HPP_

#include <stdint.h>

namespace ns_picossci_2_audio
{
//--------------------------------------------------------------------------------

/// 単一スイッチ入力クラス
class switch_state_t
{
public:
  /// フラグ定数 (init時に指定)
  static constexpr uint8_t FLAG_PULLUP     = 0x01;  // プルアップを有効にする
  static constexpr uint8_t FLAG_ACTIVE_LOW = 0x02;  // アクティブLOW (押すとLOW)

  /// デフォルトフラグ
  static constexpr uint8_t DEFAULT_FLAGS = FLAG_PULLUP | FLAG_ACTIVE_LOW;

  /// 初期化
  bool init(int8_t pin, uint8_t flags = DEFAULT_FLAGS);

  /// 状態を更新 (loop内で毎回呼び出すこと)
  void update(void);

  /// 現在押されているか
  bool isPressed(void) const { return _flags & FLAG_CURRENT; }

  /// 現在離されているか
  bool isReleased(void) const { return !(_flags & FLAG_CURRENT); }

  /// 押された瞬間か (前回離されていて今回押された)
  bool wasPressed(void) const {
    return (_flags & FLAG_CURRENT) && !(_flags & FLAG_PREVIOUS);
  }

  /// 離された瞬間か (前回押されていて今回離された)
  bool wasReleased(void) const {
    return !(_flags & FLAG_CURRENT) && (_flags & FLAG_PREVIOUS);
  }

  /// 初期化済みか
  bool isInited(void) const { return _flags & FLAG_INITED; }

protected:
  // 内部フラグ
  static constexpr uint8_t FLAG_CURRENT  = 0x04;  // 現在の状態
  static constexpr uint8_t FLAG_PREVIOUS = 0x08;  // 前回の状態
  static constexpr uint8_t FLAG_INITED   = 0x10;  // 初期化済み

  int8_t _pin = -1;
  uint8_t _flags = 0;
};

//--------------------------------------------------------------------------------
} // namespace ns_picossci_2_audio

#endif
