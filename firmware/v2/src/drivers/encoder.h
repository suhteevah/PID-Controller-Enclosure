#pragma once

#include <Arduino.h>
#include "config.h"

enum class EncoderEvent : uint8_t {
    NONE,
    ROTATE_CW,
    ROTATE_CCW,
    PRESS,
    LONG_PRESS
};

class RotaryEncoder {
public:
    RotaryEncoder();

    void begin();
    EncoderEvent poll();

private:
    volatile int32_t _encoderPos;
    int32_t _lastEncoderPos;

    bool _buttonState;
    bool _lastButtonState;
    uint32_t _buttonChangeTime;
    uint32_t _buttonPressTime;
    bool _buttonHandled;

    static void IRAM_ATTR isrHandler();
    static volatile int32_t _isrPos;
    static volatile uint32_t _isrLastTime;
};
