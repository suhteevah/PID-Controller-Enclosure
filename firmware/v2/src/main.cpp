// ============================================================
// ESP-Nail v2: ESP32 E-Nail Controller
// FreeRTOS multi-core architecture
// ============================================================

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "config.h"
#include "version.h"

// Core
#include "core/pid.h"
#include "core/channel.h"
#include "core/safety.h"
#include "core/autotune.h"

// Drivers
#include "drivers/thermocouple.h"
#include "drivers/ssr.h"
#include "drivers/display_ssd1306.h"
#include "drivers/encoder.h"
#include "drivers/buzzer.h"

// UI
#include "ui/screen_manager.h"
#include "ui/screen_main.h"
#include "ui/screen_settings.h"
#include "ui/screen_profiles.h"
#include "ui/screen_wifi.h"
#include "ui/widgets.h"

// Network
#if ENABLE_WIFI
#include "network/wifi_manager.h"
#include "network/web_server.h"
#include "network/mdns_service.h"
#endif
#if ENABLE_BLE
#include "network/ble_service.h"
#endif
#if ENABLE_MQTT
#include "network/mqtt_client.h"
#endif
#if ENABLE_OTA
#include "network/ota_updater.h"
#endif

// Data
#include "data/storage.h"
#include "data/profiles.h"
#include "data/session_log.h"
#include "data/calibration.h"

// ============================================================
// Global Objects
// ============================================================

// Inter-task queues
static QueueHandle_t queueTemp;         // PID → UI/Network
static QueueHandle_t queueCommand;      // UI/Network → PID
static QueueHandle_t queueFault;        // Safety → UI
static SemaphoreHandle_t mutexStorage;  // NVS access mutex

// Core
static Channel channels[NUM_CHANNELS];
static SafetyManager safety;

// Drivers
static DisplaySSD1306 displayDriver;
static RotaryEncoder encoder;
static Buzzer buzzer;

// UI
static ScreenManager screenMgr;

// Data
static Storage storage;
static ProfileManager profiles;
static SessionLogger sessionLog;
static CalibrationManager calibration;

// Network
#if ENABLE_WIFI
static WiFiManager wifiMgr;
static WebServer webServer;
static MDNSService mdnsService;
#endif
#if ENABLE_BLE
static BLEService bleService;
#endif
#if ENABLE_MQTT
static MQTTClient mqttClient;
#endif

// ============================================================
// Task: PID Control (Core 1, highest priority)
// Reads thermocouples, computes PID, drives SSRs
// ============================================================
void taskPID(void* param) {
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        // Process incoming commands from UI/Network
        ChannelCommand cmd;
        while (xQueueReceive(queueCommand, &cmd, 0) == pdTRUE) {
            if (cmd.channel >= NUM_CHANNELS) continue;
            Channel& ch = channels[cmd.channel];

            switch (cmd.type) {
                case ChannelCommand::CMD_ENABLE:
                    ch.enable();
                    sessionLog.startSession(cmd.channel, ch.getTargetTemp());
                    break;
                case ChannelCommand::CMD_DISABLE:
                    ch.disable();
                    sessionLog.endSession(cmd.channel);
                    break;
                case ChannelCommand::CMD_SET_TEMP:
                    ch.setTargetTemp(cmd.value);
                    break;
                case ChannelCommand::CMD_ADJUST_TEMP:
                    ch.adjustTargetTemp(cmd.value);
                    break;
                case ChannelCommand::CMD_SET_PID:
                    ch.setPIDTunings(cmd.kp, cmd.ki, cmd.kd);
                    break;
                case ChannelCommand::CMD_START_AUTOTUNE:
                    ch.startAutotune();
                    break;
                case ChannelCommand::CMD_CANCEL_AUTOTUNE:
                    ch.cancelAutotune();
                    break;
                case ChannelCommand::CMD_LOAD_PROFILE: {
                    Profile p = profiles.getProfile(cmd.channel, cmd.profileIndex);
                    ch.setTargetTemp(p.tempF);
                    if (p.hasCustomPID) {
                        ch.setPIDTunings(p.kp, p.ki, p.kd);
                    }
                    break;
                }
                case ChannelCommand::CMD_CLEAR_FAULT:
                    ch.disable();
                    break;
            }
        }

        // Update all channels (TC read + PID + SSR)
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            channels[i].update();

            // Publish temp updates to UI/Network
            TempUpdate update = channels[i].getTempUpdate();

            // Apply calibration offset for display
            if (calibration.isCalibrated(i)) {
                update.currentTemp = calibration.getCalibratedTemp(i, update.currentTemp);
            }

            xQueueOverwrite(queueTemp, &update);  // Latest wins

            // Session logging data point
            if (channels[i].isActive()) {
                sessionLog.addDataPoint(i, update.currentTemp, update.pidOutput);
            }
        }

        // Check for idle timeout triggering channel shutdowns
        if (safety.isIdleTimedOut()) {
            for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
                if (channels[i].isActive()) {
                    channels[i].disable();
                    sessionLog.endSession(i);
                }
            }
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PID_SAMPLE_MS));
    }
}

