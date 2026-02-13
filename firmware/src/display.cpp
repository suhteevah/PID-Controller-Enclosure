#include "display.h"

Display::Display()
    : _oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1),
      _lastUpdateTime(0) {}

void Display::begin() {
    Wire.begin(PIN_SDA, PIN_SCL);

    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("OLED init failed"));
        return;
    }

    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);

    // Splash screen
    _oled.setTextSize(2);
    _oled.setCursor(16, 8);
    _oled.print(F("ESP-Nail"));
    _oled.setTextSize(1);
    _oled.setCursor(28, 32);
    _oled.print(MODEL_NAME);
    _oled.setCursor(36, 48);
    _oled.print(F("v" FW_VERSION));
    _oled.display();
    delay(1500);
}

void Display::update(Channel channels[], uint8_t numChannels,
                     const SafetyManager& safety, const UIManager& ui) {
    uint32_t now = millis();
    if (now - _lastUpdateTime < DISPLAY_UPDATE_MS) return;
    _lastUpdateTime = now;

    _oled.clearDisplay();

    Screen screen = ui.getCurrentScreen();
    uint8_t selCh = ui.getSelectedChannel();

    switch (screen) {
        case Screen::MAIN:
            drawMainScreen(channels, numChannels, safety, ui);
            break;
        case Screen::CHANNEL_SELECT:
            drawChannelSelectScreen(channels, numChannels, selCh);
            break;
        case Screen::SET_TEMP:
            drawSetTempScreen(channels[selCh], ui.isFineAdjust());
            break;
        case Screen::SETTINGS:
            drawSettingsMenu(ui.getMenuIndex(), safety);
            break;
        case Screen::PID_TUNE:
            drawPIDTuneScreen(channels[selCh], ui.getMenuIndex());
            break;
        case Screen::IDLE_TIMEOUT:
            drawIdleTimeoutScreen(safety);
            break;
        case Screen::INFO:
            drawInfoScreen();
            break;
        case Screen::FAULT:
            drawFaultScreen(channels, numChannels, safety);
            break;
    }

    _oled.display();
}

void Display::drawMainScreen(Channel channels[], uint8_t numChannels,
                              const SafetyManager& safety, const UIManager& ui) {
    if (numChannels == 1) {
        drawSingleChannel(channels[0], safety);
    } else {
        drawMultiChannel(channels, numChannels, safety, ui.getSelectedChannel());
    }
}

void Display::drawSingleChannel(Channel& ch, const SafetyManager& safety) {
    // Large temperature display for single channel

    // Status bar at top
    _oled.setTextSize(1);
    _oled.setCursor(0, 0);
    _oled.print(ch.getStateString());

    // Idle timer on right
    uint32_t idleMin = safety.getIdleMinRemaining();
    if (idleMin > 0 && ch.isActive()) {
        _oled.setCursor(88, 0);
        _oled.printf("%um", idleMin);
    }

    // PID output percentage
    if (ch.isActive()) {
        _oled.setCursor(50, 0);
        _oled.printf("%3.0f%%", ch.getPIDOutput());
    }

    // Horizontal separator
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // Current temperature - large font
    _oled.setTextSize(3);
    float currentTemp = ch.getCurrentTemp();
    if (ch.getTCStatus() == TCStatus::OK) {
        _oled.setCursor(4, 16);
        _oled.printf("%5.1f", currentTemp);
        _oled.setTextSize(1);
        _oled.setCursor(112, 16);
        _oled.print(F("F"));
    } else {
        _oled.setCursor(16, 16);
        _oled.print(F("---.-"));
        _oled.setTextSize(1);
        _oled.setCursor(0, 44);
        _oled.print(F("TC: "));
        _oled.print(ch.getTC().getStatusString());
    }

    // Target temperature
    _oled.setTextSize(1);
    _oled.setCursor(0, 44);
    _oled.print(F("SET: "));
    _oled.printf("%.0fF", ch.getTargetTemp());

    // Temperature bar
    drawTempBar(0, 56, 128, 8, currentTemp, ch.getTargetTemp());
}

