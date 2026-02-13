// ============================================================
// ESP-Nail: ESP32 E-Nail Controller
// Open-source PID temperature controller for heating coils
// Supports 1-4 independent channels via 5-pin mini-XLR
// ============================================================

#include <Arduino.h>
#include "config.h"
#include "channel.h"
#include "display.h"
#include "safety.h"
#include "storage.h"
#include "ui.h"

// --- Global Objects ---
Channel     channels[NUM_CHANNELS];
Display     display;
SafetyManager safety;
Storage     storage;
UIManager   ui;

// --- Forward Declarations ---
void handleUIEvent(UIEvent event);
void handleMainScreen(UIEvent event);
void handleSetTempScreen(UIEvent event);
void handleChannelSelectScreen(UIEvent event);
void handleSettingsScreen(UIEvent event);
void handlePIDTuneScreen(UIEvent event);
void handleIdleTimeoutScreen(UIEvent event);
void handleInfoScreen(UIEvent event);
void handleFaultScreen(UIEvent event);
void loadSettings();
void saveCurrentChannelSettings();
void checkGlobalFaults();

// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println(F("\n==========================="));
    Serial.printf("  %s v%s\n", MODEL_NAME, FW_VERSION);
    Serial.printf("  Channels: %d\n", NUM_CHANNELS);
    Serial.println(F("===========================\n"));

    // Initialize storage first to load settings
    storage.begin();

    // Initialize safety manager
    safety.begin();

    // Load global settings
    GlobalSettings gs = storage.loadGlobalSettings();
    safety.setIdleTimeout(gs.idleTimeoutMin);

    // Initialize channels
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i].begin(i, SSR_PINS[i], TC_CS_PINS[i]);

        ChannelSettings cs = storage.loadChannelSettings(i);
        channels[i].setTargetTemp(cs.targetTempF);
        channels[i].setPIDTunings(cs.kp, cs.ki, cs.kd);

        Serial.printf("CH%d: target=%.0fF  PID(%.1f, %.2f, %.1f)\n",
                       i + 1, cs.targetTempF, cs.kp, cs.ki, cs.kd);
    }

    // Initialize display
    display.begin();

    // Initialize UI
    ui.begin();

    Serial.println(F("System ready."));
    safety.confirmBeep();
}

// ============================================================
void loop() {
    // 1. Update all channels (thermocouple reads + PID + SSR)
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i].update();
    }

    // 2. Safety checks
    safety.update();
    checkGlobalFaults();

    // 3. Process UI input
    UIEvent event = ui.poll();
    if (event != UIEvent::NONE) {
        safety.resetIdleTimer(); // Any interaction resets idle
        handleUIEvent(event);
    }

    // 4. Update display
    display.update(channels, NUM_CHANNELS, safety, ui);
}

// ============================================================
// UI Event Routing
// ============================================================
void handleUIEvent(UIEvent event) {
    switch (ui.getCurrentScreen()) {
        case Screen::MAIN:           handleMainScreen(event); break;
        case Screen::SET_TEMP:       handleSetTempScreen(event); break;
        case Screen::CHANNEL_SELECT: handleChannelSelectScreen(event); break;
        case Screen::SETTINGS:       handleSettingsScreen(event); break;
        case Screen::PID_TUNE:       handlePIDTuneScreen(event); break;
        case Screen::IDLE_TIMEOUT:   handleIdleTimeoutScreen(event); break;
        case Screen::INFO:           handleInfoScreen(event); break;
        case Screen::FAULT:          handleFaultScreen(event); break;
    }
}

