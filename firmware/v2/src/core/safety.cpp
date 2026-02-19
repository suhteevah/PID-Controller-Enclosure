#include "safety.h"

SafetyManager::SafetyManager()
    : _faults(FAULT_NONE), _shutdown(false), _idleTimedOut(false),
      _idleTimeoutMin(IDLE_TIMEOUT_MIN_DEFAULT), _lastActivityTime(0),
      _faultQueue(nullptr),
      _buzzerHead(0), _buzzerTail(0), _buzzerNoteStart(0), _buzzerPlaying(false) {}

void SafetyManager::begin(QueueHandle_t faultQueue) {
    _faultQueue = faultQueue;

    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    pinMode(PIN_STATUS_LED, OUTPUT);

    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    _lastActivityTime = millis();
    _faults = FAULT_NONE;
    _shutdown = false;

    confirmPattern();
}

void SafetyManager::update() {
    feedWatchdog();
    updateBuzzer();

    // Idle timeout check
    if (_idleTimeoutMin > 0 && !_idleTimedOut) {
        uint32_t elapsedMs = millis() - _lastActivityTime;
        uint32_t timeoutMs = _idleTimeoutMin * 60UL * 1000UL;

        if (elapsedMs >= timeoutMs) {
            _idleTimedOut = true;
            setFault(FAULT_IDLE_TIMEOUT);
            alarmPattern();
        }

        // 5-minute warning
        if (timeoutMs > 300000UL) {
            uint32_t warningMs = timeoutMs - 300000UL;
            if (elapsedMs >= warningMs && elapsedMs < warningMs + 1000) {
                beep(2000, 200);
            }
        }
    }

    // Status LED
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
    // Round up to avoid showing 0 when time remains
    return ((timeoutMs - elapsedMs) + 59999UL) / 60000UL;
}

void SafetyManager::setIdleTimeout(uint32_t minutes) {
    _idleTimeoutMin = min(minutes, (uint32_t)IDLE_TIMEOUT_MIN_MAX);
}

void SafetyManager::clearFault(SafetyFault fault) {
    _faults &= ~fault;
}

void SafetyManager::setFault(SafetyFault fault, uint8_t channel, float temp) {
    _faults |= fault;

    if (_faultQueue != nullptr) {
        FaultEvent evt;
        evt.fault = fault;
        evt.channel = channel;
        evt.temperature = temp;
        evt.timestamp = millis();
        xQueueSend(_faultQueue, &evt, 0);  // Non-blocking
    }
}

void SafetyManager::beep(uint16_t freqHz, uint16_t durationMs) {
    enqueueBuzzerNote(freqHz, durationMs);
}

void SafetyManager::alarmPattern() {
    enqueueBuzzerNote(3000, 150);
    enqueueBuzzerNote(0, 100);      // Silence gap
    enqueueBuzzerNote(3000, 150);
    enqueueBuzzerNote(0, 100);
    enqueueBuzzerNote(3000, 150);
}

void SafetyManager::confirmPattern() {
    enqueueBuzzerNote(1500, 80);
    enqueueBuzzerNote(0, 40);
    enqueueBuzzerNote(2000, 80);
}

void SafetyManager::updateBuzzer() {
    if (!_buzzerPlaying) {
        // Try to dequeue next note
        if (_buzzerHead != _buzzerTail) {
            BuzzerNote& note = _buzzerQueue[_buzzerHead];
            if (note.freqHz > 0) {
                tone(PIN_BUZZER, note.freqHz, note.durationMs);
            } else {
                noTone(PIN_BUZZER);
            }
            _buzzerNoteStart = millis();
            _buzzerPlaying = true;
        }
        return;
    }

    // Check if current note is done
    BuzzerNote& current = _buzzerQueue[_buzzerHead];
    if ((millis() - _buzzerNoteStart) >= current.durationMs) {
        noTone(PIN_BUZZER);
        _buzzerHead = (_buzzerHead + 1) % BUZZER_QUEUE_SIZE;
        _buzzerPlaying = false;
    }
}

void SafetyManager::enqueueBuzzerNote(uint16_t freq, uint16_t duration) {
    uint8_t next = (_buzzerTail + 1) % BUZZER_QUEUE_SIZE;
    if (next == _buzzerHead) return;  // Queue full
    _buzzerQueue[_buzzerTail] = { freq, duration };
    _buzzerTail = next;
}

void SafetyManager::emergencyShutdown() {
    _shutdown = true;
    for (int i = 0; i < NUM_CHANNELS; i++) {
        digitalWrite(SSR_PINS[i], LOW);
    }
    alarmPattern();
}

void SafetyManager::feedWatchdog() {
    esp_task_wdt_reset();
}
