#include "encoder.h"

volatile int32_t RotaryEncoder::_isrPos = 0;
volatile uint32_t RotaryEncoder::_isrLastTime = 0;

RotaryEncoder::RotaryEncoder()
    : _encoderPos(0), _lastEncoderPos(0),
      _buttonState(false), _lastButtonState(false),
      _buttonChangeTime(0), _buttonPressTime(0), _buttonHandled(false) {}

void RotaryEncoder::begin() {
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), isrHandler, FALLING);

    _isrPos = 0;
    _lastEncoderPos = 0;
}

EncoderEvent RotaryEncoder::poll() {
    // Rotation
    noInterrupts();
    int32_t currentPos = _isrPos;
    interrupts();

    if (currentPos != _lastEncoderPos) {
        int32_t delta = currentPos - _lastEncoderPos;
        _lastEncoderPos = currentPos;
        return (delta > 0) ? EncoderEvent::ROTATE_CW : EncoderEvent::ROTATE_CCW;
    }

    // Button
    bool currentButton = !digitalRead(PIN_ENC_SW);
    uint32_t now = millis();

    if (currentButton != _lastButtonState) {
        _buttonChangeTime = now;
    }

    if ((now - _buttonChangeTime) > BUTTON_DEBOUNCE_MS) {
        if (currentButton != _buttonState) {
            _buttonState = currentButton;
            if (_buttonState) {
                _buttonPressTime = now;
                _buttonHandled = false;
            } else if (!_buttonHandled) {
                _lastButtonState = currentButton;
                return EncoderEvent::PRESS;
            }
        }

        if (_buttonState && !_buttonHandled &&
            (now - _buttonPressTime) >= BUTTON_LONG_PRESS_MS) {
            _buttonHandled = true;
            _lastButtonState = currentButton;
            return EncoderEvent::LONG_PRESS;
        }
    }

    _lastButtonState = currentButton;
    return EncoderEvent::NONE;
}

void IRAM_ATTR RotaryEncoder::isrHandler() {
    uint32_t now = millis();
    if ((now - _isrLastTime) < ENCODER_DEBOUNCE_MS) return;
    _isrLastTime = now;

    if (digitalRead(PIN_ENC_DT)) _isrPos++;
    else _isrPos--;
}
