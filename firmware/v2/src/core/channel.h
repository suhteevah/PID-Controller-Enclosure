#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "config.h"
#include "pid.h"
#include "core/autotune.h"

// Forward declarations (drivers are injected)
class Thermocouple;
class SSRDriver;

enum class ChannelState : uint8_t {
    OFF,
    HEATING,
    HOLDING,
    COOLDOWN,
    AUTOTUNE,
    FAULT
};

// Command from UI/Network → PID task
struct ChannelCommand {
    enum Type : uint8_t {
        CMD_ENABLE,
        CMD_DISABLE,
        CMD_SET_TEMP,
        CMD_ADJUST_TEMP,
        CMD_SET_PID,
        CMD_START_AUTOTUNE,
        CMD_CANCEL_AUTOTUNE,
        CMD_LOAD_PROFILE,
        CMD_CLEAR_FAULT
    };
    Type type;
    uint8_t channel;
    float value;            // Temperature or delta
    float kp, ki, kd;      // For CMD_SET_PID
    uint8_t profileIndex;   // For CMD_LOAD_PROFILE
};

// Temperature update from PID task → UI/Network
struct TempUpdate {
    uint8_t channel;
    float currentTemp;
    float targetTemp;
    float pidOutput;
    ChannelState state;
    uint8_t tcStatus;       // Cast of TCStatus enum
};

class Channel {
public:
    Channel();

    void begin(uint8_t index, uint8_t ssrPin, uint8_t tcCsPin);
    void update();          // Called from PID task

    // Control
    void enable();
    void disable();
    void setTargetTemp(float tempF);
    void adjustTargetTemp(float delta);
    void setPIDTunings(float kp, float ki, float kd);

    // Autotune
    void startAutotune();
    void cancelAutotune();
    bool isAutotuning() const { return _state == ChannelState::AUTOTUNE; }
    float getAutotuneProgress() const;
    AutotuneResult getAutotuneResult() const;

    // SSR control
    void updateSSR();

    // State getters
    ChannelState getState() const       { return _state; }
    float getCurrentTemp() const;
    float getTargetTemp() const         { return _targetTempF; }
    float getPIDOutput() const          { return _pid.getOutput(); }
    bool isActive() const;
    bool isFaulted() const              { return _state == ChannelState::FAULT; }
    uint8_t getIndex() const            { return _index; }
    const PIDController& getPID() const { return _pid; }

    // Thermocouple access
    uint8_t getTCStatusRaw() const;
    bool isTCOk() const;
    uint8_t getTCErrorCount() const;

    const char* getStateString() const;

    // Fill a TempUpdate struct for queue publishing
    TempUpdate getTempUpdate() const;

private:
    uint8_t _index;
    uint8_t _ssrPin;
    float _targetTempF;

    PIDController _pid;
    PIDAutotuner _autotuner;
    Thermocouple* _tc;
    ChannelState _state;

    // SSR time-proportioning
    uint32_t _ssrPeriodStart;
    bool _ssrState;

    uint32_t _lastActiveTime;

    void setState(ChannelState s);
    void ssrOn();
    void ssrOff();
    void checkFaults();
};
