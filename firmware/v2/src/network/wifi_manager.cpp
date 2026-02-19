#include "wifi_manager.h"

WiFiManager::WiFiManager()
    : _mode(WiFiMode_t_App::NONE),
      _hasCredentials(false),
      _wasConnected(false),
      _lastReconnectAttempt(0),
      _reconnectIntervalMs(RECONNECT_INITIAL_MS),
      _reconnectCount(0) {}

void WiFiManager::begin() {
    computeMACSuffix();
    loadCredentials();

    if (_hasCredentials) {
        // Try stored STA credentials first
        if (!connectSTA(_staSSID.c_str(), _staPassword.c_str())) {
            // If connection fails, fall back to AP mode
            startAP();
        }
    } else {
        // No stored credentials, start in AP mode
        startAP();
    }
}

void WiFiManager::update() {
    if (_mode == WiFiMode_t_App::STA) {
        bool connected = (WiFi.status() == WL_CONNECTED);

        if (_wasConnected && !connected) {
            // Lost connection
            Serial.println("[WiFi] Connection lost, will attempt reconnect");
            _wasConnected = false;
            _lastReconnectAttempt = millis();
            _reconnectIntervalMs = RECONNECT_INITIAL_MS;
            _reconnectCount = 0;
        }

        if (!connected) {
            uint32_t now = millis();
            if ((now - _lastReconnectAttempt) >= _reconnectIntervalMs) {
                _lastReconnectAttempt = now;
                _reconnectCount++;

                Serial.printf("[WiFi] Reconnect attempt %u...\n", _reconnectCount);
                WiFi.disconnect();
                WiFi.begin(_staSSID.c_str(), _staPassword.c_str());

                // Exponential backoff
                _reconnectIntervalMs = min(_reconnectIntervalMs * 2, RECONNECT_MAX_MS);

                // After too many failures, fall back to AP mode
                if (_reconnectCount >= MAX_RECONNECT_BEFORE_AP) {
                    Serial.println("[WiFi] Too many reconnect failures, starting AP");
                    startAP();
                }
            }
        } else if (!_wasConnected) {
            // Just connected
            _wasConnected = true;
            _reconnectCount = 0;
            _reconnectIntervalMs = RECONNECT_INITIAL_MS;
            Serial.print("[WiFi] Connected, IP: ");
            Serial.println(WiFi.localIP());
        }
    }
}

void WiFiManager::startAP() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    String apSSID = String(WIFI_AP_SSID_PREFIX) + _macSuffix;

    WiFi.softAP(apSSID.c_str(), WIFI_AP_PASSWORD);
    _mode = WiFiMode_t_App::AP;

    Serial.printf("[WiFi] AP started: %s (pass: %s)\n", apSSID.c_str(), WIFI_AP_PASSWORD);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
}

bool WiFiManager::connectSTA(const char* ssid, const char* password) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);  // We handle reconnection ourselves

    _staSSID = String(ssid);
    _staPassword = String(password);
    _mode = WiFiMode_t_App::STA;
    _wasConnected = false;

    Serial.printf("[WiFi] Connecting to %s...\n", ssid);
    WiFi.begin(ssid, password);

    // Block for connection with timeout
    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if ((millis() - startMs) >= (WIFI_CONNECT_TIMEOUT_S * 1000UL)) {
            Serial.println("[WiFi] Connection timeout");
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    _wasConnected = true;
    _reconnectCount = 0;
    _reconnectIntervalMs = RECONNECT_INITIAL_MS;

    Serial.print("[WiFi] Connected, IP: ");
    Serial.println(WiFi.localIP());

    // Save credentials on successful connection
    saveCredentials(ssid, password);

    return true;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    _mode = WiFiMode_t_App::NONE;
    _wasConnected = false;
}

bool WiFiManager::isConnected() const {
    if (_mode == WiFiMode_t_App::STA) {
        return WiFi.status() == WL_CONNECTED;
    }
    if (_mode == WiFiMode_t_App::AP) {
        return true;  // AP is always "connected"
    }
    return false;
}

IPAddress WiFiManager::getIP() const {
    if (_mode == WiFiMode_t_App::STA) {
        return WiFi.localIP();
    }
    if (_mode == WiFiMode_t_App::AP) {
        return WiFi.softAPIP();
    }
    return IPAddress(0, 0, 0, 0);
}

String WiFiManager::getSSID() const {
    if (_mode == WiFiMode_t_App::STA) {
        return WiFi.SSID();
    }
    if (_mode == WiFiMode_t_App::AP) {
        return String(WIFI_AP_SSID_PREFIX) + _macSuffix;
    }
    return String();
}

int32_t WiFiManager::getRSSI() const {
    if (_mode == WiFiMode_t_App::STA && WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

uint8_t WiFiManager::getConnectedClients() const {
    if (_mode == WiFiMode_t_App::AP) {
        return WiFi.softAPgetStationNum();
    }
    return 0;
}

String WiFiManager::getMACSuffix() const {
    return _macSuffix;
}

void WiFiManager::loadCredentials() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);  // read-only

    _staSSID = prefs.getString("wifi_ssid", "");
    _staPassword = prefs.getString("wifi_pass", "");
    _hasCredentials = (_staSSID.length() > 0);

    prefs.end();

    if (_hasCredentials) {
        Serial.printf("[WiFi] Loaded credentials for SSID: %s\n", _staSSID.c_str());
    }
}

void WiFiManager::saveCredentials(const char* ssid, const char* password) {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);

    prefs.putString("wifi_ssid", ssid);
    prefs.putString("wifi_pass", password);

    prefs.end();

    _hasCredentials = true;
    Serial.printf("[WiFi] Credentials saved for SSID: %s\n", ssid);
}

void WiFiManager::clearStoredCredentials() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);

    prefs.remove("wifi_ssid");
    prefs.remove("wifi_pass");

    prefs.end();

    _staSSID = "";
    _staPassword = "";
    _hasCredentials = false;

    Serial.println("[WiFi] Stored credentials cleared");
}

void WiFiManager::computeMACSuffix() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char buf[5];
    snprintf(buf, sizeof(buf), "%02X%02X", mac[4], mac[5]);
    _macSuffix = String(buf);
}
