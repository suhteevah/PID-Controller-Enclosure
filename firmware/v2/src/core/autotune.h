#pragma once

#include <Arduino.h>
#include "config.h"

// PID Auto-Tuner using relay feedback (Ziegler-Nichols) method.
// Oscillates the output between on/off around the setpoint,
// measures the oscillation period and amplitude, then computes
// optimal Kp/Ki/Kd values.

enum class AutotuneState {
    IDLE,           // Not running
    WAITING_HEAT,   // Waiting for temp to reach setpoint area
    OSCILLATING,    // Performing relay oscillations
    COMPLETE,       // Tuning complete, results available
    FAILED          // Tuning failed (timeout, bad data)
};

struct AutotuneResult {
    float kp;
    float ki;
    float kd;
    float ultimateGain;     // Ku
    float ultimatePeriod;   // Tu (seconds)
    bool valid;
};

class PIDAutotuner {
public:
    PIDAutotuner();

    // Start auto-tune for a given setpoint. outputHigh/Low are the
    // relay output percentages (e.g., 100% and 0%).
    void begin(float setpoint, float outputHigh = 100.0f, float outputLow = 0.0f);

    // Call each PID cycle with current temperature.
    // Returns the output to apply (either outputHigh or outputLow).
    float update(float measurement);

    // Cancel auto-tune
    void cancel();

    AutotuneState getState() const      { return _state; }
    AutotuneResult getResult() const    { return _result; }
    float getProgress() const;          // 0.0 - 1.0

    // Configuration
    void setOscillationCount(uint8_t count) { _targetOscillations = count; }
    void setTimeout(uint32_t ms)            { _timeoutMs = ms; }

private:
    AutotuneState _state;
    AutotuneResult _result;

    float _setpoint;
    float _outputHigh;
    float _outputLow;
    float _currentOutput;

    // Oscillation tracking
    uint8_t _targetOscillations;
    uint8_t _oscillationCount;
    bool _aboveSetpoint;
    uint32_t _lastCrossTime;
    float _peakHigh;
    float _peakLow;

    // Accumulated measurements for averaging
    float _periodSum;
    float _amplitudeSum;
    uint8_t _periodCount;

    uint32_t _startTime;
    uint32_t _timeoutMs;

    void computeResult();
};
