#include "storage.h"

Storage::Storage() {}

void Storage::begin() {
    _prefs.begin(NVS_NAMESPACE, false);
}

void Storage::saveChannelSettings(uint8_t channel, const ChannelSettings& settings) {
    _prefs.putFloat(channelKey(channel, "temp").c_str(), settings.targetTempF);
    _prefs.putFloat(channelKey(channel, "kp").c_str(),   settings.kp);
    _prefs.putFloat(channelKey(channel, "ki").c_str(),   settings.ki);
    _prefs.putFloat(channelKey(channel, "kd").c_str(),   settings.kd);
}

ChannelSettings Storage::loadChannelSettings(uint8_t channel) {
    ChannelSettings s;
    s.targetTempF = _prefs.getFloat(channelKey(channel, "temp").c_str(), TEMP_DEFAULT_F);
    s.kp          = _prefs.getFloat(channelKey(channel, "kp").c_str(),   PID_KP_DEFAULT);
    s.ki          = _prefs.getFloat(channelKey(channel, "ki").c_str(),   PID_KI_DEFAULT);
    s.kd          = _prefs.getFloat(channelKey(channel, "kd").c_str(),   PID_KD_DEFAULT);
    return s;
}

void Storage::saveGlobalSettings(const GlobalSettings& settings) {
    _prefs.putUInt("idleTimeout", settings.idleTimeoutMin);
    _prefs.putUChar("brightness", settings.displayBrightness);
    _prefs.putBool("fahrenheit",  settings.fahrenheit);
    _prefs.putBool("autoEnable",  settings.startupAutoEnable);
}

GlobalSettings Storage::loadGlobalSettings() {
    GlobalSettings s;
    s.idleTimeoutMin    = _prefs.getUInt("idleTimeout",  IDLE_TIMEOUT_MIN);
    s.displayBrightness = _prefs.getUChar("brightness",  255);
    s.fahrenheit        = _prefs.getBool("fahrenheit",    true);
    s.startupAutoEnable = _prefs.getBool("autoEnable",    false);
    return s;
}

void Storage::factoryReset() {
    _prefs.clear();
}

String Storage::channelKey(uint8_t ch, const char* param) {
    return String("ch") + String(ch) + String(param);
}
