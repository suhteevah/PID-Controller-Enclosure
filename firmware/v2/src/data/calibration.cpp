#include "calibration.h"

CalibrationManager::CalibrationManager() {
    for (int i = 0; i < 4; i++) {
        _cache[i] = {0, "", false};
    }
}

void CalibrationManager::begin() {
    _prefs.begin("enail_cal", false);
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        String key = String("cal") + String(i);
        if (_prefs.isKey((key + "en").c_str())) {
            _cache[i].enabled = _prefs.getBool((key + "en").c_str(), false);
            _cache[i].offset = _prefs.getFloat((key + "off").c_str(), 0);
            _prefs.getString((key + "lbl").c_str(), _cache[i].coilLabel, 16);
        }
    }
}

void CalibrationManager::setCalibration(uint8_t ch, const CalibrationData& data) {
    if (ch >= NUM_CHANNELS) return;
    _cache[ch] = data;
    String key = String("cal") + String(ch);
    _prefs.putBool((key + "en").c_str(), data.enabled);
    _prefs.putFloat((key + "off").c_str(), data.offset);
    _prefs.putString((key + "lbl").c_str(), data.coilLabel);
}

CalibrationData CalibrationManager::getCalibration(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return {0, "", false};
    return _cache[ch];
}

float CalibrationManager::getCalibratedTemp(uint8_t ch, float rawTemp) {
    if (ch >= NUM_CHANNELS || !_cache[ch].enabled) return rawTemp;
    return rawTemp + _cache[ch].offset;
}

bool CalibrationManager::isCalibrated(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return false;
    return _cache[ch].enabled;
}

void CalibrationManager::clearCalibration(uint8_t ch) {
    if (ch >= NUM_CHANNELS) return;
    _cache[ch] = {0, "", false};
    String key = String("cal") + String(ch);
    _prefs.remove((key + "en").c_str());
    _prefs.remove((key + "off").c_str());
    _prefs.remove((key + "lbl").c_str());
}
