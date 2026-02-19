#pragma once

// ============================================================
// ESP-Nail v2 - Versioned NVS Storage Manager
// Persistent settings with version migration and validation
// ============================================================

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// --- Per-Channel Settings ---
struct ChannelSettings {
    float targetTempF;
    float kp;
    float ki;
    float kd;
    uint8_t activeProfileIndex;
};

// --- Global Settings ---
struct GlobalSettings {
    uint32_t idleTimeoutMin;
    uint8_t displayBrightness;
    bool fahrenheit;
    bool startupAutoEnable;
    uint8_t wifiMode;           // 0 = AP, 1 = STA
    char wifiSSID[33];
    char wifiPass[65];
    char mqttHost[65];
    uint16_t mqttPort;
    char mqttUser[33];
    char mqttPass[65];
};

class StorageManager {
public:
    StorageManager();

    /// Initialize NVS, check settings version, load or reset as needed
    bool begin();

    /// Channel settings persistence
    bool saveChannelSettings(uint8_t ch, const ChannelSettings& settings);
    ChannelSettings loadChannelSettings(uint8_t ch);

    /// Global settings persistence
    bool saveGlobalSettings(const GlobalSettings& settings);
    GlobalSettings loadGlobalSettings();

    /// Reset all stored settings to factory defaults
    void factoryReset();

    /// Get stored settings version
    uint8_t getSettingsVersion();

private:
    Preferences _prefs;

    /// Build NVS key string for a channel-specific value
    String channelKey(uint8_t ch, const char* suffix);

    /// Create default channel settings
    ChannelSettings defaultChannelSettings();

    /// Create default global settings
    GlobalSettings defaultGlobalSettings();

    /// Validate and clamp channel settings to sane ranges
    void validateChannelSettings(ChannelSettings& s);

    /// Validate and clamp global settings to sane ranges
    void validateGlobalSettings(GlobalSettings& s);

    /// Write defaults for all channels and globals
    void writeAllDefaults();
};
