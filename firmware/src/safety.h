#pragma once

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "config.h"

// Safety fault codes (bitmask)
enum SafetyFault : uint8_t {
    FAULT_NONE          = 0x00,
    FAULT_OVERTEMP      = 0x01,
    FAULT_TC_ERROR      = 0x02,
    FAULT_IDLE_TIMEOUT  = 0x04,
    FAULT_WATCHDOG      = 0x08,
    FAULT_SSR_STUCK     = 0x10
};

class SafetyManager {
public:
    SafetyManager();

    void begin();
    void update();

    // Idle timeout management
    void resetIdleTimer();
    bool isIdleTimedOut() const     { return _idleTimedOut; }
    uint32_t getIdleMinRemaining() const;

    // Fault management
    uint8_t getFaults() const       { return _faults; }
    bool hasFault() const           { return _faults != FAULT_NONE; }
    void clearFault(SafetyFault fault);
    void setFault(SafetyFault fault);

    // Buzzer alerts
    void beep(uint16_t freqHz, uint16_t durationMs);
    void alarmBeep();
    void confirmBeep();

    // Emergency shutdown - disables all outputs
    void emergencyShutdown();
    bool isShutdown() const         { return _shutdown; }

    // Watchdog
    void feedWatchdog();

    // Set idle timeout in minutes (0 = disabled)
    void setIdleTimeout(uint32_t minutes) { _idleTimeoutMin = minutes; }
    uint32_t getIdleTimeout() const { return _idleTimeoutMin; }

private:
    uint8_t _faults;
    bool _shutdown;
    bool _idleTimedOut;

    uint32_t _idleTimeoutMin;
    uint32_t _lastActivityTime;
    uint32_t _lastBeepTime;
};
