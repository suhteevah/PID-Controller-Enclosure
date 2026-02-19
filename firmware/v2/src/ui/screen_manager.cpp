#include "screen_manager.h"
#include "core/channel.h"
#include "core/safety.h"
#include "data/profiles.h"
#include "data/storage.h"
#include "data/calibration.h"
#include "drivers/display_ssd1306.h"
#include "ui/widgets.h"

ScreenManager::ScreenManager()
    : _current(Screen::MAIN), _selectedCh(0), _menuIdx(0), _fineAdj(false) {}

void ScreenManager::setScreen(Screen s) {
    _current = s;
    _menuIdx = 0;
}

void ScreenManager::handleEvent(EncoderEvent evt, Channel channels[], TempUpdate temps[],
                                 QueueHandle_t cmdQueue, SafetyManager& safety,
                                 ProfileManager& profiles, Storage& storage,
                                 CalibrationManager& calibration) {
    uint8_t ch = _selectedCh;
    ChannelCommand cmd = {};
    cmd.channel = ch;

    switch (_current) {
        case Screen::MAIN:
            if (evt == EncoderEvent::PRESS) {
                cmd.type = channels[ch].isActive() ?
                    ChannelCommand::CMD_DISABLE : ChannelCommand::CMD_ENABLE;
                xQueueSend(cmdQueue, &cmd, 0);
            } else if (evt == EncoderEvent::LONG_PRESS) {
                setScreen(Screen::SETTINGS);
            } else if (evt == EncoderEvent::ROTATE_CW || evt == EncoderEvent::ROTATE_CCW) {
                if (NUM_CHANNELS > 1) {
                    int8_t d = (evt == EncoderEvent::ROTATE_CW) ? 1 : -1;
                    _selectedCh = (uint8_t)(((int8_t)_selectedCh + d + NUM_CHANNELS) % NUM_CHANNELS);
                } else {
                    setScreen(Screen::SET_TEMP);
                    float step = _fineAdj ? TEMP_STEP_FINE : TEMP_STEP_NORMAL;
                    cmd.type = ChannelCommand::CMD_ADJUST_TEMP;
                    cmd.value = (evt == EncoderEvent::ROTATE_CW) ? step : -step;
                    xQueueSend(cmdQueue, &cmd, 0);
                }
            }
            break;

        case Screen::SET_TEMP: {
            float step = _fineAdj ? TEMP_STEP_FINE : TEMP_STEP_NORMAL;
            if (evt == EncoderEvent::ROTATE_CW || evt == EncoderEvent::ROTATE_CCW) {
                cmd.type = ChannelCommand::CMD_ADJUST_TEMP;
                cmd.value = (evt == EncoderEvent::ROTATE_CW) ? step : -step;
                xQueueSend(cmdQueue, &cmd, 0);
            } else if (evt == EncoderEvent::PRESS) {
                // Save and return
                xSemaphoreTake(xSemaphoreCreateMutex(), portMAX_DELAY); // simplified
                ChannelSettings cs;
                cs.targetTempF = channels[ch].getTargetTemp();
                cs.kp = channels[ch].getPID().getKp();
                cs.ki = channels[ch].getPID().getKi();
                cs.kd = channels[ch].getPID().getKd();
                storage.saveChannelSettings(ch, cs);
                setScreen(Screen::MAIN);
            } else if (evt == EncoderEvent::LONG_PRESS) {
                toggleFineAdjust();
            }
            break;
        }

        case Screen::SETTINGS:
            if (evt == EncoderEvent::ROTATE_CW && _menuIdx < 6) _menuIdx++;
            else if (evt == EncoderEvent::ROTATE_CCW && _menuIdx > 0) _menuIdx--;
            else if (evt == EncoderEvent::PRESS) {
                switch (_menuIdx) {
                    case 0: setScreen(Screen::PID_TUNE); break;
                    case 1: setScreen(Screen::PROFILES); break;
                    case 2: setScreen(Screen::IDLE_TIMEOUT); break;
                    case 3: setScreen(Screen::WIFI_STATUS); break;
                    case 4: setScreen(Screen::INFO); break;
                    case 5: storage.factoryReset(); ESP.restart(); break;
                    case 6: setScreen(Screen::MAIN); break;
                }
            } else if (evt == EncoderEvent::LONG_PRESS) {
                setScreen(Screen::MAIN);
            }
            break;

        case Screen::PROFILES:
            if (evt == EncoderEvent::ROTATE_CW && _menuIdx < MAX_PROFILES_PER_CH - 1) _menuIdx++;
            else if (evt == EncoderEvent::ROTATE_CCW && _menuIdx > 0) _menuIdx--;
            else if (evt == EncoderEvent::PRESS) {
                profiles.setActiveProfile(ch, _menuIdx);
                cmd.type = ChannelCommand::CMD_LOAD_PROFILE;
                cmd.profileIndex = _menuIdx;
                xQueueSend(cmdQueue, &cmd, 0);
                setScreen(Screen::MAIN);
            } else if (evt == EncoderEvent::LONG_PRESS) {
                setScreen(Screen::SETTINGS);
            }
            break;

        case Screen::PID_TUNE:
            if (evt == EncoderEvent::ROTATE_CW || evt == EncoderEvent::ROTATE_CCW) {
                float delta = (evt == EncoderEvent::ROTATE_CW) ? 0.1f : -0.1f;
                float kp = channels[ch].getPID().getKp();
                float ki = channels[ch].getPID().getKi();
                float kd = channels[ch].getPID().getKd();
                switch (_menuIdx) {
                    case 0: kp = max(0.0f, kp + delta); break;
                    case 1: ki = max(0.0f, ki + delta * 0.01f); break;
                    case 2: kd = max(0.0f, kd + delta); break;
                    case 3: break; // Auto-tune row
                }
                cmd.type = ChannelCommand::CMD_SET_PID;
                cmd.kp = kp; cmd.ki = ki; cmd.kd = kd;
                xQueueSend(cmdQueue, &cmd, 0);
            } else if (evt == EncoderEvent::PRESS) {
                if (_menuIdx < 3) _menuIdx++;
                else if (_menuIdx == 3) {
                    cmd.type = ChannelCommand::CMD_START_AUTOTUNE;
                    xQueueSend(cmdQueue, &cmd, 0);
                    setScreen(Screen::AUTOTUNE);
                } else {
                    setScreen(Screen::SETTINGS);
                }
            } else if (evt == EncoderEvent::LONG_PRESS) {
                setScreen(Screen::SETTINGS);
            }
            break;

        case Screen::IDLE_TIMEOUT:
            if (evt == EncoderEvent::ROTATE_CW) {
                safety.setIdleTimeout(min((uint32_t)IDLE_TIMEOUT_MIN_MAX, safety.getIdleTimeout() + 5));
            } else if (evt == EncoderEvent::ROTATE_CCW) {
                uint32_t t = safety.getIdleTimeout();
                safety.setIdleTimeout(t >= 5 ? t - 5 : 0);
            } else if (evt == EncoderEvent::PRESS) {
                GlobalSettings gs = storage.loadGlobalSettings();
                gs.idleTimeoutMin = safety.getIdleTimeout();
                storage.saveGlobalSettings(gs);
                setScreen(Screen::SETTINGS);
            } else if (evt == EncoderEvent::LONG_PRESS) {
                setScreen(Screen::SETTINGS);
            }
            break;

        case Screen::AUTOTUNE:
            if (evt == EncoderEvent::PRESS || evt == EncoderEvent::LONG_PRESS) {
                cmd.type = ChannelCommand::CMD_CANCEL_AUTOTUNE;
                xQueueSend(cmdQueue, &cmd, 0);
                setScreen(Screen::MAIN);
            }
            break;

        case Screen::FAULT:
            if (evt == EncoderEvent::PRESS) {
                for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
                    ChannelCommand c;
                    c.type = ChannelCommand::CMD_CLEAR_FAULT;
                    c.channel = i;
                    xQueueSend(cmdQueue, &c, 0);
                }
                safety.clearFault(FAULT_OVERTEMP);
                safety.clearFault(FAULT_TC_ERROR);
                safety.clearFault(FAULT_IDLE_TIMEOUT);
                setScreen(Screen::MAIN);
            }
            break;

        case Screen::INFO:
        case Screen::WIFI_STATUS:
            if (evt == EncoderEvent::PRESS || evt == EncoderEvent::LONG_PRESS) {
                setScreen(Screen::SETTINGS);
            }
            break;

        default: break;
    }
}

