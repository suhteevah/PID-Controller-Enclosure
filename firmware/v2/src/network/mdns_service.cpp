#if ENABLE_WIFI
#include "mdns_service.h"

MDNSService::MDNSService() : _started(false) {}

void MDNSService::begin() {
    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
        _started = true;
        Serial.printf("[MDNS] http://%s.local\n", MDNS_HOSTNAME);
    }
}

void MDNSService::update() { /* mDNS handled by ESP32 stack */ }
#endif
