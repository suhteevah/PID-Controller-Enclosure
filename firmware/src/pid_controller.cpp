#include "pid_controller.h"

PIDController::PIDController()
    : _kp(PID_KP_DEFAULT), _ki(PID_KI_DEFAULT), _kd(PID_KD_DEFAULT),
      _setpoint(0), _output(0),
      _outputMin(PID_OUTPUT_MIN), _outputMax(PID_OUTPUT_MAX),
      _integral(0), _prevMeasurement(0),
      _pTerm(0), _iTerm(0), _dTerm(0),
      _derivativeFilter(0.1f),
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

    // Scale Ki and Kd to sample time for consistent behavior
    float sampleTimeSec = (float)_sampleTimeMs / 1000.0f;
    _kp = kp;
    _ki = ki * sampleTimeSec;
    _kd = kd / sampleTimeSec;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    _outputMin = min;
    _outputMax = max;

    // Clamp existing integral
    _integral = constrain(_integral, _outputMin, _outputMax);
    _output = constrain(_output, _outputMin, _outputMax);
}

void PIDController::setSetpoint(float setpoint) {
    _setpoint = constrain(setpoint, TEMP_MIN_F, TEMP_MAX_F);
}

float PIDController::compute(float measurement) {
    if (!_enabled) {
        _output = 0;
        return _output;
    }

    uint32_t now = millis();
    uint32_t elapsed = now - _lastComputeTime;

    if (elapsed < _sampleTimeMs) {
        return _output; // Not time yet
    }

    if (_firstRun) {
        _prevMeasurement = measurement;
        _firstRun = false;
        _lastComputeTime = now;
        return _output;
    }

    // Error
    float error = _setpoint - measurement;

    // Proportional term
    _pTerm = _kp * error;

    // Integral term with anti-windup clamping
    _integral += _ki * error;
    _integral = constrain(_integral, _outputMin, _outputMax);
    _iTerm = _integral;

    // Derivative term: derivative-on-measurement to avoid setpoint kick
    // With low-pass filter: dFiltered = alpha * dRaw + (1-alpha) * dPrev
    float dMeasurement = measurement - _prevMeasurement;
    float dRaw = -_kd * dMeasurement;
    _dTerm = _derivativeFilter * dRaw + (1.0f - _derivativeFilter) * _dTerm;

    // Sum and clamp output
    _output = _pTerm + _iTerm + _dTerm;
    _output = constrain(_output, _outputMin, _outputMax);

    // Store state
    _prevMeasurement = measurement;
    _lastComputeTime = now;

    return _output;
}

void PIDController::reset() {
    _integral = 0;
    _prevMeasurement = 0;
    _pTerm = 0;
    _iTerm = 0;
    _dTerm = 0;
    _output = 0;
    _firstRun = true;
    _lastComputeTime = millis();
}

void PIDController::setEnabled(bool enabled) {
    if (enabled && !_enabled) {
        // Transitioning to enabled - reset to avoid integral bump
        reset();
    }
    _enabled = enabled;
    if (!_enabled) {
        _output = 0;
    }
}