void handleMainScreen(UIEvent event) {
    uint8_t ch = ui.getSelectedChannel();

    switch (event) {
        case UIEvent::PRESS:
            // Toggle channel on/off
            if (channels[ch].isActive()) {
                channels[ch].disable();
                safety.confirmBeep();
            } else {
                channels[ch].enable();
                safety.confirmBeep();
            }
            break;

        case UIEvent::LONG_PRESS:
            // Enter settings
            ui.setScreen(Screen::SETTINGS);
            break;

        case UIEvent::ROTATE_CW:
        case UIEvent::ROTATE_CCW: {
            if (NUM_CHANNELS > 1) {
                // Multi-channel: rotate to select channel
                int8_t delta = (event == UIEvent::ROTATE_CW) ? 1 : -1;
                int8_t newCh = (int8_t)ch + delta;
                if (newCh < 0) newCh = NUM_CHANNELS - 1;
                if (newCh >= NUM_CHANNELS) newCh = 0;
                ui.setSelectedChannel(newCh);
            } else {
                // Single channel: go to set temp
                ui.setScreen(Screen::SET_TEMP);
                // Pre-adjust in the direction turned
                float step = ui.isFineAdjust() ? TEMP_STEP_FINE : TEMP_STEP_NORMAL;
                float delta = (event == UIEvent::ROTATE_CW) ? step : -step;
                channels[0].adjustTargetTemp(delta);
            }
            break;
        }
        default: break;
    }
}

void handleSetTempScreen(UIEvent event) {
    uint8_t ch = ui.getSelectedChannel();
    float step = ui.isFineAdjust() ? TEMP_STEP_FINE : TEMP_STEP_NORMAL;

    switch (event) {
        case UIEvent::ROTATE_CW:
            channels[ch].adjustTargetTemp(step);
            break;

        case UIEvent::ROTATE_CCW:
            channels[ch].adjustTargetTemp(-step);
            break;

        case UIEvent::PRESS:
            // Save and return to main
            saveCurrentChannelSettings();
            safety.confirmBeep();
            ui.setScreen(Screen::MAIN);
            break;

        case UIEvent::LONG_PRESS:
            // Toggle fine adjust
            ui.toggleFineAdjust();
            break;

        default: break;
    }
}

void handleChannelSelectScreen(UIEvent event) {
    uint8_t ch = ui.getSelectedChannel();

    switch (event) {
        case UIEvent::ROTATE_CW:
            if (ch < NUM_CHANNELS - 1) ui.setSelectedChannel(ch + 1);
            break;

        case UIEvent::ROTATE_CCW:
            if (ch > 0) ui.setSelectedChannel(ch - 1);
            break;

        case UIEvent::PRESS:
            ui.setScreen(Screen::SET_TEMP);
            break;

        case UIEvent::LONG_PRESS:
            ui.setScreen(Screen::MAIN);
            break;

        default: break;
    }
}

void handleSettingsScreen(UIEvent event) {
    uint8_t idx = ui.getMenuIndex();

    switch (event) {
        case UIEvent::ROTATE_CW:
            if (idx < 4) ui.setMenuIndex(idx + 1);
            break;

        case UIEvent::ROTATE_CCW:
            if (idx > 0) ui.setMenuIndex(idx - 1);
            break;

        case UIEvent::PRESS:
            switch (idx) {
                case 0: ui.setScreen(Screen::PID_TUNE); break;
                case 1: ui.setScreen(Screen::IDLE_TIMEOUT); break;
                case 2: ui.setScreen(Screen::INFO); break;
                case 3:
                    storage.factoryReset();
                    safety.confirmBeep();
                    ESP.restart();
                    break;
                case 4: ui.setScreen(Screen::MAIN); break;
            }
            break;

        case UIEvent::LONG_PRESS:
            ui.setScreen(Screen::MAIN);
            break;

        default: break;
    }
}

