#include "storage.h"

// ============================================================
// ESP-Nail v2 - Versioned NVS Storage Manager
// ============================================================

StorageManager::StorageManager() {}

bool StorageManager::begin() {
    bool ok = _prefs.begin(NVS_NAMESPACE, false);
    if (!ok) {
        Serial.println("[Storage] ERROR: NVS begin failed");
        return false;
    }

    uint8_t storedVersion = _prefs.getUChar("version", 0);
    if (storedVersion != NVS_SETTINGS_VERSION) {
        Serial.printf("[Storage] Version mismatch (stored=%d, expected=%d). Resetting to defaults.\n",
                      storedVersion, NVS_SETTINGS_VERSION);
        writeAllDefaults();
        _prefs.putUChar("version", NVS_SETTINGS_VERSION);
    }

    _prefs.end();
    Serial.println("[Storage] Initialized OK");
    return true;
}

// --- Channel Settings ---

String StorageManager::channelKey(uint8_t ch, const char* suffix) {
    char buf[24];
    snprintf(buf, sizeof(buf), "ch%u_%s", ch, suffix);
    return String(buf);
}

ChannelSettings StorageManager::defaultChannelSettings() {
    ChannelSettings s;
    s.targetTempF = TEMP_DEFAULT_F;
    s.kp = PID_KP_DEFAULT;
    s.ki = PID_KI_DEFAULT;
    s.kd = PID_KD_DEFAULT;
    s.activeProfileIndex = 2;   // "Standard" profile
    return s;
}

void StorageManager::validateChannelSettings(ChannelSettings& s) {
    // Target temperature
    if (s.targetTempF < TEMP_MIN_F || s.targetTempF > TEMP_MAX_F || isnan(s.targetTempF)) {
        s.targetTempF = TEMP_DEFAULT_F;
    }
    // PID gains: must be non-negative and within practical bounds
    if (s.kp < 0.0f || s.kp > 100.0f || isnan(s.kp)) s.kp = PID_KP_DEFAULT;
    if (s.ki < 0.0f || s.ki > 50.0f  || isnan(s.ki)) s.ki = PID_KI_DEFAULT;
    if (s.kd < 0.0f || s.kd > 100.0f || isnan(s.kd)) s.kd = PID_KD_DEFAULT;
    // Active profile index
    if (s.activeProfileIndex >= MAX_PROFILES_PER_CH) {
        s.activeProfileIndex = 0;
    }
}

bool StorageManager::saveChannelSettings(uint8_t ch, const ChannelSettings& settings) {
    if (ch >= NUM_CHANNELS) return false;

    ChannelSettings s = settings;
    validateChannelSettings(s);

    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putFloat(channelKey(ch, "temp").c_str(),    s.targetTempF);
    _prefs.putFloat(channelKey(ch, "kp").c_str(),      s.kp);
    _prefs.putFloat(channelKey(ch, "ki").c_str(),      s.ki);
    _prefs.putFloat(channelKey(ch, "kd").c_str(),      s.kd);
    _prefs.putUChar(channelKey(ch, "profIdx").c_str(), s.activeProfileIndex);
    _prefs.end();

    Serial.printf("[Storage] Saved channel %u settings\n", ch);
    return true;
}

ChannelSettings StorageManager::loadChannelSettings(uint8_t ch) {
    ChannelSettings s = defaultChannelSettings();
    if (ch >= NUM_CHANNELS) return s;

    _prefs.begin(NVS_NAMESPACE, true);
    s.targetTempF       = _prefs.getFloat(channelKey(ch, "temp").c_str(),    s.targetTempF);
    s.kp                = _prefs.getFloat(channelKey(ch, "kp").c_str(),      s.kp);
    s.ki                = _prefs.getFloat(channelKey(ch, "ki").c_str(),      s.ki);
    s.kd                = _prefs.getFloat(channelKey(ch, "kd").c_str(),      s.kd);
    s.activeProfileIndex = _prefs.getUChar(channelKey(ch, "profIdx").c_str(), s.activeProfileIndex);
    _prefs.end();

    validateChannelSettings(s);
    return s;
}

// --- Global Settings ---

GlobalSettings StorageManager::defaultGlobalSettings() {
    GlobalSettings s;
    s.idleTimeoutMin    = IDLE_TIMEOUT_MIN_DEFAULT;
    s.displayBrightness = 255;
    s.fahrenheit        = true;
    s.startupAutoEnable = false;
    s.wifiMode          = 0;   // AP mode default
    memset(s.wifiSSID, 0, sizeof(s.wifiSSID));
    memset(s.wifiPass, 0, sizeof(s.wifiPass));
    memset(s.mqttHost, 0, sizeof(s.mqttHost));
    s.mqttPort          = MQTT_PORT;
    memset(s.mqttUser, 0, sizeof(s.mqttUser));
    memset(s.mqttPass, 0, sizeof(s.mqttPass));
    return s;
}

