#pragma once

#include <Arduino.h>
#include "config.h"

// SSR driver with time-proportioning and stuck detection.
// The SSR switches AC power in a slow on/off cycle (1s period)
// with duty cycle controlled by PID output percentage.

class SSRDriver {
public:
    SSRDriver();

    void begin(uint8_t pin);
    void setDutyCycle(float percent);   // 0-100%
    void update();                      // Call in PID loop
    void forceOff();

    bool isOn() const       { return _state; }
    float getDutyCycle() const { return _dutyCycle; }
    uint8_t getPin() const  { return _pin; }

    // Stuck detection (requires external temp feedback)
    void reportTempChange(float deltaPerSecond);
    bool isStuck() const    { return _stuck; }

private:
    uint8_t _pin;
    float _dutyCycle;
    bool _state;
    bool _stuck;

    uint32_t _periodStart;

    // Stuck detection
    uint32_t _stuckCheckStart;
    float _avgTempDelta;
    uint8_t _stuckSamples;
};