// ============================================================
// Task: Safety Monitor (Core 1, highest priority)
// Watchdog feed, idle timeout, fault detection
// ============================================================
void taskSafety(void* param) {
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        safety.update();

        // Check all channels for faults
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            if (channels[i].isFaulted()) {
                if (!channels[i].isTCOk()) {
                    safety.setFault(FAULT_TC_ERROR, i, channels[i].getCurrentTemp());
                }
                if (channels[i].getCurrentTemp() >= TEMP_ABS_MAX_F) {
                    safety.setFault(FAULT_OVERTEMP, i, channels[i].getCurrentTemp());
                }
            }
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
    }
}

// ============================================================
// Task: UI (Core 0)
// Encoder input, display rendering, menu navigation
// ============================================================
void taskUI(void* param) {
    TickType_t lastWake = xTaskGetTickCount();
    TempUpdate latestTemps[NUM_CHANNELS] = {};

    for (;;) {
        // Read latest temps from PID task
        TempUpdate update;
        while (xQueueReceive(queueTemp, &update, 0) == pdTRUE) {
            if (update.channel < NUM_CHANNELS) {
                latestTemps[update.channel] = update;
            }
        }

        // Read fault events
        FaultEvent faultEvt;
        while (xQueueReceive(queueFault, &faultEvt, 0) == pdTRUE) {
            screenMgr.setScreen(Screen::FAULT);
            buzzer.playAlarm();
        }

        // Process encoder
        EncoderEvent evt = encoder.poll();
        if (evt != EncoderEvent::NONE) {
            safety.resetIdleTimer();
            screenMgr.handleEvent(evt, channels, latestTemps, queueCommand,
                                  safety, profiles, storage, calibration);
        }

        // Update buzzer (non-blocking)
        buzzer.update();

        // Render display
        displayDriver.clear();
        screenMgr.render(&displayDriver, channels, latestTemps, NUM_CHANNELS,
                         safety, profiles);
        displayDriver.display();

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(DISPLAY_UPDATE_MS));
    }
}

// ============================================================
// Task: Network (Core 0)
// WiFi, BLE, Web Server, MQTT, OTA
// ============================================================
#if ENABLE_WIFI || ENABLE_BLE
void taskNetwork(void* param) {
    // Initialize networking (after WiFi stack is ready)
    #if ENABLE_WIFI
    {
        xSemaphoreTake(mutexStorage, portMAX_DELAY);
        GlobalSettings gs = storage.loadGlobalSettings();
        xSemaphoreGive(mutexStorage);

        wifiMgr.begin(gs.wifiMode, gs.wifiSSID, gs.wifiPass);
        webServer.begin(&wifiMgr, channels, &safety, &profiles,
                        &sessionLog, &calibration, &storage, queueCommand);
        mdnsService.begin();
    }
    #endif

    #if ENABLE_BLE
    bleService.begin(channels, queueCommand);
    #endif

    #if ENABLE_MQTT
    {
        xSemaphoreTake(mutexStorage, portMAX_DELAY);
        GlobalSettings gs = storage.loadGlobalSettings();
        xSemaphoreGive(mutexStorage);

        mqttClient.begin(gs.mqttHost, gs.mqttPort, gs.mqttUser, gs.mqttPass);
    }
    #endif

    TickType_t lastWake = xTaskGetTickCount();
    uint32_t lastPublish = 0;

    for (;;) {
        #if ENABLE_WIFI
        wifiMgr.update();
        // WebSocket temp broadcast
        if (wifiMgr.isConnected() || wifiMgr.getMode() == 0) {
            webServer.broadcastTemps(channels, NUM_CHANNELS);
        }
        #endif

        #if ENABLE_BLE
        bleService.update(channels, NUM_CHANNELS);
        #endif

        #if ENABLE_MQTT
        if (wifiMgr.isConnected()) {
            mqttClient.update();
            uint32_t now = millis();
            if ((now - lastPublish) >= (MQTT_PUBLISH_INTERVAL_S * 1000)) {
                for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
                    mqttClient.publishChannel(i, channels[i]);
                }
                lastPublish = now;
            }
        }
        #endif

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(50));
    }
}
#endif