void ScreenManager::render(DisplayDriver* d, Channel channels[], TempUpdate temps[],
                            uint8_t numCh, SafetyManager& safety, ProfileManager& profiles) {
    switch (_current) {
        case Screen::MAIN:
            if (numCh == 1) {
                // Single channel large display
                Widgets::drawHeader(d, channels[0].getStateString());
                d->setTextSize(3);
                d->setCursor(4, 16);
                if (channels[0].isTCOk()) {
                    d->printf("%5.1f", temps[0].currentTemp);
                    d->setTextSize(1);
                    d->setCursor(112, 16);
                    d->print("F");
                } else {
                    d->print("---.-");
                }
                d->setTextSize(1);
                d->setCursor(0, 44);
                d->printf("SET:%.0fF", channels[0].getTargetTemp());
                d->setCursor(70, 44);
                d->printf("%3.0f%%", channels[0].getPIDOutput());
                if (safety.getIdleMinRemaining() > 0 && channels[0].isActive()) {
                    d->setCursor(100, 0);
                    d->printf("%um", safety.getIdleMinRemaining());
                }
                Widgets::drawTempBar(d, 0, 56, 128, 8, temps[0].currentTemp, channels[0].getTargetTemp());
            } else {
                // Multi-channel compact view
                Widgets::drawHeader(d, MODEL_NAME);
                for (uint8_t i = 0; i < numCh && i < 4; i++) {
                    uint8_t y = 11 + i * 13;
                    if (i == _selectedCh) {
                        d->drawTriangle(0, y + 1, 0, y + 9, 4, y + 5);
                    }
                    d->setTextSize(1);
                    d->setCursor(7, y + 1);
                    d->printf("C%d", i + 1);
                    d->setCursor(24, y + 1);
                    if (channels[i].isTCOk()) {
                        d->printf("%5.0fF", temps[i].currentTemp);
                    } else {
                        d->print(" ---F");
                    }
                    d->setCursor(66, y + 1);
                    d->printf(">%3.0f", channels[i].getTargetTemp());
                    d->setCursor(100, y + 1);
                    d->print(channels[i].getStateString());
                }
            }
            break;

        case Screen::SET_TEMP:
            Widgets::drawHeader(d, "SET TEMP");
            d->setTextSize(3);
            d->setCursor(10, 18);
            d->printf("%5.0f", channels[_selectedCh].getTargetTemp());
            d->setTextSize(2);
            d->setCursor(106, 20);
            d->print("F");
            d->setTextSize(1);
            Widgets::drawFooter(d, _fineAdj ? "FINE +/-1F  [OK]" : "STEP +/-5F  [OK]");
            break;

        case Screen::SETTINGS: {
            Widgets::drawHeader(d, "SETTINGS");
            const char* items[] = {"PID Tuning", "Profiles", "Idle Timeout",
                                   "WiFi", "System Info", "Factory Reset", "<< Back"};
            for (uint8_t i = 0; i < 7; i++) {
                Widgets::drawMenuItem(d, 13 + i * 7, items[i], i == _menuIdx);
            }
            break;
        }

        case Screen::PROFILES: {
            Widgets::drawHeader(d, "PROFILES");
            uint8_t activePr = profiles.getActiveProfile(_selectedCh);
            for (uint8_t i = 0; i < MAX_PROFILES_PER_CH && i < 6; i++) {
                Profile p = profiles.getProfile(_selectedCh, i);
                uint8_t y = 12 + i * 8;
                bool sel = (i == _menuIdx);
                if (sel) d->fillRect(0, y - 1, 128, 9);
                if (sel) d->setInvertText(true);
                d->setCursor(2, y);
                d->printf("%s%.0fF%s", p.name, p.tempF, (i == activePr) ? " *" : "");
                if (sel) d->setInvertText(false);
            }
            break;
        }

        case Screen::PID_TUNE: {
            Widgets::drawHeader(d, "PID TUNE");
            const PIDController& pid = channels[_selectedCh].getPID();
            const char* labels[] = {"Kp", "Ki", "Kd", "Auto-Tune"};
            float values[] = {pid.getKp(), pid.getKi(), pid.getKd(), 0};
            for (uint8_t i = 0; i < 4; i++) {
                uint8_t y = 14 + i * 11;
                Widgets::drawMenuItem(d, y, "", i == _menuIdx);
                if (i == _menuIdx) d->setInvertText(true);
                d->setCursor(4, y);
                if (i < 3) d->printf("%s: %7.3f", labels[i], values[i]);
                else d->print(">> Auto-Tune");
                if (i == _menuIdx) d->setInvertText(false);
            }
            break;
        }

        case Screen::IDLE_TIMEOUT:
            Widgets::drawHeader(d, "IDLE TIMEOUT");
            d->setTextSize(2);
            d->setCursor(16, 24);
            if (safety.getIdleTimeout() == 0) d->print("OFF");
            else d->printf("%u min", safety.getIdleTimeout());
            d->setTextSize(1);
            Widgets::drawFooter(d, "Turn:adj  Press:save");
            break;

        case Screen::WIFI_STATUS:
            Widgets::drawHeader(d, "WIFI");
            d->setTextSize(1);
            d->setCursor(0, 14);
            d->print("Mode: AP/STA");
            d->setCursor(0, 24);
            d->print("SSID: ESPNail-XXXX");
            d->setCursor(0, 34);
            d->print("IP: 192.168.4.1");
            d->setCursor(0, 44);
            d->print("http://espnail.local");
            Widgets::drawFooter(d, "Press:back");
            break;

        case Screen::INFO:
            Widgets::drawHeader(d, "SYS INFO");
            d->setCursor(0, 14);
            d->printf("Model: %s", MODEL_NAME);
            d->setCursor(0, 24);
            d->printf("FW: v%s", FW_VERSION_STRING);
            d->setCursor(0, 34);
            d->printf("Chans: %d", NUM_CHANNELS);
            d->setCursor(0, 44);
            d->printf("Heap: %dK", ESP.getFreeHeap() / 1024);
            d->setCursor(0, 54);
            d->printf("Up: %lus", millis() / 1000);
            break;

        case Screen::AUTOTUNE: {
            Widgets::drawHeader(d, "AUTO-TUNE");
            d->setTextSize(1);
            d->setCursor(8, 20);
            d->printf("Channel %d", _selectedCh + 1);
            float prog = channels[_selectedCh].getAutotuneProgress();
            Widgets::drawProgressBar(d, 8, 32, 112, 10, prog * 100.0f);
            d->setCursor(8, 48);
            d->printf("%.0f%%", prog * 100.0f);
            Widgets::drawFooter(d, "Press:cancel");
            break;
        }

        case Screen::FAULT: {
            bool blink = (millis() / 500) % 2;
            if (blink) {
                d->fillRect(0, 0, 128, 12);
                d->setInvertText(true);
            }
            d->setTextSize(1);
            d->setCursor(28, 2);
            d->print("!! FAULT !!");
            d->setInvertText(false);
            d->drawLine(0, 13, 127, 13);

            uint8_t y = 16;
            uint8_t faults = safety.getFaults();
            if (faults & FAULT_OVERTEMP) { d->setCursor(4, y); d->print("OVER-TEMPERATURE"); y += 10; }
            if (faults & FAULT_TC_ERROR) { d->setCursor(4, y); d->print("THERMOCOUPLE ERROR"); y += 10; }
            if (faults & FAULT_IDLE_TIMEOUT) { d->setCursor(4, y); d->print("IDLE TIMEOUT"); y += 10; }
            if (faults & FAULT_SSR_STUCK) { d->setCursor(4, y); d->print("SSR STUCK"); y += 10; }
            Widgets::drawFooter(d, "Press:clear");
            break;
        }

        default: break;
    }
}
