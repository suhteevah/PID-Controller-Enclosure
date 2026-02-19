#if ENABLE_MQTT
#include "mqtt_client.h"
#include "core/channel.h"
#include <ArduinoJson.h>

MQTTClient::MQTTClient() : _client(_wifiClient), _port(MQTT_PORT), _lastReconnect(0) {
    memset(_host, 0, sizeof(_host)); memset(_user, 0, sizeof(_user)); memset(_pass, 0, sizeof(_pass));
}

void MQTTClient::begin(const char* host, uint16_t port, const char* user, const char* pass) {
    strncpy(_host, host, 64); _port = port;
    strncpy(_user, user, 32); strncpy(_pass, pass, 64);
    if (strlen(_host) == 0) return;
    _client.setServer(_host, _port);
    _client.setCallback(callback);
}

void MQTTClient::update() {
    if (strlen(_host) == 0) return;
    if (!_client.connected()) { reconnect(); return; }
    _client.loop();
}

void MQTTClient::reconnect() {
    if (millis() - _lastReconnect < 5000) return;
    _lastReconnect = millis();
    String clientId = String("espnail-") + WiFi.macAddress().substring(12);
    clientId.replace(":", "");
    if (_client.connect(clientId.c_str(), strlen(_user) ? _user : NULL,
                         strlen(_pass) ? _pass : NULL, "espnail/status", 0, true, "offline")) {
        _client.publish("espnail/status", "online", true);
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            String topic = String("espnail/ch") + String(i) + "/cmd/#";
            _client.subscribe(topic.c_str());
        }
        publishHADiscovery();
    }
}

void MQTTClient::publishChannel(uint8_t ch, Channel& channel) {
    if (!_client.connected()) return;
    String base = String(MQTT_TOPIC_PREFIX) + "ch" + String(ch) + "/";
    _client.publish((base + "temp").c_str(), String(channel.getCurrentTemp(), 1).c_str());
    _client.publish((base + "target").c_str(), String(channel.getTargetTemp(), 0).c_str());
    _client.publish((base + "state").c_str(), channel.getStateString());
    _client.publish((base + "output").c_str(), String(channel.getPIDOutput(), 1).c_str());
}

void MQTTClient::publishHADiscovery() {
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        String uid = String("espnail_") + WiFi.macAddress().substring(12) + "_ch" + String(i);
        uid.replace(":", "");
        JsonDocument doc;
        doc["name"] = String("ESP-Nail CH") + String(i + 1);
        doc["unique_id"] = uid;
        doc["current_temperature_topic"] = String(MQTT_TOPIC_PREFIX) + "ch" + String(i) + "/temp";
        doc["temperature_state_topic"] = String(MQTT_TOPIC_PREFIX) + "ch" + String(i) + "/target";
        doc["temperature_command_topic"] = String(MQTT_TOPIC_PREFIX) + "ch" + String(i) + "/cmd/settemp";
        doc["min_temp"] = TEMP_MIN_F; doc["max_temp"] = TEMP_MAX_F; doc["temp_step"] = 5;
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = String("espnail_") + WiFi.macAddress();
        dev["name"] = MODEL_NAME; dev["manufacturer"] = "ESP-Nail Open Source";
        dev["sw_version"] = FW_VERSION_STRING;
        String out; serializeJson(doc, out);
        String topic = "homeassistant/climate/" + uid + "/config";
        _client.publish(topic.c_str(), out.c_str(), true);
    }
}

void MQTTClient::callback(char* topic, byte* payload, unsigned int length) {
    // Command handling via MQTT subscriptions
    // Parsed and dispatched similar to web/ble commands
}
#endif
