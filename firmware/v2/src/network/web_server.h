#pragma once
#if ENABLE_WIFI
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"

class Channel;
class SafetyManager;
class ProfileManager;
class SessionLogger;
class CalibrationManager;
class Storage;
class WiFiManager;

class WebServer {
public:
    WebServer();
    void begin(WiFiManager* wifi, Channel* channels, SafetyManager* safety,
               ProfileManager* profiles, SessionLogger* logger,
               CalibrationManager* cal, Storage* storage, QueueHandle_t cmdQueue);
    void broadcastTemps(Channel* channels, uint8_t numCh);
private:
    AsyncWebServer _server;
    AsyncWebSocket _ws;
    Channel* _channels;
    SafetyManager* _safety;
    ProfileManager* _profiles;
    SessionLogger* _logger;
    CalibrationManager* _cal;
    Storage* _storage;
    QueueHandle_t _cmdQueue;
    uint32_t _lastBroadcast;
    void setupRoutes();
    void setupAPI();
    void handleWSEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                       AwsEventType type, void* arg, uint8_t* data, size_t len);
};
#endif
