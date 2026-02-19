#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "config.h"

enum class WiFiMode_t_App {
    NONE,
    AP,
    STA
};

class WiFiManager {
public:
    WiFiManager();

    void begin();
    void update();

    // Mode control
    void startAP();
    bool connectSTA(const char* ssid, const char* password);
    void disconnect();

    // Status
    bool isConnected() const;
    WiFiMode_t_App getMode() const         { return _mode; }
    IPAddress getIP() const;
    String getSSID() const;
    int32_t getRSSI() const;
    uint8_t getConnectedClients() const;
    String getMACSuffix() const;

    // Credential management
    bool hasStoredCredentials() const      { return _hasCredentials; }
    void clearStoredCredentials();

private:
    WiFiMode_t_App _mode;
    bool _hasCredentials;
    bool _wasConnected;
    uint32_t _lastReconnectAttempt;
    uint32_t _reconnectIntervalMs;
    uint8_t _reconnectCount;

    String _staSSID;
    String _staPassword;
    String _macSuffix;

    static const uint32_t RECONNECT_INITIAL_MS = 5000;
    static const uint32_t RECONNECT_MAX_MS = 60000;
    static const uint8_t  MAX_RECONNECT_BEFORE_AP = 5;

    void loadCredentials();
    void saveCredentials(const char* ssid, const char* password);
    void computeMACSuffix();
};