void StorageManager::validateGlobalSettings(GlobalSettings& s) {
    // Idle timeout: 1..IDLE_TIMEOUT_MIN_MAX minutes, or 0 for disabled
    if (s.idleTimeoutMin > IDLE_TIMEOUT_MIN_MAX) {
        s.idleTimeoutMin = IDLE_TIMEOUT_MIN_DEFAULT;
    }
    // Display brightness: 0-255
    // (uint8_t already constrains this)
    // WiFi mode: only 0 or 1
    if (s.wifiMode > 1) s.wifiMode = 0;
    // Null-terminate all strings to be safe
    s.wifiSSID[sizeof(s.wifiSSID) - 1] = '\0';
    s.wifiPass[sizeof(s.wifiPass) - 1] = '\0';
    s.mqttHost[sizeof(s.mqttHost) - 1] = '\0';
    s.mqttUser[sizeof(s.mqttUser) - 1] = '\0';
    s.mqttPass[sizeof(s.mqttPass) - 1] = '\0';
    // MQTT port
    if (s.mqttPort == 0) s.mqttPort = MQTT_PORT;
}

bool StorageManager::saveGlobalSettings(const GlobalSettings& settings) {
    GlobalSettings s = settings;
    validateGlobalSettings(s);

    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putUInt("idleTimeout",   s.idleTimeoutMin);
    _prefs.putUChar("brightness",   s.displayBrightness);
    _prefs.putBool("fahrenheit",    s.fahrenheit);
    _prefs.putBool("autoEnable",    s.startupAutoEnable);
    _prefs.putUChar("wifiMode",     s.wifiMode);
    _prefs.putString("wifiSSID",    s.wifiSSID);
    _prefs.putString("wifiPass",    s.wifiPass);
    _prefs.putString("mqttHost",    s.mqttHost);
    _prefs.putUShort("mqttPort",    s.mqttPort);
    _prefs.putString("mqttUser",    s.mqttUser);
    _prefs.putString("mqttPass",    s.mqttPass);
    _prefs.end();

    Serial.println("[Storage] Saved global settings");
    return true;
}

GlobalSettings StorageManager::loadGlobalSettings() {
    GlobalSettings s = defaultGlobalSettings();

    _prefs.begin(NVS_NAMESPACE, true);
    s.idleTimeoutMin    = _prefs.getUInt("idleTimeout",   s.idleTimeoutMin);
    s.displayBrightness = _prefs.getUChar("brightness",   s.displayBrightness);
    s.fahrenheit        = _prefs.getBool("fahrenheit",     s.fahrenheit);
    s.startupAutoEnable = _prefs.getBool("autoEnable",     s.startupAutoEnable);
    s.wifiMode          = _prefs.getUChar("wifiMode",      s.wifiMode);

    // Load strings safely
    String ssid = _prefs.getString("wifiSSID", "");
    String pass = _prefs.getString("wifiPass", "");
    String mqttH = _prefs.getString("mqttHost", "");
    String mqttU = _prefs.getString("mqttUser", "");
    String mqttP = _prefs.getString("mqttPass", "");

    strncpy(s.wifiSSID, ssid.c_str(), sizeof(s.wifiSSID) - 1);
    strncpy(s.wifiPass, pass.c_str(), sizeof(s.wifiPass) - 1);
    strncpy(s.mqttHost, mqttH.c_str(), sizeof(s.mqttHost) - 1);
    strncpy(s.mqttUser, mqttU.c_str(), sizeof(s.mqttUser) - 1);
    strncpy(s.mqttPass, mqttP.c_str(), sizeof(s.mqttPass) - 1);

    s.mqttPort = _prefs.getUShort("mqttPort", s.mqttPort);
    _prefs.end();

    validateGlobalSettings(s);
    return s;
}

// --- Factory Reset ---

void StorageManager::factoryReset() {
    Serial.println("[Storage] Factory reset - clearing all settings");
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.clear();
    _prefs.end();
    writeAllDefaults();
    _prefs.begin(NVS_NAMESPACE, false);
    _prefs.putUChar("version", NVS_SETTINGS_VERSION);
    _prefs.end();
    Serial.println("[Storage] Factory reset complete");
}

uint8_t StorageManager::getSettingsVersion() {
    _prefs.begin(NVS_NAMESPACE, true);
    uint8_t v = _prefs.getUChar("version", 0);
    _prefs.end();
    return v;
}

void StorageManager::writeAllDefaults() {
    GlobalSettings gs = defaultGlobalSettings();
    saveGlobalSettings(gs);
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++) {
        ChannelSettings cs = defaultChannelSettings();
        saveChannelSettings(ch, cs);
    }
}
