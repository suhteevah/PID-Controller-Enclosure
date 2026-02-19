#include "display_ssd1306.h"
#include <cstdarg>

DisplaySSD1306::DisplaySSD1306()
    : _oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {}

void DisplaySSD1306::begin() {
    Wire.begin(PIN_SDA, PIN_SCL);
    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("[DISPLAY] SSD1306 init failed"));
        return;
    }
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
    _oled.display();
}

void DisplaySSD1306::clear() {
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
}

void DisplaySSD1306::display() {
    _oled.display();
}

void DisplaySSD1306::setTextSize(uint8_t size) {
    _oled.setTextSize(size);
}

void DisplaySSD1306::setCursor(int16_t x, int16_t y) {
    _oled.setCursor(x, y);
}

void DisplaySSD1306::print(const char* text) {
    _oled.print(text);
}

void DisplaySSD1306::printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(_printBuf, sizeof(_printBuf), fmt, args);
    va_end(args);
    _oled.print(_printBuf);
}

void DisplaySSD1306::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    _oled.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
}

void DisplaySSD1306::drawRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    _oled.drawRect(x, y, w, h, SSD1306_WHITE);
}

void DisplaySSD1306::fillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    _oled.fillRect(x, y, w, h, SSD1306_WHITE);
}

void DisplaySSD1306::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    _oled.fillTriangle(x0, y0, x1, y1, x2, y2, SSD1306_WHITE);
}

void DisplaySSD1306::setInvertText(bool invert) {
    if (invert) {
        _oled.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
        _oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
}
