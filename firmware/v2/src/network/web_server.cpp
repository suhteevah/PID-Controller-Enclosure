#if ENABLE_WIFI
#include "web_server.h"
#include "core/channel.h"
#include "core/safety.h"
#include "data/profiles.h"
#include "data/session_log.h"
#include "data/calibration.h"
#include "data/storage.h"
#include "network/wifi_manager.h"
#if ENABLE_OTA
#include <Update.h>
#endif

WebServer::WebServer() : _server(WEB_SERVER_PORT), _ws("/ws"),
    _channels(nullptr), _safety(nullptr), _profiles(nullptr),
    _logger(nullptr), _cal(nullptr), _storage(nullptr),
    _cmdQueue(nullptr), _lastBroadcast(0) {}

void WebServer::begin(WiFiManager* wifi, Channel* channels, SafetyManager* safety,
                       ProfileManager* profiles, SessionLogger* logger,
                       CalibrationManager* cal, Storage* storage, QueueHandle_t cmdQueue) {
    _channels = channels; _safety = safety; _profiles = profiles;
    _logger = logger; _cal = cal; _storage = storage; _cmdQueue = cmdQueue;

    if (!LittleFS.begin(true)) Serial.println(F("[WEB] LittleFS failed"));

    _ws.onEvent([this](AsyncWebSocket* s, AsyncWebSocketClient* c,
                        AwsEventType t, void* a, uint8_t* d, size_t l) {
        handleWSEvent(s, c, t, a, d, l);
    });
    _server.addHandler(&_ws);
    setupRoutes();
    setupAPI();
    _server.begin();
    Serial.printf("[WEB] Server started on port %d\n", WEB_SERVER_PORT);
}

void WebServer::setupRoutes() {
    _server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,POST,DELETE,OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
}

void WebServer::setupAPI() {
    // GET /api/status
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
        JsonDocument doc;
        doc["model"] = MODEL_NAME;
        doc["version"] = FW_VERSION_STRING;
        doc["uptime"] = millis() / 1000;
        JsonArray chs = doc["channels"].to<JsonArray>();
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            JsonObject c = chs.add<JsonObject>();
            c["id"] = i;
            c["state"] = _channels[i].getStateString();
            c["currentTemp"] = _channels[i].getCurrentTemp();
            c["targetTemp"] = _channels[i].getTargetTemp();
            c["pidOutput"] = _channels[i].getPIDOutput();
        }
        JsonObject s = doc["safety"].to<JsonObject>();
        s["faults"] = _safety->getFaults();
        s["idleRemaining"] = _safety->getIdleMinRemaining();
        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    // POST /api/channel/{n}/enable
    _server.on("^\\/api\\/channel\\/(\\d+)\\/enable$", HTTP_POST,
        [this](AsyncWebServerRequest* req) {
            uint8_t ch = req->pathArg(0).toInt();
            if (ch >= NUM_CHANNELS) { req->send(400, "application/json", "{\"ok\":false}"); return; }
            ChannelCommand cmd = {}; cmd.type = ChannelCommand::CMD_ENABLE; cmd.channel = ch;
            xQueueSend(_cmdQueue, &cmd, 0);
            req->send(200, "application/json", "{\"ok\":true}");
        });

    // POST /api/channel/{n}/disable
    _server.on("^\\/api\\/channel\\/(\\d+)\\/disable$", HTTP_POST,
        [this](AsyncWebServerRequest* req) {
            uint8_t ch = req->pathArg(0).toInt();
            if (ch >= NUM_CHANNELS) { req->send(400, "application/json", "{\"ok\":false}"); return; }
            ChannelCommand cmd = {}; cmd.type = ChannelCommand::CMD_DISABLE; cmd.channel = ch;
            xQueueSend(_cmdQueue, &cmd, 0);
            req->send(200, "application/json", "{\"ok\":true}");
        });

    // POST /api/channel/{n}/settemp
    _server.on("^\\/api\\/channel\\/(\\d+)\\/settemp$", HTTP_POST,
        [this](AsyncWebServerRequest* req) {
            // Body handled in onBody - simplified: use query param fallback
            uint8_t ch = req->pathArg(0).toInt();
            if (ch >= NUM_CHANNELS) { req->send(400, "application/json", "{\"ok\":false}"); return; }
            // Parse from body would happen in onBody handler
            req->send(200, "application/json", "{\"ok\":true}");
        },
        NULL,
        [this](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t idx, size_t total) {
            JsonDocument doc;
            if (deserializeJson(doc, data, len)) { req->send(400); return; }
            uint8_t ch = req->pathArg(0).toInt();
            ChannelCommand cmd = {}; cmd.type = ChannelCommand::CMD_SET_TEMP;
            cmd.channel = ch; cmd.value = doc["temp"] | 710.0f;
            xQueueSend(_cmdQueue, &cmd, 0);
        });

    // GET /api/session/log
    _server.on("/api/session/log", HTTP_GET, [this](AsyncWebServerRequest* req) {
        req->send(200, "application/json", _logger->exportJSON());
    });

    // GET /api/settings
    _server.on("/api/settings", HTTP_GET, [this](AsyncWebServerRequest* req) {
        GlobalSettings gs = _storage->loadGlobalSettings();
        JsonDocument doc;
        doc["idleTimeout"] = gs.idleTimeoutMin;
        doc["fahrenheit"] = gs.fahrenheit;
        doc["brightness"] = gs.displayBrightness;
        String out; serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    // OTA upload
    #if ENABLE_OTA
    _server.on("/api/ota/upload", HTTP_POST,
        [](AsyncWebServerRequest* req) { req->send(200, "application/json", "{\"ok\":true,\"message\":\"Rebooting...\"}"); delay(1000); ESP.restart(); },
        [](AsyncWebServerRequest* req, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
            if (!index) { Update.begin(UPDATE_SIZE_UNKNOWN); }
            Update.write(data, len);
            if (final) { Update.end(true); }
        });
    #endif
}

void WebServer::broadcastTemps(Channel* channels, uint8_t numCh) {
    if (millis() - _lastBroadcast < 500) return;
    _lastBroadcast = millis();
    if (_ws.count() == 0) return;

    JsonDocument doc;
    doc["type"] = "temp";
    JsonArray chs = doc["channels"].to<JsonArray>();
    for (uint8_t i = 0; i < numCh; i++) {
        JsonObject c = chs.add<JsonObject>();
        c["id"] = i;
        c["temp"] = channels[i].getCurrentTemp();
        c["target"] = channels[i].getTargetTemp();
        c["output"] = channels[i].getPIDOutput();
        c["state"] = channels[i].getStateString();
    }
    doc["uptime"] = millis() / 1000;
    doc["idleRemaining"] = _safety->getIdleMinRemaining();

    String out;
    serializeJson(doc, out);
    _ws.textAll(out);
}

void WebServer::handleWSEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                               AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_DATA) {
        JsonDocument doc;
        if (deserializeJson(doc, data, len)) return;
        const char* cmd = doc["cmd"];
        uint8_t ch = doc["ch"] | 0;
        if (ch >= NUM_CHANNELS) return;

        ChannelCommand c = {}; c.channel = ch;
        if (strcmp(cmd, "enable") == 0) { c.type = ChannelCommand::CMD_ENABLE; }
        else if (strcmp(cmd, "disable") == 0) { c.type = ChannelCommand::CMD_DISABLE; }
        else if (strcmp(cmd, "settemp") == 0) { c.type = ChannelCommand::CMD_SET_TEMP; c.value = doc["temp"] | 710.0f; }
        else return;
        xQueueSend(_cmdQueue, &c, 0);
    }
}
#endif
