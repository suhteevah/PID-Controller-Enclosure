#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct CalibrationData {
    float offset;
    char coilLabel[16];
    bool enabled;
};

class CalibrationManager {
public:
    CalibrationManager();
    void begin();
    void setCalibration(uint8_t ch, const CalibrationData& data);
    CalibrationData getCalibration(uint8_t ch);
    float getCalibratedTemp(uint8_t ch, float rawTemp);
    bool isCalibrated(uint8_t ch);
    void clearCalibration(uint8_t ch);
private:
    Preferences _prefs;
    CalibrationData _cache[4];
};
