#pragma once

#include <Arduino.h>
#include "config.h"

// PID Controller with anti-windup, derivative filtering,
// and derivative-on-measurement to avoid setpoint kick.
class PIDController {
public:
    PIDController();

    void begin(float kp, float ki, float kd, uint32_t sampleTimeMs);
    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void setSetpoint(float setpoint);

    // Compute PID output given current measurement.
    // Returns output in range [outputMin, outputMax] as a percentage.
    float compute(float measurement);

    // Reset integrator and derivative state
    void reset();

    // Enable/disable the controller
    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }

    // Getters
    float getSetpoint() const   { return _setpoint; }
    float getOutput() const     { return _output; }
    float getKp() const         { return _kp; }
    float getKi() const         { return _ki; }
    float getKd() const         { return _kd; }
    float getPTerm() const      { return _pTerm; }
    float getITerm() const      { return _iTerm; }
    float getDTerm() const      { return _dTerm; }

private:
    float _kp, _ki, _kd;
    float _setpoint;
    float _output;
    float _outputMin, _outputMax;
    float _integral;
    float _prevMeasurement;
    float _pTerm, _iTerm, _dTerm;
    float _derivativeFilter;

    uint32_t _sampleTimeMs;
    uint32_t _lastComputeTime;
    bool _enabled;
    bool _firstRun;
};
