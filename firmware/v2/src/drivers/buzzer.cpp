#include "buzzer.h"

Buzzer::Buzzer()
    : _head(0), _tail(0), _noteStartTime(0), _playing(false) {}

void Buzzer::begin() {
    pinMode(PIN_BUZZER, OUTPUT);
    noTone(PIN_BUZZER);
}

void Buzzer::update() {
    if (!_playing) {
        if (_head != _tail) {
            Note& n = _queue[_head];
            if (n.freq > 0) tone(PIN_BUZZER, n.freq, n.duration);
            else noTone(PIN_BUZZER);
            _noteStartTime = millis();
            _playing = true;
        }
        return;
    }

    Note& current = _queue[_head];
    if ((millis() - _noteStartTime) >= current.duration) {
        noTone(PIN_BUZZER);
        _head = (_head + 1) % QUEUE_SIZE;
        _playing = false;
    }
}

void Buzzer::playNote(uint16_t freqHz, uint16_t durationMs) {
    enqueue(freqHz, durationMs);
}

void Buzzer::playSilence(uint16_t durationMs) {
    enqueue(0, durationMs);
}

void Buzzer::playConfirm() {
    enqueue(1500, 80);
    enqueue(0, 40);
    enqueue(2000, 80);
}

void Buzzer::playAlarm() {
    for (int i = 0; i < 3; i++) {
        enqueue(3000, 150);
        enqueue(0, 100);
    }
}

void Buzzer::playStartup() {
    enqueue(800, 100);
    enqueue(0, 50);
    enqueue(1200, 100);
    enqueue(0, 50);
    enqueue(1600, 150);
}

void Buzzer::playError() {
    enqueue(400, 200);
    enqueue(0, 100);
    enqueue(400, 200);
}

void Buzzer::stop() {
    noTone(PIN_BUZZER);
    _head = _tail = 0;
    _playing = false;
}

void Buzzer::enqueue(uint16_t freq, uint16_t duration) {
    uint8_t next = (_tail + 1) % QUEUE_SIZE;
    if (next == _head) return;  // Full
    _queue[_tail] = { freq, duration };
    _tail = next;
}
