#pragma once

// ============================================================
// ESP32 E-Nail Controller - Global Configuration
// ============================================================

#include <Arduino.h>

// --- Build Configuration Defaults ---
#ifndef NUM_CHANNELS
#define NUM_CHANNELS 1
#endif

#ifndef MODEL_NAME
#define MODEL_NAME "ESP-Nail"
#endif

#define FW_VERSION "1.0.0"

// --- Pin Assignments (ESP32 DevKit V1) ---

// I2C - OLED Display (SSD1306 0.96")
#define PIN_SDA         21
#define PIN_SCL         22

// SPI - MAX31855 Thermocouple Amplifiers
#define PIN_SPI_MISO    19
#define PIN_SPI_SCK     18
// CS pins per channel
#define PIN_TC_CS_1     5
#define PIN_TC_CS_2     17
#define PIN_TC_CS_3     16
#define PIN_TC_CS_4     4

// SSR Output Pins (one per channel)
#define PIN_SSR_1       25
#define PIN_SSR_2       26
#define PIN_SSR_3       27
#define PIN_SSR_4       14

// Rotary Encoder
#define PIN_ENC_CLK     32
#define PIN_ENC_DT      33
#define PIN_ENC_SW      35

// Buzzer
#define PIN_BUZZER      13

// Status LED (built-in on most ESP32 boards)
#define PIN_STATUS_LED  2

// --- Pin Arrays (indexed by channel) ---
static const uint8_t TC_CS_PINS[]  = { PIN_TC_CS_1, PIN_TC_CS_2, PIN_TC_CS_3, PIN_TC_CS_4 };
static const uint8_t SSR_PINS[]    = { PIN_SSR_1, PIN_SSR_2, PIN_SSR_3, PIN_SSR_4 };

// --- Temperature Limits ---
#define TEMP_MIN_F          0.0f
#define TEMP_MAX_F          999.0f
#define TEMP_DEFAULT_F      710.0f
#define TEMP_OVERSHOOT_F    50.0f   // Safety cutoff above setpoint
#define TEMP_ABS_MAX_F      1050.0f // Absolute maximum - immediate shutdown

// --- PID Defaults ---
#define PID_KP_DEFAULT      8.0f
#define PID_KI_DEFAULT      0.2f
#define PID_KD_DEFAULT      2.0f
#define PID_SAMPLE_MS       250     // PID computation interval
#define PID_OUTPUT_MIN      0.0f
#define PID_OUTPUT_MAX      100.0f

// --- SSR Time-Proportioning ---
#define SSR_PERIOD_MS       1000    // 1 second switching period
#define SSR_MIN_ON_MS       50      // Minimum on-time (avoid flickering)

// --- Safety ---
#define IDLE_TIMEOUT_MIN    60      // Auto-off after 60 minutes idle
#define WATCHDOG_TIMEOUT_S  10      // Hardware watchdog timeout
#define TC_READ_INTERVAL_MS 250     // Thermocouple read interval
#define TC_ERROR_COUNT_MAX  10      // Consecutive errors before fault

// --- Display ---
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_ADDR           0x3C
#define DISPLAY_UPDATE_MS   100     // Display refresh interval

// --- UI ---
#define ENCODER_DEBOUNCE_MS     5
#define BUTTON_DEBOUNCE_MS      50
#define BUTTON_LONG_PRESS_MS    1000
#define TEMP_STEP_NORMAL        5.0f    // Degrees per encoder click
#define TEMP_STEP_FINE          1.0f    // Fine adjustment step

// --- Storage ---
#define NVS_NAMESPACE       "enail"
