#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "channel.h"
#include "safety.h"
#include "ui.h"

class Display {
public:
    Display();

    void begin();
    void update(Channel channels[], uint8_t numChannels,
                const SafetyManager& safety, const UIManager& ui);

private:
    Adafruit_SSD1306 _oled;
    uint32_t _lastUpdateTime;

    // Screen renderers
    void drawMainScreen(Channel channels[], uint8_t numChannels,
                        const SafetyManager& safety, const UIManager& ui);
    void drawSingleChannel(Channel& ch, const SafetyManager& safety);
    void drawMultiChannel(Channel channels[], uint8_t numChannels,
                          const SafetyManager& safety, uint8_t selectedCh);
    void drawSetTempScreen(Channel& ch, bool fineAdjust);
    void drawSettingsMenu(uint8_t menuIndex, const SafetyManager& safety);
    void drawPIDTuneScreen(Channel& ch, uint8_t menuIndex);
    void drawIdleTimeoutScreen(const SafetyManager& safety);
    void drawInfoScreen();
    void drawFaultScreen(Channel channels[], uint8_t numChannels,
                         const SafetyManager& safety);
    void drawChannelSelectScreen(Channel channels[], uint8_t numChannels,
                                 uint8_t selectedCh);

    // Helpers
    void drawTempBar(int16_t x, int16_t y, int16_t w, int16_t h,
                     float current, float target);
    void drawStatusIcon(int16_t x, int16_t y, ChannelState state);
};