void Display::drawMultiChannel(Channel channels[], uint8_t numChannels,
                                const SafetyManager& safety, uint8_t selectedCh) {
    // Compact multi-channel view
    _oled.setTextSize(1);

    // Header
    _oled.setCursor(0, 0);
    _oled.print(MODEL_NAME);

    uint32_t idleMin = safety.getIdleMinRemaining();
    if (idleMin > 0) {
        _oled.setCursor(88, 0);
        _oled.printf("%um", idleMin);
    }

    _oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    // Each channel gets a row
    uint8_t rowHeight = min(13, (int)(54 / numChannels));

    for (uint8_t i = 0; i < numChannels; i++) {
        uint8_t y = 11 + i * rowHeight;

        // Selection indicator
        if (i == selectedCh) {
            _oled.fillTriangle(0, y + 1, 0, y + rowHeight - 2, 4, y + rowHeight / 2, SSD1306_WHITE);
        }

        // Channel label
        _oled.setCursor(7, y + 1);
        _oled.printf("C%d", i + 1);

        // Current temp
        if (channels[i].getTCStatus() == TCStatus::OK) {
            _oled.setCursor(24, y + 1);
            _oled.printf("%5.0fF", channels[i].getCurrentTemp());
        } else {
            _oled.setCursor(24, y + 1);
            _oled.print(F(" ---F"));
        }

        // Arrow and target
        _oled.setCursor(66, y + 1);
        _oled.print(F(">"));
        _oled.printf("%3.0f", channels[i].getTargetTemp());

        // State
        _oled.setCursor(100, y + 1);
        _oled.print(channels[i].getStateString());
    }
}

void Display::drawSetTempScreen(Channel& ch, bool fineAdjust) {
    _oled.setTextSize(1);
    _oled.setCursor(16, 0);
    _oled.printf("SET TEMP - CH%d", ch.getIndex() + 1);

    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // Large target temp
    _oled.setTextSize(3);
    _oled.setCursor(10, 18);
    _oled.printf("%5.0f", ch.getTargetTemp());
    _oled.setTextSize(2);
    _oled.setCursor(106, 20);
    _oled.print(F("F"));

    // Fine adjust indicator
    _oled.setTextSize(1);
    _oled.setCursor(0, 52);
    if (fineAdjust) {
        _oled.print(F("FINE +/- 1F"));
    } else {
        _oled.print(F("STEP +/- 5F"));
    }
    _oled.setCursor(88, 52);
    _oled.print(F("[press]OK"));
}

void Display::drawSettingsMenu(uint8_t menuIndex, const SafetyManager& safety) {
    _oled.setTextSize(1);
    _oled.setCursor(28, 0);
    _oled.print(F("SETTINGS"));
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    const char* items[] = {
        "PID Tuning",
        "Idle Timeout",
        "System Info",
        "Factory Reset",
        "<< Back"
    };
    const uint8_t numItems = 5;

    for (uint8_t i = 0; i < numItems; i++) {
        uint8_t y = 13 + i * 10;
        if (i == menuIndex) {
            _oled.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
            _oled.setTextColor(SSD1306_BLACK);
        }
        _oled.setCursor(4, y);
        _oled.print(items[i]);
        _oled.setTextColor(SSD1306_WHITE);
    }
}

void Display::drawPIDTuneScreen(Channel& ch, uint8_t menuIndex) {
    _oled.setTextSize(1);
    _oled.setCursor(16, 0);
    _oled.printf("PID TUNE - CH%d", ch.getIndex() + 1);
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    const PIDController& pid = ch.getPID();
    const char* labels[] = { "Kp", "Ki", "Kd" };
    float values[] = { pid.getKp(), pid.getKi(), pid.getKd() };

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t y = 14 + i * 12;
        if (i == menuIndex) {
            _oled.fillRect(0, y - 1, 128, 11, SSD1306_WHITE);
            _oled.setTextColor(SSD1306_BLACK);
        }
        _oled.setCursor(4, y);
        _oled.printf("%s: %7.3f", labels[i], values[i]);
        _oled.setTextColor(SSD1306_WHITE);
    }

    _oled.setCursor(0, 52);
    _oled.print(F("Turn:adj Press:next"));
}

void Display::drawIdleTimeoutScreen(const SafetyManager& safety) {
    _oled.setTextSize(1);
    _oled.setCursor(16, 0);
    _oled.print(F("IDLE TIMEOUT"));
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    _oled.setTextSize(2);
    _oled.setCursor(16, 24);
    uint32_t timeout = safety.getIdleTimeout();
    if (timeout == 0) {
        _oled.print(F("OFF"));
    } else {
        _oled.printf("%u min", timeout);
    }

    _oled.setTextSize(1);
    _oled.setCursor(0, 52);
    _oled.print(F("Turn:adj Press:save"));
}

