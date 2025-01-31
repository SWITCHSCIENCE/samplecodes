#ifndef __LEVER_AND_PUSH_SW__
#define __LEVER_AND_PUSH_SW__

#include <Arduino.h>

class LeverAndPushSw {
public:
  enum State {
    OPEN,
    LEVER_CCW,
    LEVER_CW,
    PUSH
  };

  enum Event {
    NONE,
    PRESSED,
    RELEASED,
    HELD
  };

  LeverAndPushSw(int pin, int range = 1023)
    : _pin(pin), _lastState(OPEN), _lastEvent(NONE), _lastPressTime(0), _lastHoldTime(0), _range(range) {}

  void update() {
    int ain = analogRead(_pin);
    unsigned long currentTime = millis();
    State currentState = analogToState(ain);

    // 押された瞬間
    if (currentState != _lastState && currentState != OPEN) {
      _lastEvent = PRESSED;
      _lastPressTime = currentTime;
      _lastHoldTime = currentTime + 300;  // リピート開始までの待ち時間
      _releaseState = currentState;       // リリース時に返す状態を保存
    }
    // 離された瞬間
    else if (currentState == OPEN && _lastState != OPEN) {
      _lastEvent = RELEASED;
    }
    // 押し続けられている場合イベント発生
    else if (currentState != OPEN && _lastState == currentState && currentTime > _lastHoldTime && (currentTime - _lastHoldTime >= 200)) {
      _lastEvent = HELD;
      _lastHoldTime = currentTime;
    }
    // 変化なし
    else {
      _lastEvent = NONE;
    }

    _lastState = currentState;
  }

  State getState() {
    if (_lastEvent == RELEASED) {
      return _releaseState;
    }
    return _lastState;
  }

  Event getEvent() {
    return _lastEvent;
  }

private:
  int _pin;
  int _range;
  State _lastState;
  State _releaseState;
  Event _lastEvent;
  unsigned long _lastPressTime;
  unsigned long _lastHoldTime;

  State analogToState(int ain) {
    if (ain > _range * 0.84) return OPEN;       // (100% + 68%) / 2
    if (ain > _range * 0.51) return LEVER_CCW;  // (68% + 35%) / 2
    if (ain > _range * 0.19) return LEVER_CW;   // (35% + 4%) / 2
    return PUSH;
  }
};

#endif
