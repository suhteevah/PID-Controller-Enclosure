#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// Persistent settings stored in ESP32 NVS flash
struct ChannelSettings {
    float targetTempF;
    float kp;
    float ki;
    float kd;
};

struct GlobalSettings {
    uint32_t idleTimeoutMin;
    uint8_t displayBrightness;
    bool fahrenheit;            // true = F, false = C
    bool startupAutoEnable;     // Auto-enable channels on boot
};

class Storage {
public:
    Storage();

    void begin();

    // Channel settings
    void saveChannelSettings(uint8_t channel, const ChannelSettings& settings);
    ChannelSettings loadChannelSettings(uint8_t channel);

    // Global settings
    void saveGlobalSettings(const GlobalSettings& settings);
    GlobalSettings loadGlobalSettings();

    // Factory reset
    void factoryReset();

private:
    Preferences _prefs;

    // Generate key names per channel
    String channelKey(uint8_t ch, const char* param);
};
