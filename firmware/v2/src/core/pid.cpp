#include "pid.h"

PIDController::PIDController()
    : _kp(PID_KP_DEFAULT), _ki(PID_KI_DEFAULT), _kd(PID_KD_DEFAULT),
      _setpoint(0), _output(0),
      _outputMin(PID_OUTPUT_MIN), _outputMax(PID_OUTPUT_MAX),
      _integral(0), _prevMeasurement(0), _lastError(0),
      _pTerm(0), _iTerm(0), _dTerm(0),
      _derivativeFilterAlpha(PID_DERIVATIVE_FILTER),
      _sampleTimeMs(PID_SAMPLE_MS),
      _lastComputeTime(0),
      _enabled(false), _firstRun(true) {}

void PIDController::begin(float kp, float ki, float kd, uint32_t sampleTimeMs) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _sampleTimeMs = sampleTimeMs;
    reset();
}

void PIDController::setTunings(float kp, float ki, float kd) {
    if (kp < 0 || ki < 0 || kd < 0) return;

    float sampleTimeSec = (float)_sampleTimeMs / 1000.0f;
    _kp = kp;
    _ki = ki * sampleTimeSec;
    _kd = kd / sampleTimeSec;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    _outputMin = min;
    _outputMax = max;
    _integral = constrain(_integral, _outputMin, _outputMax);
    _output = constrain(_output, _outputMin, _outputMax);
}

void PIDController::setSetpoint(float setpoint) {
    _setpoint = constrain(setpoint, TEMP_MIN_F, TEMP_MAX_F);
}

void PIDController::setDerivativeFilter(float alpha) {
    _derivativeFilterAlpha = constrain(alpha, 0.01f, 1.0f);
}

float PIDController::compute(float measurement) {
    if (!_enabled) {
        _output = 0;
        return _output;
    }

    uint32_t now = millis();
    if ((now - _lastComputeTime) < _sampleTimeMs) {
        return _output;
    }

    if (_firstRun) {
        _prevMeasurement = measurement;
        _firstRun = false;
        _lastComputeTime = now;
        return _output;
    }

    float error = _setpoint - measurement;
    _lastError = error;

    // Proportional
    _pTerm = _kp * error;

    // Integral with anti-windup clamping
    _integral += _ki * error;
    _integral = constrain(_integral, _outputMin, _outputMax);
    _iTerm = _integral;

    // Derivative on measurement (not error) with low-pass filter
    float dMeasurement = measurement - _prevMeasurement;
    float dRaw = -_kd * dMeasurement;
    _dTerm = _derivativeFilterAlpha * dRaw + (1.0f - _derivativeFilterAlpha) * _dTerm;

    // Sum and clamp
    _output = constrain(_pTerm + _iTerm + _dTerm, _outputMin, _outputMax);

    _prevMeasurement = measurement;
    _lastComputeTime = now;

    return _output;
}

void PIDController::reset() {
    _integral = 0;
    _prevMeasurement = 0;
    _lastError = 0;
    _pTerm = _iTerm = _dTerm = 0;
    _output = 0;
    _firstRun = true;
    _lastComputeTime = millis();
}

void PIDController::setEnabled(bool enabled) {
    if (enabled && !_enabled) {
        reset();  // Bumpless transfer
    }
    _enabled = enabled;
    if (!_enabled) _output = 0;
}
