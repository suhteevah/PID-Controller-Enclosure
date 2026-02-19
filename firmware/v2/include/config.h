#pragma once

// ============================================================
// ESP-Nail v2 - Global Configuration
// ============================================================

#include <Arduino.h>

// --- Build Defaults ---
#ifndef NUM_CHANNELS
#define NUM_CHANNELS 1
#endif

#ifndef MODEL_NAME
#define MODEL_NAME "ESP-Nail"
#endif

#ifndef ENABLE_WIFI
#define ENABLE_WIFI 1
#endif

#ifndef ENABLE_BLE
#define ENABLE_BLE 1
#endif

#ifndef ENABLE_OTA
#define ENABLE_OTA 1
#endif

#ifndef ENABLE_MQTT
#define ENABLE_MQTT 0
#endif

#ifndef DISPLAY_TYPE_SSD1306
#define DISPLAY_TYPE_SSD1306 1
#endif

#ifndef DISPLAY_TYPE_ST7789
#define DISPLAY_TYPE_ST7789 0
#endif

// --- Pin Assignments (ESP32 DevKit V1 / Custom PCB) ---

// I2C - OLED Display (SSD1306)
#define PIN_SDA             21
#define PIN_SCL             22

// SPI - Thermocouple (MAX31855) & TFT Display
#define PIN_SPI_MISO        19
#define PIN_SPI_MOSI        23  // Used for TFT only
#define PIN_SPI_SCK         18

// MAX31855 CS pins per channel
#define PIN_TC_CS_1         5
#define PIN_TC_CS_2         17
#define PIN_TC_CS_3         16
#define PIN_TC_CS_4         4

// SSR Output Pins per channel
#define PIN_SSR_1           25
#define PIN_SSR_2           26
#define PIN_SSR_3           27
#define PIN_SSR_4           14

// Rotary Encoder
#define PIN_ENC_CLK         32
#define PIN_ENC_DT          33
#define PIN_ENC_SW          35

// Buzzer (passive piezo)
#define PIN_BUZZER          13

// Status LED (built-in)
#define PIN_STATUS_LED      2

// ST7789 TFT Display (optional, if DISPLAY_TYPE_ST7789)
#define PIN_TFT_CS          15
#define PIN_TFT_DC          0
#define PIN_TFT_RST         -1  // Tied to ESP32 reset

// ACS712 Current Sensor (optional, future)
#define PIN_CURRENT_SENSE   36  // ADC1_CH0 (input only)

// Pin arrays for indexed access
static const uint8_t TC_CS_PINS[]  = { PIN_TC_CS_1, PIN_TC_CS_2, PIN_TC_CS_3, PIN_TC_CS_4 };
static const uint8_t SSR_PINS[]    = { PIN_SSR_1, PIN_SSR_2, PIN_SSR_3, PIN_SSR_4 };

// --- Temperature Limits ---
#define TEMP_MIN_F              0.0f
#define TEMP_MAX_F              999.0f
#define TEMP_DEFAULT_F          710.0f
#define TEMP_ABS_MAX_F          1050.0f     // Hard shutdown
#define TEMP_HOLDING_BAND_F     10.0f       // Within this = HOLDING state
#define TEMP_HEATING_BAND_F     20.0f       // Outside this = HEATING state
#define TEMP_COOLDOWN_THRESH_F  150.0f      // Below this = OFF (from COOLDOWN)

// --- PID Defaults ---
#define PID_KP_DEFAULT          8.0f
#define PID_KI_DEFAULT          0.2f
#define PID_KD_DEFAULT          2.0f
#define PID_SAMPLE_MS           250
#define PID_OUTPUT_MIN          0.0f
#define PID_OUTPUT_MAX          100.0f
#define PID_DERIVATIVE_FILTER   0.1f

// --- SSR Time-Proportioning ---
#define SSR_PERIOD_MS           1000
#define SSR_MIN_ON_MS           50

// --- Safety ---
#define IDLE_TIMEOUT_MIN_DEFAULT    60
#define IDLE_TIMEOUT_MIN_MAX        120
#define WATCHDOG_TIMEOUT_S          10
#define TC_READ_INTERVAL_MS         250
#define TC_ERROR_COUNT_MAX          10

// --- Display ---
#define OLED_WIDTH              128
#define OLED_HEIGHT             64
#define OLED_ADDR               0x3C
#define DISPLAY_UPDATE_MS       100

// TFT (ST7789)
#define TFT_WIDTH               240
#define TFT_HEIGHT              240

// --- UI ---
#define ENCODER_DEBOUNCE_MS     5
#define BUTTON_DEBOUNCE_MS      50
#define BUTTON_LONG_PRESS_MS    1000
#define TEMP_STEP_NORMAL        5.0f
#define TEMP_STEP_FINE          1.0f

// --- Profiles ---
#define MAX_PROFILES_PER_CH     8
#define PROFILE_NAME_MAX_LEN    16

// --- Session Logging ---
#define MAX_SESSION_RECORDS     50
#define SESSION_SAMPLE_INTERVAL_S   30  // Log a data point every 30s

// --- Network ---
#define WIFI_AP_SSID_PREFIX     "ESPNail-"
#define WIFI_AP_PASSWORD        "espnail42"
#define WIFI_CONNECT_TIMEOUT_S  15
#define WEB_SERVER_PORT         80
#define MDNS_HOSTNAME           "espnail"

// MQTT
#define MQTT_PORT               1883
#define MQTT_TOPIC_PREFIX       "espnail/"
#define MQTT_PUBLISH_INTERVAL_S 5

// BLE
#define BLE_DEVICE_NAME_PREFIX  "ESPNail-"
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHAR_TEMP_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_CHAR_CMD_UUID       "beb5483f-36e1-4688-b7f5-ea07361b26a8"
#define BLE_CHAR_STATE_UUID     "beb54840-36e1-4688-b7f5-ea07361b26a8"

// --- Storage / NVS ---
#define NVS_NAMESPACE           "enail2"
#define NVS_SETTINGS_VERSION    2

// --- FreeRTOS Task Config ---
#define TASK_PID_STACK          4096
#define TASK_PID_PRIORITY       5       // Highest app priority
#define TASK_PID_CORE           1       // Dedicated core

#define TASK_SAFETY_STACK       2048
#define TASK_SAFETY_PRIORITY    6       // Higher than PID
#define TASK_SAFETY_CORE        1

#define TASK_UI_STACK           4096
#define TASK_UI_PRIORITY        3
#define TASK_UI_CORE            0

#define TASK_NETWORK_STACK      8192    // WiFi needs larger stack
#define TASK_NETWORK_PRIORITY   2
#define TASK_NETWORK_CORE       0

#define TASK_LOGGER_STACK       2048
#define TASK_LOGGER_PRIORITY    1       // Lowest priority
#define TASK_LOGGER_CORE        0

// --- Queue Sizes ---
#define QUEUE_TEMP_SIZE         8
#define QUEUE_CMD_SIZE          16
#define QUEUE_FAULT_SIZE        8