void handlePIDTuneScreen(UIEvent event) {
    uint8_t ch = ui.getSelectedChannel();
    uint8_t idx = ui.getMenuIndex();
    const PIDController& pid = channels[ch].getPID();

    float kp = pid.getKp();
    float ki = pid.getKi();
    float kd = pid.getKd();

    switch (event) {
        case UIEvent::ROTATE_CW:
        case UIEvent::ROTATE_CCW: {
            float delta = (event == UIEvent::ROTATE_CW) ? 0.1f : -0.1f;
            switch (idx) {
                case 0: kp = max(0.0f, kp + delta); break;
                case 1: ki = max(0.0f, ki + delta * 0.01f); break;
                case 2: kd = max(0.0f, kd + delta); break;
            }
            channels[ch].setPIDTunings(kp, ki, kd);
            break;
        }

        case UIEvent::PRESS:
            // Cycle through parameters, then exit
            if (idx < 2) {
                ui.setMenuIndex(idx + 1);
            } else {
                // Save and exit
                ChannelSettings cs;
                cs.targetTempF = channels[ch].getTargetTemp();
                cs.kp = kp;
                cs.ki = ki;
                cs.kd = kd;
                storage.saveChannelSettings(ch, cs);
                safety.confirmBeep();
                ui.setScreen(Screen::SETTINGS);
            }
            break;

        case UIEvent::LONG_PRESS:
            ui.setScreen(Screen::SETTINGS);
            break;

        default: break;
    }
}

void handleIdleTimeoutScreen(UIEvent event) {
    uint32_t timeout = safety.getIdleTimeout();

    switch (event) {
        case UIEvent::ROTATE_CW:
            safety.setIdleTimeout(min(120U, timeout + 5));
            break;

        case UIEvent::ROTATE_CCW:
            if (timeout >= 5) safety.setIdleTimeout(timeout - 5);
            else safety.setIdleTimeout(0);
            break;

        case UIEvent::PRESS: {
            // Save
            GlobalSettings gs = storage.loadGlobalSettings();
            gs.idleTimeoutMin = safety.getIdleTimeout();
            storage.saveGlobalSettings(gs);
            safety.confirmBeep();
            ui.setScreen(Screen::SETTINGS);
            break;
        }

        case UIEvent::LONG_PRESS:
            ui.setScreen(Screen::SETTINGS);
            break;

        default: break;
    }
}

void handleInfoScreen(UIEvent event) {
    if (event == UIEvent::PRESS || event == UIEvent::LONG_PRESS) {
        ui.setScreen(Screen::SETTINGS);
    }
}

void handleFaultScreen(UIEvent event) {
    if (event == UIEvent::PRESS) {
        // Clear faults and disable all channels
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            channels[i].disable();
        }
        safety.clearFault(FAULT_OVERTEMP);
        safety.clearFault(FAULT_TC_ERROR);
        safety.clearFault(FAULT_IDLE_TIMEOUT);
        ui.setScreen(Screen::MAIN);
    }
}

// ============================================================
// Helpers
// ============================================================
void saveCurrentChannelSettings() {
    uint8_t ch = ui.getSelectedChannel();
    const PIDController& pid = channels[ch].getPID();
    ChannelSettings cs;
    cs.targetTempF = channels[ch].getTargetTemp();
    cs.kp = pid.getKp();
    cs.ki = pid.getKi();
    cs.kd = pid.getKd();
    storage.saveChannelSettings(ch, cs);
}

void checkGlobalFaults() {
    if (safety.isShutdown()) return;

    // Idle timeout: disable all active channels
    if (safety.isIdleTimedOut()) {
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            if (channels[i].isActive()) {
                channels[i].disable();
            }
        }
        ui.setScreen(Screen::FAULT);
        return;
    }

    // Check for any faulted channel
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        if (channels[i].isFaulted()) {
            if (channels[i].getTCStatus() != TCStatus::OK) {
                safety.setFault(FAULT_TC_ERROR);
            }
            if (channels[i].getCurrentTemp() >= TEMP_ABS_MAX_F) {
                safety.setFault(FAULT_OVERTEMP);
            }
            ui.setScreen(Screen::FAULT);
            return;
        }
    }
}
