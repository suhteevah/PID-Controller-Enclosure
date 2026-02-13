#pragma once

#include <Arduino.h>
#include "config.h"
#include "pid_controller.h"
#include "thermocouple.h"

// Channel state machine
enum class ChannelState {
    OFF,        // Channel disabled
    HEATING,    // Actively heating toward setpoint
    HOLDING,    // At setpoint, maintaining temperature
    COOLDOWN,   // Cooling down after disable
    FAULT       // Error condition - output disabled
};

class Channel {
public:
    Channel();

    void begin(uint8_t index, uint8_t ssrPin, uint8_t tcCsPin);
    void update();

    // Control
    void enable();
    void disable();
    void setTargetTemp(float tempF);
    void adjustTargetTemp(float delta);

    // SSR time-proportioning output
    void updateSSR();

    // State
    ChannelState getState() const       { return _state; }
    float getCurrentTemp() const        { return _thermocouple.getTemperatureF(); }
    float getTargetTemp() const         { return _targetTempF; }
    float getPIDOutput() const          { return _pid.getOutput(); }
    bool isActive() const               { return _state == ChannelState::HEATING || _state == ChannelState::HOLDING; }
    bool isFaulted() const              { return _state == ChannelState::FAULT; }
    uint8_t getIndex() const            { return _index; }
    TCStatus getTCStatus() const        { return _thermocouple.getStatus(); }
    const Thermocouple& getTC() const   { return _thermocouple; }
    const PIDController& getPID() const { return _pid; }

    // PID tuning
    void setPIDTunings(float kp, float ki, float kd);

    const char* getStateString() const;

private:
    uint8_t _index;
    uint8_t _ssrPin;
    float _targetTempF;

    PIDController _pid;
    Thermocouple _thermocouple;
    ChannelState _state;

    // SSR time-proportioning
    uint32_t _ssrPeriodStart;
    bool _ssrState;

    // Fault tracking
    uint32_t _lastActiveTime;

    void setState(ChannelState newState);
    void ssrOn();
    void ssrOff();
    void checkFaults();
};
