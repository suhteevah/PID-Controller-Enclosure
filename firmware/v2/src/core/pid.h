#pragma once

#include <Arduino.h>
#include "config.h"

// Production-grade PID controller with:
// - Anti-windup (integral clamping)
// - Derivative-on-measurement (no setpoint kick)
// - Low-pass derivative filter
// - Bumpless transfer on enable/disable
// - Thread-safe (designed for RTOS)

class PIDController {
public:
    PIDController();

    void begin(float kp, float ki, float kd, uint32_t sampleTimeMs);
    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void setSetpoint(float setpoint);
    void setDerivativeFilter(float alpha);

    // Compute PID - returns output percentage [0-100]
    float compute(float measurement);

    void reset();
    void setEnabled(bool enabled);

    // Getters
    bool isEnabled() const      { return _enabled; }
    float getSetpoint() const   { return _setpoint; }
    float getOutput() const     { return _output; }
    float getKp() const         { return _kp; }
    float getKi() const         { return _ki; }
    float getKd() const         { return _kd; }
    float getPTerm() const      { return _pTerm; }
    float getITerm() const      { return _iTerm; }
    float getDTerm() const      { return _dTerm; }
    float getError() const      { return _lastError; }

private:
    float _kp, _ki, _kd;
    float _setpoint;
    float _output;
    float _outputMin, _outputMax;

    float _integral;
    float _prevMeasurement;
    float _lastError;
    float _pTerm, _iTerm, _dTerm;
    float _derivativeFilterAlpha;

    uint32_t _sampleTimeMs;
    uint32_t _lastComputeTime;
    bool _enabled;
    bool _firstRun;
};
