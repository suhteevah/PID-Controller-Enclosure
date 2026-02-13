#include "safety.h"

SafetyManager::SafetyManager()
    : _faults(FAULT_NONE), _shutdown(false), _idleTimedOut(false),
      _idleTimeoutMin(IDLE_TIMEOUT_MIN), _lastActivityTime(0),
      _lastBeepTime(0) {}

void SafetyManager::begin() {
    // Initialize buzzer pin
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    // Initialize status LED
    pinMode(PIN_STATUS_LED, OUTPUT);

    // Initialize hardware watchdog
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    _lastActivityTime = millis();
    _faults = FAULT_NONE;
    _shutdown = false;

    confirmBeep();
}

void SafetyManager::update() {
    // Feed the watchdog
    feedWatchdog();

    // Check idle timeout
    if (_idleTimeoutMin > 0 && !_idleTimedOut) {
        uint32_t elapsedMs = millis() - _lastActivityTime;
        uint32_t timeoutMs = _idleTimeoutMin * 60UL * 1000UL;

        if (elapsedMs >= timeoutMs) {
            _idleTimedOut = true;
            setFault(FAULT_IDLE_TIMEOUT);
            alarmBeep();
        }

        // Warning beep at 5 minutes remaining
        uint32_t warningMs = timeoutMs - (5UL * 60UL * 1000UL);
        if (elapsedMs >= warningMs && elapsedMs < warningMs + 1000) {
            beep(2000, 200);
        }
    }

    // Blink status LED if faulted
    if (hasFault()) {
        digitalWrite(PIN_STATUS_LED, (millis() / 250) % 2);
    } else if (!_shutdown) {
        digitalWrite(PIN_STATUS_LED, HIGH);
    } else {
        digitalWrite(PIN_STATUS_LED, LOW);
    }
}

void SafetyManager::resetIdleTimer() {
    _lastActivityTime = millis();
    _idleTimedOut = false;
    clearFault(FAULT_IDLE_TIMEOUT);
}

uint32_t SafetyManager::getIdleMinRemaining() const {
    if (_idleTimeoutMin == 0) return 0;
    uint32_t elapsedMs = millis() - _lastActivityTime;
    uint32_t timeoutMs = _idleTimeoutMin * 60UL * 1000UL;
    if (elapsedMs >= timeoutMs) return 0;
    return (timeoutMs - elapsedMs) / 60000UL;
}

void SafetyManager::clearFault(SafetyFault fault) {
    _faults &= ~fault;
}

void SafetyManager::setFault(SafetyFault fault) {
    _faults |= fault;
}

void SafetyManager::beep(uint16_t freqHz, uint16_t durationMs) {
    tone(PIN_BUZZER, freqHz, durationMs);
}

void SafetyManager::alarmBeep() {
    // Three rapid beeps
    for (int i = 0; i < 3; i++) {
        beep(3000, 150);
        delay(100);
    }
}

void SafetyManager::confirmBeep() {
    beep(1500, 80);
    delay(80);
    beep(2000, 80);
}

void SafetyManager::emergencyShutdown() {
    _shutdown = true;
    // Force all SSR pins LOW
    for (int i = 0; i < NUM_CHANNELS; i++) {
        digitalWrite(SSR_PINS[i], LOW);
    }
    alarmBeep();
}

void SafetyManager::feedWatchdog() {
    esp_task_wdt_reset();
}
