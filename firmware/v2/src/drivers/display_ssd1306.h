#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Abstract display interface for swappable display drivers
class DisplayDriver {
public:
    virtual ~DisplayDriver() {}
    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void display() = 0;
    virtual void setTextSize(uint8_t size) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char* text) = 0;
    virtual void printf(const char* fmt, ...) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
    virtual void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) = 0;
    virtual void setInvertText(bool invert) = 0;
    virtual uint16_t width() = 0;
    virtual uint16_t height() = 0;
};

// SSD1306 128x64 OLED implementation
class DisplaySSD1306 : public DisplayDriver {
public:
    DisplaySSD1306();

    void begin() override;
    void clear() override;
    void display() override;
    void setTextSize(uint8_t size) override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    void printf(const char* fmt, ...) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h) override;
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) override;
    void setInvertText(bool invert) override;
    uint16_t width() override   { return OLED_WIDTH; }
    uint16_t height() override  { return OLED_HEIGHT; }

    Adafruit_SSD1306& getRaw() { return _oled; }

private:
    Adafruit_SSD1306 _oled;
    char _printBuf[64];
};
