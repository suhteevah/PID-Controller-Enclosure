#pragma once
#if ENABLE_WIFI
#include <Arduino.h>
#include <ESPmDNS.h>
#include "config.h"
class MDNSService {
public:
    MDNSService();
    void begin();
    void update();
private:
    bool _started;
};
#endif