void Display::drawInfoScreen() {
    _oled.setTextSize(1);
    _oled.setCursor(28, 0);
    _oled.print(F("SYS INFO"));
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    _oled.setCursor(0, 14);
    _oled.printf("Model: %s", MODEL_NAME);
    _oled.setCursor(0, 24);
    _oled.printf("FW:    v%s", FW_VERSION);
    _oled.setCursor(0, 34);
    _oled.printf("Chans: %d", NUM_CHANNELS);
    _oled.setCursor(0, 44);
    _oled.printf("Heap:  %dK", ESP.getFreeHeap() / 1024);
    _oled.setCursor(0, 54);
    _oled.printf("Up:    %lus", millis() / 1000);
}

void Display::drawFaultScreen(Channel channels[], uint8_t numChannels,
                               const SafetyManager& safety) {
    // Flashing FAULT header
    if ((millis() / 500) % 2) {
        _oled.fillRect(0, 0, 128, 12, SSD1306_WHITE);
        _oled.setTextColor(SSD1306_BLACK);
        _oled.setTextSize(1);
        _oled.setCursor(36, 2);
        _oled.print(F("!! FAULT !!"));
        _oled.setTextColor(SSD1306_WHITE);
    } else {
        _oled.setTextSize(1);
        _oled.setCursor(36, 2);
        _oled.print(F("!! FAULT !!"));
    }

    _oled.drawLine(0, 13, 127, 13, SSD1306_WHITE);

    uint8_t y = 16;

    // Check safety faults
    uint8_t faults = safety.getFaults();
    if (faults & FAULT_OVERTEMP) {
        _oled.setCursor(4, y);
        _oled.print(F("OVER-TEMPERATURE"));
        y += 10;
    }
    if (faults & FAULT_TC_ERROR) {
        _oled.setCursor(4, y);
        _oled.print(F("THERMOCOUPLE ERROR"));
        y += 10;
    }
    if (faults & FAULT_IDLE_TIMEOUT) {
        _oled.setCursor(4, y);
        _oled.print(F("IDLE TIMEOUT"));
        y += 10;
    }

    // Per-channel faults
    for (uint8_t i = 0; i < numChannels; i++) {
        if (channels[i].isFaulted()) {
            _oled.setCursor(4, y);
            _oled.printf("CH%d: %s", i + 1, channels[i].getTC().getStatusString());
            y += 10;
        }
    }

    _oled.setCursor(4, 54);
    _oled.print(F("Press to clear"));
}

void Display::drawChannelSelectScreen(Channel channels[], uint8_t numChannels,
                                       uint8_t selectedCh) {
    _oled.setTextSize(1);
    _oled.setCursor(16, 0);
    _oled.print(F("SELECT CHANNEL"));
    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    for (uint8_t i = 0; i < numChannels; i++) {
        uint8_t y = 14 + i * 12;
        if (i == selectedCh) {
            _oled.fillRect(0, y - 1, 128, 11, SSD1306_WHITE);
            _oled.setTextColor(SSD1306_BLACK);
        }
        _oled.setCursor(4, y);
        _oled.printf("CH%d: %.0fF [%s]", i + 1,
                      channels[i].getCurrentTemp(),
                      channels[i].getStateString());
        _oled.setTextColor(SSD1306_WHITE);
    }
}

void Display::drawTempBar(int16_t x, int16_t y, int16_t w, int16_t h,
                           float current, float target) {
    // Draw outline
    _oled.drawRect(x, y, w, h, SSD1306_WHITE);

    // Fill proportional to current/target
    if (target > 0) {
        float ratio = constrain(current / target, 0.0f, 1.2f);
        int16_t fillW = (int16_t)(ratio * (w - 2));
        fillW = constrain(fillW, 0, w - 2);
        _oled.fillRect(x + 1, y + 1, fillW, h - 2, SSD1306_WHITE);

        // Target marker
        int16_t markerX = x + (int16_t)((float)(w - 2) * (target / TEMP_MAX_F));
        markerX = constrain(markerX, x + 1, x + w - 2);
        _oled.drawLine(markerX, y, markerX, y + h - 1, SSD1306_WHITE);
    }
}
