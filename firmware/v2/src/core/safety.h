#pragma once

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "config.h"

// Safety fault codes (bitmask)
enum SafetyFault : uint8_t {
    FAULT_NONE          = 0x00,
    FAULT_OVERTEMP      = 0x01,
    FAULT_TC_ERROR      = 0x02,
    FAULT_IDLE_TIMEOUT  = 0x04,
    FAULT_WATCHDOG      = 0x08,
    FAULT_SSR_STUCK     = 0x10,
    FAULT_POWER_ERROR   = 0x20
};

// Fault event for inter-task communication
struct FaultEvent {
    SafetyFault fault;
    uint8_t channel;       // 0xFF = global
    float temperature;     // Temp at time of fault
    uint32_t timestamp;
};

class SafetyManager {
public:
    SafetyManager();

    void begin(QueueHandle_t faultQueue);
    void update();

    // Idle timeout
    void resetIdleTimer();
    bool isIdleTimedOut() const     { return _idleTimedOut; }
    uint32_t getIdleMinRemaining() const;
    void setIdleTimeout(uint32_t minutes);
    uint32_t getIdleTimeout() const { return _idleTimeoutMin; }

    // Fault management
    uint8_t getFaults() const       { return _faults; }
    bool hasFault() const           { return _faults != FAULT_NONE; }
    void clearFault(SafetyFault fault);
    void setFault(SafetyFault fault, uint8_t channel = 0xFF, float temp = 0);

    // Non-blocking buzzer
    void beep(uint16_t freqHz, uint16_t durationMs);
    void alarmPattern();
    void confirmPattern();
    void updateBuzzer();

    // Emergency shutdown
    void emergencyShutdown();
    bool isShutdown() const { return _shutdown; }

    // Watchdog
    void feedWatchdog();

private:
    uint8_t _faults;
    bool _shutdown;
    bool _idleTimedOut;
    uint32_t _idleTimeoutMin;
    uint32_t _lastActivityTime;

    QueueHandle_t _faultQueue;

    // Non-blocking buzzer state
    struct BuzzerNote {
        uint16_t freqHz;
        uint16_t durationMs;
    };
    static const uint8_t BUZZER_QUEUE_SIZE = 8;
    BuzzerNote _buzzerQueue[BUZZER_QUEUE_SIZE];
    uint8_t _buzzerHead;
    uint8_t _buzzerTail;
    uint32_t _buzzerNoteStart;
    bool _buzzerPlaying;

    void enqueueBuzzerNote(uint16_t freq, uint16_t duration);
};
