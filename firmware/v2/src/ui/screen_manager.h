#pragma once

#include <Arduino.h>
#include "config.h"
#include "drivers/encoder.h"

enum class Screen : uint8_t {
    MAIN,
    CHANNEL_SELECT,
    SET_TEMP,
    PROFILES,
    SETTINGS,
    PID_TUNE,
    IDLE_TIMEOUT,
    WIFI_STATUS,
    INFO,
    AUTOTUNE,
    FAULT
};

// Forward declarations
class Channel;
struct TempUpdate;
class SafetyManager;
class ProfileManager;
class Storage;
class CalibrationManager;
class DisplayDriver;

class ScreenManager {
public:
    ScreenManager();

    void setScreen(Screen s);
    Screen getCurrentScreen() const     { return _current; }

    uint8_t getSelectedChannel() const  { return _selectedCh; }
    void setSelectedChannel(uint8_t ch) { _selectedCh = ch; }

    uint8_t getMenuIndex() const        { return _menuIdx; }
    void setMenuIndex(uint8_t i)        { _menuIdx = i; }

    bool isFineAdjust() const           { return _fineAdj; }
    void toggleFineAdjust()             { _fineAdj = !_fineAdj; }

    // Process encoder events and dispatch to current screen handler
    void handleEvent(EncoderEvent evt, Channel channels[], TempUpdate temps[],
                     QueueHandle_t cmdQueue, SafetyManager& safety,
                     ProfileManager& profiles, Storage& storage,
                     CalibrationManager& calibration);

    // Render current screen
    void render(DisplayDriver* d, Channel channels[], TempUpdate temps[],
                uint8_t numCh, SafetyManager& safety, ProfileManager& profiles);

private:
    Screen _current;
    uint8_t _selectedCh;
    uint8_t _menuIdx;
    bool _fineAdj;
};
