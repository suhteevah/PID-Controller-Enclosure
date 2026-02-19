#include "ssr.h"

SSRDriver::SSRDriver()
    : _pin(0), _dutyCycle(0), _state(false), _stuck(false),
      _periodStart(0), _stuckCheckStart(0), _avgTempDelta(0), _stuckSamples(0) {}

void SSRDriver::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    forceOff();
    _periodStart = millis();
}

void SSRDriver::setDutyCycle(float percent) {
    _dutyCycle = constrain(percent, 0.0f, 100.0f);
}

void SSRDriver::update() {
    uint32_t now = millis();
    uint32_t elapsed = now - _periodStart;

    if (elapsed >= SSR_PERIOD_MS) {
        _periodStart = now;
        elapsed = 0;
    }

    uint32_t onTimeMs = (uint32_t)(_dutyCycle / 100.0f * SSR_PERIOD_MS);
    if (onTimeMs < SSR_MIN_ON_MS) onTimeMs = 0;

    bool shouldBeOn = (elapsed < onTimeMs);

    if (shouldBeOn && !_state) {
        digitalWrite(_pin, HIGH);
        _state = true;
    } else if (!shouldBeOn && _state) {
        digitalWrite(_pin, LOW);
        _state = false;
    }
}

void SSRDriver::forceOff() {
    digitalWrite(_pin, LOW);
    _state = false;
    _dutyCycle = 0;
}

void SSRDriver::reportTempChange(float deltaPerSecond) {
    _stuckSamples++;
    _avgTempDelta += deltaPerSecond;

    // After 30 seconds of high output with no temp rise, flag as stuck
    if (_stuckSamples >= 30) {
        float avg = _avgTempDelta / (float)_stuckSamples;
        if (_dutyCycle > 50.0f && avg < 0.5f) {
            _stuck = true;
        } else {
            _stuck = false;
        }
        _avgTempDelta = 0;
        _stuckSamples = 0;
    }
}
