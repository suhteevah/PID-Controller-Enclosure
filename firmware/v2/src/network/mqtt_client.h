#pragma once
#if ENABLE_MQTT
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"
class Channel;
class MQTTClient {
public:
    MQTTClient();
    void begin(const char* host, uint16_t port, const char* user, const char* pass);
    void update();
    void publishChannel(uint8_t ch, Channel& channel);
    bool isConnected() { return _client.connected(); }
private:
    WiFiClient _wifiClient;
    PubSubClient _client;
    char _host[65]; uint16_t _port; char _user[33]; char _pass[65];
    uint32_t _lastReconnect;
    void reconnect();
    void publishHADiscovery();
    static void callback(char* topic, byte* payload, unsigned int length);
};
#endif
