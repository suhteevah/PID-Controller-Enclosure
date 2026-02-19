#pragma once
#if ENABLE_OTA
#include <Arduino.h>
#include <Update.h>
#include "config.h"

class OTAUpdater {
public:
    OTAUpdater();
    void begin();
    bool handleChunk(uint8_t* data, size_t len, size_t index, size_t total, bool final);
    float getProgress() const { return _progress; }
    bool isUpdating() const { return _updating; }
    bool hasError() const { return _error; }
    String getErrorMessage() const { return _errorMsg; }
private:
    float _progress;
    bool _updating;
    bool _error;
    String _errorMsg;
};
#endif
