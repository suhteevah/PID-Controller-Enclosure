#pragma once
#if ENABLE_BLE
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "config.h"

class Channel;

class BLEService {
public:
    BLEService();
    void begin(Channel* channels, QueueHandle_t cmdQueue);
    void update(Channel* channels, uint8_t numCh);
private:
    BLEServer* _server;
    BLECharacteristic* _tempChar;
    BLECharacteristic* _cmdChar;
    BLECharacteristic* _stateChar;
    Channel* _channels;
    QueueHandle_t _cmdQueue;
    uint32_t _lastNotify;
    bool _connected;
    class ServerCallbacks;
    class CmdCallbacks;
};
#endif
