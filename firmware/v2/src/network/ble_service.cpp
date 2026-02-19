#if ENABLE_BLE
#include "ble_service.h"
#include "core/channel.h"

class BLEService::ServerCallbacks : public BLEServerCallbacks {
    BLEService* _parent;
public:
    ServerCallbacks(BLEService* p) : _parent(p) {}
    void onConnect(BLEServer* s) { _parent->_connected = true; }
    void onDisconnect(BLEServer* s) { _parent->_connected = false; s->startAdvertising(); }
};

class BLEService::CmdCallbacks : public BLECharacteristicCallbacks {
    BLEService* _parent;
public:
    CmdCallbacks(BLEService* p) : _parent(p) {}
    void onWrite(BLECharacteristic* c) {
        std::string val = c->getValue();
        JsonDocument doc;
        if (deserializeJson(doc, val.c_str())) return;
        const char* cmd = doc["cmd"];
        uint8_t ch = doc["ch"] | 0;
        if (ch >= NUM_CHANNELS) return;
        ChannelCommand cc = {}; cc.channel = ch;
        if (strcmp(cmd, "enable") == 0) cc.type = ChannelCommand::CMD_ENABLE;
        else if (strcmp(cmd, "disable") == 0) cc.type = ChannelCommand::CMD_DISABLE;
        else if (strcmp(cmd, "settemp") == 0) { cc.type = ChannelCommand::CMD_SET_TEMP; cc.value = doc["temp"] | 710.0f; }
        else return;
        xQueueSend(_parent->_cmdQueue, &cc, 0);
    }
};

BLEService::BLEService() : _server(nullptr), _tempChar(nullptr), _cmdChar(nullptr),
    _stateChar(nullptr), _channels(nullptr), _cmdQueue(nullptr), _lastNotify(0), _connected(false) {}

void BLEService::begin(Channel* channels, QueueHandle_t cmdQueue) {
    _channels = channels; _cmdQueue = cmdQueue;
    String name = String(BLE_DEVICE_NAME_PREFIX) + WiFi.macAddress().substring(12);
    name.replace(":", "");
    BLEDevice::init(name.c_str());
    _server = BLEDevice::createServer();
    _server->setCallbacks(new ServerCallbacks(this));
    ::BLEService* svc = _server->createService(BLE_SERVICE_UUID);
    _tempChar = svc->createCharacteristic(BLE_CHAR_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _tempChar->addDescriptor(new BLE2902());
    _cmdChar = svc->createCharacteristic(BLE_CHAR_CMD_UUID, BLECharacteristic::PROPERTY_WRITE);
    _cmdChar->setCallbacks(new CmdCallbacks(this));
    _stateChar = svc->createCharacteristic(BLE_CHAR_STATE_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    _stateChar->addDescriptor(new BLE2902());
    svc->start();
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->start();
    Serial.println(F("[BLE] Service started"));
}

void BLEService::update(Channel* channels, uint8_t numCh) {
    if (!_connected || (millis() - _lastNotify) < 2000) return;
    _lastNotify = millis();
    JsonDocument doc;
    JsonArray arr = doc["ch"].to<JsonArray>();
    for (uint8_t i = 0; i < numCh; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["t"] = channels[i].getCurrentTemp();
        o["s"] = channels[i].getTargetTemp();
        o["o"] = channels[i].getPIDOutput();
    }
    String out; serializeJson(doc, out);
    _tempChar->setValue(out.c_str());
    _tempChar->notify();
}
#endif
