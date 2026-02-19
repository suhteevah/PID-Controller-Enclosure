#pragma once

#include <Arduino.h>
#include "config.h"

// Non-blocking buzzer driver using a note queue.
// Patterns are enqueued as sequences of (freq, duration) pairs.

class Buzzer {
public:
    Buzzer();

    void begin();
    void update();  // Call from main loop / UI task

    void playNote(uint16_t freqHz, uint16_t durationMs);
    void playSilence(uint16_t durationMs);
    void playConfirm();
    void playAlarm();
    void playStartup();
    void playError();

    bool isPlaying() const { return _playing; }
    void stop();

private:
    struct Note {
        uint16_t freq;
        uint16_t duration;
    };

    static const uint8_t QUEUE_SIZE = 16;
    Note _queue[QUEUE_SIZE];
    uint8_t _head;
    uint8_t _tail;
    uint32_t _noteStartTime;
    bool _playing;

    void enqueue(uint16_t freq, uint16_t duration);
};
