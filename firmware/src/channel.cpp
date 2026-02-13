#include "channel.h"

Channel::Channel()
    : _index(0), _ssrPin(0), _targetTempF(TEMP_DEFAULT_F),
      _state(ChannelState::OFF),
      _ssrPeriodStart(0), _ssrState(false),
      _lastActiveTime(0) {}

void Channel::begin(uint8_t index, uint8_t ssrPin, uint8_t tcCsPin) {
    _index = index;
    _ssrPin = ssrPin;

    // Configure SSR output
    pinMode(_ssrPin, OUTPUT);
    ssrOff();

    // Initialize thermocouple
    _thermocouple.begin(tcCsPin);

    // Initialize PID
    _pid.begin(PID_KP_DEFAULT, PID_KI_DEFAULT, PID_KD_DEFAULT, PID_SAMPLE_MS);
    _pid.setOutputLimits(PID_OUTPUT_MIN, PID_OUTPUT_MAX);
    _pid.setSetpoint(_targetTempF);
    _pid.setEnabled(false);

    _state = ChannelState::OFF;
    _ssrPeriodStart = millis();
}

void Channel::update() {
    // Always read thermocouple
    _thermocouple.update();

    // Check for faults
    checkFaults();

    if (_state == ChannelState::FAULT || _state == ChannelState::OFF) {
        ssrOff();
        return;
    }

    // Update PID
    if (_thermocouple.isOk()) {
        float currentTemp = _thermocouple.getTemperatureF();
        _pid.compute(currentTemp);

        // Update state based on temperature proximity to setpoint
        float error = abs(_targetTempF - currentTemp);
        if (_state == ChannelState::HEATING && error < 10.0f) {
            setState(ChannelState::HOLDING);
        } else if (_state == ChannelState::HOLDING && error > 20.0f) {
            setState(ChannelState::HEATING);
        }

        _lastActiveTime = millis();
    }

    // Update SSR output
    updateSSR();
}

void Channel::enable() {
    if (_state == ChannelState::FAULT) return;

    _pid.setSetpoint(_targetTempF);
    _pid.setEnabled(true);
    _pid.reset();
    _lastActiveTime = millis();
    setState(ChannelState::HEATING);
}

void Channel::disable() {
    _pid.setEnabled(false);
    ssrOff();
    if (_thermocouple.getTemperatureF() > 150.0f) {
        setState(ChannelState::COOLDOWN);
    } else {
        setState(ChannelState::OFF);
    }
}

void Channel::setTargetTemp(float tempF) {
    _targetTempF = constrain(tempF, TEMP_MIN_F, TEMP_MAX_F);
    _pid.setSetpoint(_targetTempF);
}

void Channel::adjustTargetTemp(float delta) {
    setTargetTemp(_targetTempF + delta);
}

void Channel::updateSSR() {
    if (!_pid.isEnabled() || _state == ChannelState::FAULT) {
        ssrOff();
        return;
    }

    float output = _pid.getOutput();
    uint32_t now = millis();
    uint32_t elapsed = now - _ssrPeriodStart;

    // New period
    if (elapsed >= SSR_PERIOD_MS) {
        _ssrPeriodStart = now;
        elapsed = 0;
    }

    // Calculate on-time from PID output percentage
    uint32_t onTimeMs = (uint32_t)(output / 100.0f * SSR_PERIOD_MS);

    // Apply minimum on-time threshold
    if (onTimeMs < SSR_MIN_ON_MS) {
        onTimeMs = 0;
    }

    if (elapsed < onTimeMs) {
        ssrOn();
    } else {
        ssrOff();
    }
}

void Channel::setPIDTunings(float kp, float ki, float kd) {
    _pid.setTunings(kp, ki, kd);
}

void Channel::checkFaults() {
    // Thermocouple fault: too many consecutive errors
    if (_thermocouple.getErrorCount() >= TC_ERROR_COUNT_MAX && isActive()) {
        ssrOff();
        _pid.setEnabled(false);
        setState(ChannelState::FAULT);
        return;
    }

    // Over-temperature protection
    if (_thermocouple.isOk() && isActive()) {
        float currentTemp = _thermocouple.getTemperatureF();
        if (currentTemp >= TEMP_ABS_MAX_F) {
            ssrOff();
            _pid.setEnabled(false);
            setState(ChannelState::FAULT);
            return;
        }
    }

    // Cooldown → Off transition
    if (_state == ChannelState::COOLDOWN) {
        if (_thermocouple.isOk() && _thermocouple.getTemperatureF() < 150.0f) {
            setState(ChannelState::OFF);
        }
    }
}

void Channel::setState(ChannelState newState) {
    _state = newState;
}

void Channel::ssrOn() {
    if (!_ssrState) {
        digitalWrite(_ssrPin, HIGH);
        _ssrState = true;
    }
}

void Channel::ssrOff() {
    if (_ssrState) {
        digitalWrite(_ssrPin, LOW);
        _ssrState = false;
    }
}

const char* Channel::getStateString() const {
    switch (_state) {
        case ChannelState::OFF:      return "OFF";
        case ChannelState::HEATING:  return "HEAT";
        case ChannelState::HOLDING:  return "HOLD";
        case ChannelState::COOLDOWN: return "COOL";
        case ChannelState::FAULT:    return "FAULT";
        default:                     return "???";
    }
}