// ============================================================
// Task: Logger (Core 0, lowest priority)
// Session data recording and maintenance
// ============================================================
void taskLogger(void* param) {
    TickType_t lastWake = xTaskGetTickCount();

    for (;;) {
        sessionLog.update();

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000));
    }
}

// ============================================================
// Arduino setup() - runs once, creates all RTOS tasks
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("==================================="));
    Serial.printf("  %s  v%s\n", MODEL_NAME, FW_VERSION_STRING);
    Serial.printf("  Channels: %d | Build: %s\n", NUM_CHANNELS, FW_BUILD_DATE);
    Serial.println(F("==================================="));

    // Create inter-task communication
    queueTemp    = xQueueCreate(QUEUE_TEMP_SIZE, sizeof(TempUpdate));
    queueCommand = xQueueCreate(QUEUE_CMD_SIZE, sizeof(ChannelCommand));
    queueFault   = xQueueCreate(QUEUE_FAULT_SIZE, sizeof(FaultEvent));
    mutexStorage = xSemaphoreCreateMutex();

    // Initialize storage
    storage.begin();
    profiles.begin();
    sessionLog.begin();
    calibration.begin();

    // Initialize safety
    safety.begin(queueFault);

    // Load settings and init channels
    GlobalSettings gs = storage.loadGlobalSettings();
    safety.setIdleTimeout(gs.idleTimeoutMin);

    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i].begin(i, SSR_PINS[i], TC_CS_PINS[i]);

        ChannelSettings cs = storage.loadChannelSettings(i);
        channels[i].setTargetTemp(cs.targetTempF);
        channels[i].setPIDTunings(cs.kp, cs.ki, cs.kd);

        Serial.printf("  CH%d: %.0fF  PID(%.1f, %.2f, %.1f)\n",
                       i + 1, cs.targetTempF, cs.kp, cs.ki, cs.kd);
    }

    // Initialize drivers
    displayDriver.begin();
    encoder.begin();
    buzzer.begin();

    // Splash screen
    displayDriver.clear();
    displayDriver.setTextSize(2);
    displayDriver.setCursor(16, 8);
    displayDriver.print("ESP-Nail");
    displayDriver.setTextSize(1);
    displayDriver.setCursor(20, 32);
    displayDriver.printf("%s v%s", MODEL_NAME, FW_VERSION_STRING);
    displayDriver.setCursor(20, 48);
    displayDriver.printf("%d Channel%s | WiFi", NUM_CHANNELS, NUM_CHANNELS > 1 ? "s" : "");
    displayDriver.display();

    buzzer.playStartup();
    // Process startup buzzer before tasks take over
    for (int i = 0; i < 30; i++) { buzzer.update(); delay(20); }

    // ---- Create RTOS Tasks ----

    xTaskCreatePinnedToCore(taskPID, "PID",
        TASK_PID_STACK, NULL, TASK_PID_PRIORITY, NULL, TASK_PID_CORE);

    xTaskCreatePinnedToCore(taskSafety, "Safety",
        TASK_SAFETY_STACK, NULL, TASK_SAFETY_PRIORITY, NULL, TASK_SAFETY_CORE);

    xTaskCreatePinnedToCore(taskUI, "UI",
        TASK_UI_STACK, NULL, TASK_UI_PRIORITY, NULL, TASK_UI_CORE);

    #if ENABLE_WIFI || ENABLE_BLE
    xTaskCreatePinnedToCore(taskNetwork, "Network",
        TASK_NETWORK_STACK, NULL, TASK_NETWORK_PRIORITY, NULL, TASK_NETWORK_CORE);
    #endif

    xTaskCreatePinnedToCore(taskLogger, "Logger",
        TASK_LOGGER_STACK, NULL, TASK_LOGGER_PRIORITY, NULL, TASK_LOGGER_CORE);

    Serial.println(F("\nAll tasks launched. System running."));
}

// Arduino loop() is unused - all work happens in RTOS tasks
void loop() {
    vTaskDelay(portMAX_DELAY);
}
