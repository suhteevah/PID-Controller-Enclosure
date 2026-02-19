#include "channel.h"
#include "drivers/thermocouple.h"

Channel::Channel()
    : _index(0), _ssrPin(0), _targetTempF(TEMP_DEFAULT_F),
      _tc(nullptr), _state(ChannelState::OFF),
      _ssrPeriodStart(0), _ssrState(false), _lastActiveTime(0) {}

void Channel::begin(uint8_t index, uint8_t ssrPin, uint8_t tcCsPin) {
    _index = index;
    _ssrPin = ssrPin;

    pinMode(_ssrPin, OUTPUT);
    ssrOff();

    _tc = new Thermocouple();
    _tc->begin(tcCsPin);

    _pid.begin(PID_KP_DEFAULT, PID_KI_DEFAULT, PID_KD_DEFAULT, PID_SAMPLE_MS);
    _pid.setOutputLimits(PID_OUTPUT_MIN, PID_OUTPUT_MAX);
    _pid.setSetpoint(_targetTempF);
    _pid.setEnabled(false);

    _state = ChannelState::OFF;
    _ssrPeriodStart = millis();
}

void Channel::update() {
    if (_tc) _tc->update();
    checkFaults();

    if (_state == ChannelState::FAULT || _state == ChannelState::OFF) {
        ssrOff();
        return;
    }

    if (_state == ChannelState::AUTOTUNE) {
        if (_tc && _tc->isOk()) {
            float output = _autotuner.update(_tc->getTemperatureF());

            if (_autotuner.getState() == AutotuneState::COMPLETE) {
                AutotuneResult result = _autotuner.getResult();
                if (result.valid) {
                    _pid.setTunings(result.kp, result.ki, result.kd);
                }
                enable();  // Return to normal operation
            } else if (_autotuner.getState() == AutotuneState::FAILED) {
                enable();
            }

            // Apply autotune relay output via SSR
            uint32_t now = millis();
            uint32_t elapsed = now - _ssrPeriodStart;
            if (elapsed >= SSR_PERIOD_MS) { _ssrPeriodStart = now; elapsed = 0; }
            uint32_t onTime = (uint32_t)(output / 100.0f * SSR_PERIOD_MS);
            if (elapsed < onTime) ssrOn(); else ssrOff();
        }
        return;
    }

    // Normal PID operation
    if (_tc && _tc->isOk()) {
        float currentTemp = _tc->getTemperatureF();
        _pid.compute(currentTemp);

        float error = abs(_targetTempF - currentTemp);
        if (_state == ChannelState::HEATING && error < TEMP_HOLDING_BAND_F) {
            setState(ChannelState::HOLDING);
        } else if (_state == ChannelState::HOLDING && error > TEMP_HEATING_BAND_F) {
            setState(ChannelState::HEATING);
        }
        _lastActiveTime = millis();
    }

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
    if (_tc && _tc->isOk() && _tc->getTemperatureF() > TEMP_COOLDOWN_THRESH_F) {
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

void Channel::setPIDTunings(float kp, float ki, float kd) {
    _pid.setTunings(kp, ki, kd);
}

void Channel::startAutotune() {
    if (_state == ChannelState::FAULT) return;
    _autotuner.begin(_targetTempF, 100.0f, 0.0f);
    _pid.setEnabled(false);
    setState(ChannelState::AUTOTUNE);
}

void Channel::cancelAutotune() {
    _autotuner.cancel();
    enable();
}

float Channel::getAutotuneProgress() const {
    return _autotuner.getProgress();
}

AutotuneResult Channel::getAutotuneResult() const {
    return _autotuner.getResult();
}

void Channel::updateSSR() {
    if (!_pid.isEnabled() || _state == ChannelState::FAULT) { ssrOff(); return; }

    float output = _pid.getOutput();
    uint32_t now = millis();
    uint32_t elapsed = now - _ssrPeriodStart;
    if (elapsed >= SSR_PERIOD_MS) { _ssrPeriodStart = now; elapsed = 0; }

    uint32_t onTimeMs = (uint32_t)(output / 100.0f * SSR_PERIOD_MS);
    if (onTimeMs < SSR_MIN_ON_MS) onTimeMs = 0;

    if (elapsed < onTimeMs) ssrOn(); else ssrOff();
}

float Channel::getCurrentTemp() const {
    return (_tc && _tc->isOk()) ? _tc->getTemperatureF() : 0.0f;
}

bool Channel::isActive() const {
    return _state == ChannelState::HEATING || _state == ChannelState::HOLDING;
}

uint8_t Channel::getTCStatusRaw() const {
    return _tc ? (uint8_t)_tc->getStatus() : 4; // NOT_READY
}

bool Channel::isTCOk() const {
    return _tc && _tc->isOk();
}

uint8_t Channel::getTCErrorCount() const {
    return _tc ? _tc->getErrorCount() : 0;
}

void Channel::checkFaults() {
    if (_tc && _tc->getErrorCount() >= TC_ERROR_COUNT_MAX && isActive()) {
        ssrOff(); _pid.setEnabled(false);
        setState(ChannelState::FAULT);
        return;
    }

    if (_tc && _tc->isOk() && isActive()) {
        if (_tc->getTemperatureF() >= TEMP_ABS_MAX_F) {
            ssrOff(); _pid.setEnabled(false);
            setState(ChannelState::FAULT);
            return;
        }
    }

    // SSR stuck detection: if PID output is 0 but we detect temp still rising
    // (future: requires current sensor or temp trend analysis)

    if (_state == ChannelState::COOLDOWN) {
        if (_tc && _tc->isOk() && _tc->getTemperatureF() < TEMP_COOLDOWN_THRESH_F) {
            setState(ChannelState::OFF);
        }
    }
}

TempUpdate Channel::getTempUpdate() const {
    TempUpdate u;
    u.channel = _index;
    u.currentTemp = getCurrentTemp();
    u.targetTemp = _targetTempF;
    u.pidOutput = _pid.getOutput();
    u.state = _state;
    u.tcStatus = getTCStatusRaw();
    return u;
}

void Channel::setState(ChannelState s) { _state = s; }
void Channel::ssrOn() { if (!_ssrState) { digitalWrite(_ssrPin, HIGH); _ssrState = true; } }
void Channel::ssrOff() { if (_ssrState) { digitalWrite(_ssrPin, LOW); _ssrState = false; } }

const char* Channel::getStateString() const {
    switch (_state) {
        case ChannelState::OFF:      return "OFF";
        case ChannelState::HEATING:  return "HEAT";
        case ChannelState::HOLDING:  return "HOLD";
        case ChannelState::COOLDOWN: return "COOL";
        case ChannelState::AUTOTUNE: return "TUNE";
        case ChannelState::FAULT:    return "FAULT";
        default:                     return "???";
    }
}
