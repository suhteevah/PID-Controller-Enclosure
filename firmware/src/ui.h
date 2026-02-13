#pragma once

#include <Arduino.h>
#include "config.h"

// UI input events
enum class UIEvent {
    NONE,
    ROTATE_CW,     // Clockwise rotation
    ROTATE_CCW,    // Counter-clockwise rotation
    PRESS,         // Short button press
    LONG_PRESS     // Long button press (>1s)
};

// Menu screens
enum class Screen {
    MAIN,           // Main temperature display
    CHANNEL_SELECT, // Select active channel (multi-channel models)
    SET_TEMP,       // Adjust target temperature
    SETTINGS,       // Settings menu
    PID_TUNE,       // PID tuning submenu
    IDLE_TIMEOUT,   // Idle timeout setting
    INFO,           // System info screen
    FAULT           // Fault display screen
};

class UIManager {
public:
    UIManager();

    void begin();
    UIEvent poll();

    Screen getCurrentScreen() const     { return _currentScreen; }
    void setScreen(Screen screen);

    uint8_t getSelectedChannel() const  { return _selectedChannel; }
    void setSelectedChannel(uint8_t ch) { _selectedChannel = ch; }

    uint8_t getMenuIndex() const        { return _menuIndex; }
    void setMenuIndex(uint8_t idx)      { _menuIndex = idx; }

    bool isFineAdjust() const           { return _fineAdjust; }
    void toggleFineAdjust()             { _fineAdjust = !_fineAdjust; }

private:
    Screen _currentScreen;
    uint8_t _selectedChannel;
    uint8_t _menuIndex;
    bool _fineAdjust;

    // Encoder state
    volatile int32_t _encoderPos;
    int32_t _lastEncoderPos;
    uint32_t _lastEncTime;

    // Button state
    bool _buttonState;
    bool _lastButtonState;
    uint32_t _buttonPressTime;
    bool _buttonHandled;

    static void IRAM_ATTR encoderISR();
    static volatile int32_t _isrEncoderPos;
    static volatile uint32_t _isrLastTime;
};
