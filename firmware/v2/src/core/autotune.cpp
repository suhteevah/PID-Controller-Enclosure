#include "autotune.h"

PIDAutotuner::PIDAutotuner()
    : _state(AutotuneState::IDLE),
      _setpoint(0), _outputHigh(100.0f), _outputLow(0.0f), _currentOutput(0),
      _targetOscillations(5), _oscillationCount(0),
      _aboveSetpoint(false), _lastCrossTime(0),
      _peakHigh(0), _peakLow(9999),
      _periodSum(0), _amplitudeSum(0), _periodCount(0),
      _startTime(0), _timeoutMs(300000) // 5 min default timeout
{
    _result = { 0, 0, 0, 0, 0, false };
}

void PIDAutotuner::begin(float setpoint, float outputHigh, float outputLow) {
    _setpoint = setpoint;
    _outputHigh = outputHigh;
    _outputLow = outputLow;
    _currentOutput = _outputHigh;

    _oscillationCount = 0;
    _aboveSetpoint = false;
    _lastCrossTime = 0;
    _peakHigh = 0;
    _peakLow = 9999;
    _periodSum = 0;
    _amplitudeSum = 0;
    _periodCount = 0;

    _startTime = millis();
    _result = { 0, 0, 0, 0, 0, false };
    _state = AutotuneState::WAITING_HEAT;
}

float PIDAutotuner::update(float measurement) {
    if (_state == AutotuneState::IDLE ||
        _state == AutotuneState::COMPLETE ||
        _state == AutotuneState::FAILED) {
        return 0;
    }

    // Timeout check
    if ((millis() - _startTime) > _timeoutMs) {
        _state = AutotuneState::FAILED;
        return 0;
    }

    bool nowAbove = (measurement > _setpoint);

    if (_state == AutotuneState::WAITING_HEAT) {
        // Wait for first crossing of setpoint
        if (nowAbove) {
            _state = AutotuneState::OSCILLATING;
            _aboveSetpoint = true;
            _lastCrossTime = millis();
            _currentOutput = _outputLow;
            _peakHigh = measurement;
        } else {
            _currentOutput = _outputHigh;
        }
        return _currentOutput;
    }

    // OSCILLATING state - relay feedback
    // Track peaks
    if (nowAbove) {
        if (measurement > _peakHigh) _peakHigh = measurement;
    } else {
        if (measurement < _peakLow) _peakLow = measurement;
    }

    // Detect zero crossing (setpoint crossing)
    if (nowAbove != _aboveSetpoint) {
        uint32_t now = millis();

        if (_aboveSetpoint && !nowAbove) {
            // Crossed below setpoint - switch to heating
            _currentOutput = _outputHigh;

            // Record half-period (high to low crossing)
            if (_lastCrossTime > 0) {
                float halfPeriod = (float)(now - _lastCrossTime) / 1000.0f;
                _periodSum += halfPeriod * 2.0f;  // Full period estimate
                _amplitudeSum += (_peakHigh - _peakLow);
                _periodCount++;
                _oscillationCount++;
            }
            _peakLow = measurement;
            _lastCrossTime = now;

        } else if (!_aboveSetpoint && nowAbove) {
            // Crossed above setpoint - switch to cooling
            _currentOutput = _outputLow;
            _peakHigh = measurement;
            _lastCrossTime = now;
        }

        _aboveSetpoint = nowAbove;

        // Check if we have enough oscillations
        if (_oscillationCount >= _targetOscillations) {
            computeResult();
            _state = AutotuneState::COMPLETE;
            return 0;
        }
    }

    return _currentOutput;
}

void PIDAutotuner::cancel() {
    _state = AutotuneState::IDLE;
    _currentOutput = 0;
}

float PIDAutotuner::getProgress() const {
    if (_state == AutotuneState::COMPLETE) return 1.0f;
    if (_state == AutotuneState::IDLE || _state == AutotuneState::FAILED) return 0.0f;
    if (_targetOscillations == 0) return 0.0f;
    return (float)_oscillationCount / (float)_targetOscillations;
}

void PIDAutotuner::computeResult() {
    if (_periodCount == 0) {
        _result.valid = false;
        return;
    }

    // Average period and amplitude
    float avgPeriod = _periodSum / (float)_periodCount;
    float avgAmplitude = _amplitudeSum / (float)_periodCount;

    if (avgAmplitude < 0.1f || avgPeriod < 0.1f) {
        _result.valid = false;
        return;
    }

    // Ultimate gain: Ku = 4d / (pi * a)
    // where d = relay output amplitude, a = process oscillation amplitude
    float relayAmplitude = _outputHigh - _outputLow;
    float Ku = (4.0f * relayAmplitude) / (3.14159f * avgAmplitude);
    float Tu = avgPeriod;

    _result.ultimateGain = Ku;
    _result.ultimatePeriod = Tu;

    // Ziegler-Nichols PID tuning rules
    _result.kp = 0.6f * Ku;
    _result.ki = 1.2f * Ku / Tu;
    _result.kd = 0.075f * Ku * Tu;
    _result.valid = true;
}
