#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"

struct SessionRecord {
    uint32_t startTime;
    uint32_t durationSec;
    float peakTempF;
    float avgTempF;
    float targetTempF;
    uint8_t channel;
    float energyEstWh;
};

class SessionLogger {
public:
    SessionLogger();
    void begin();
    void update();
    void startSession(uint8_t ch, float targetTemp);
    void endSession(uint8_t ch);
    void addDataPoint(uint8_t ch, float temp, float pidOutput);
    uint16_t getSessionCount();
    SessionRecord getSession(uint16_t idx);
    String exportJSON();
    void clearAll();
private:
    struct ActiveSession {
        bool active;
        uint32_t startMs;
        float peakTemp;
        float tempSum;
        float energySum;
        uint32_t sampleCount;
        float targetTemp;
    };
    ActiveSession _active[NUM_CHANNELS];
    static const char* LOG_PATH;
    void appendRecord(const SessionRecord& rec);
};
