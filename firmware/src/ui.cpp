#include "ui.h"

// Static ISR variables
volatile int32_t UIManager::_isrEncoderPos = 0;
volatile uint32_t UIManager::_isrLastTime = 0;

UIManager::UIManager()
    : _currentScreen(Screen::MAIN), _selectedChannel(0),
      _menuIndex(0), _fineAdjust(false),
      _encoderPos(0), _lastEncoderPos(0), _lastEncTime(0),
      _buttonState(false), _lastButtonState(false),
      _buttonPressTime(0), _buttonHandled(false) {}

void UIManager::begin() {
    // Configure encoder pins with internal pullups
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // Attach interrupt for encoder rotation
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), encoderISR, FALLING);

    _isrEncoderPos = 0;
    _lastEncoderPos = 0;
}

UIEvent UIManager::poll() {
    // --- Encoder rotation ---
    noInterrupts();
    int32_t currentPos = _isrEncoderPos;
    interrupts();

    if (currentPos != _lastEncoderPos) {
        int32_t delta = currentPos - _lastEncoderPos;
        _lastEncoderPos = currentPos;

        if (delta > 0) return UIEvent::ROTATE_CW;
        if (delta < 0) return UIEvent::ROTATE_CCW;
    }

    // --- Button handling ---
    bool currentButton = !digitalRead(PIN_ENC_SW); // Active low
    uint32_t now = millis();

    // Debounce
    if (currentButton != _lastButtonState) {
        _lastEncTime = now;
    }

    if ((now - _lastEncTime) > BUTTON_DEBOUNCE_MS) {
        if (currentButton != _buttonState) {
            _buttonState = currentButton;

            if (_buttonState) {
                // Button just pressed
                _buttonPressTime = now;
                _buttonHandled = false;
            } else {
                // Button released
                if (!_buttonHandled) {
                    return UIEvent::PRESS;
                }
            }
        }

        // Check for long press while held
        if (_buttonState && !_buttonHandled) {
            if ((now - _buttonPressTime) >= BUTTON_LONG_PRESS_MS) {
                _buttonHandled = true;
                return UIEvent::LONG_PRESS;
            }
        }
    }

    _lastButtonState = currentButton;
    return UIEvent::NONE;
}

void UIManager::setScreen(Screen screen) {
    _currentScreen = screen;
    _menuIndex = 0;
}

void IRAM_ATTR UIManager::encoderISR() {
    uint32_t now = millis();
    if (now - _isrLastTime < ENCODER_DEBOUNCE_MS) return;
    _isrLastTime = now;

    if (digitalRead(PIN_ENC_DT)) {
        _isrEncoderPos++;
    } else {
        _isrEncoderPos--;
    }
}
