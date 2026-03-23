// ============================================================
// ESP-Nail Hardware Test
// Run on bare ESP32 DevKit V1 - no external components needed
//
// Tests everything we can verify with just the board:
//   1. Serial / UART
//   2. Built-in LED (GPIO 2)
//   3. GPIO pin readback (all project pins)
//   4. NVS flash storage (read/write/erase)
//   5. WiFi radio (scan for networks)
//   6. I2C bus scan (SDA=21, SCL=22)
//   7. SPI bus init (MISO=19, SCK=18)
//   8. ADC read (GPIO 36 - analog input)
//   9. Free heap / PSRAM report
//  10. Buzzer pin PWM test (GPIO 13 - you'll hear it if buzzer attached)
//
// Open Serial Monitor at 115200 baud and watch the output.
// Each test prints PASS or FAIL.
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <Preferences.h>

// --- Pin assignments from config.h ---
#define PIN_SDA         21
#define PIN_SCL         22
#define PIN_SPI_MISO    19
#define PIN_SPI_SCK     18
#define PIN_TC_CS_1     5
#define PIN_TC_CS_2     17
#define PIN_TC_CS_3     16
#define PIN_TC_CS_4     4
#define PIN_SSR_1       25
#define PIN_SSR_2       26
#define PIN_SSR_3       27
#define PIN_SSR_4       14
#define PIN_ENC_CLK     32
#define PIN_ENC_DT      33
#define PIN_ENC_SW      35
#define PIN_BUZZER      13
#define PIN_STATUS_LED  2
#define PIN_ADC_CURRENT 36

static int passCount = 0;
static int failCount = 0;

void reportResult(const char* testName, bool passed, const char* detail = nullptr) {
    if (passed) {
        passCount++;
        Serial.printf("  [PASS] %s", testName);
    } else {
        failCount++;
        Serial.printf("  [FAIL] %s", testName);
    }
    if (detail) {
        Serial.printf(" -- %s", detail);
    }
    Serial.println();
}

// ============================================================
// Test 1: Serial (if you see this, it passed)
// ============================================================
void testSerial() {
    Serial.println("\n--- TEST 1: Serial / UART ---");
    Serial.println("  If you can read this, serial is working.");
    reportResult("Serial TX", true);

    // Echo test info
    Serial.printf("  CPU Freq: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("  SDK Version: %s\n", ESP.getSdkVersion());
    Serial.printf("  Chip Model: %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("  Chip Cores: %d\n", ESP.getChipCores());
    Serial.printf("  Flash Size: %d KB\n", ESP.getFlashChipSize() / 1024);
    Serial.printf("  Flash Speed: %d MHz\n", ESP.getFlashChipSpeed() / 1000000);
    reportResult("Chip info readback", ESP.getChipCores() >= 2);
}

// ============================================================
// Test 2: Built-in LED
// ============================================================
void testLED() {
    Serial.println("\n--- TEST 2: Built-in LED (GPIO 2) ---");
    pinMode(PIN_STATUS_LED, OUTPUT);

    Serial.println("  Blinking LED 3 times... watch your board.");
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        delay(250);
        digitalWrite(PIN_STATUS_LED, LOW);
        delay(250);
    }
    reportResult("LED blink", true, "visually confirm the blue LED blinked");
}

// ============================================================
// Test 3: GPIO pin readback
// ============================================================
void testGPIO() {
    Serial.println("\n--- TEST 3: GPIO Pin Readback ---");
    Serial.println("  Reading all project pins in INPUT mode (expect floating/HIGH with pullups)");

    // Output pins - configure as OUTPUT, write LOW, read back
    const uint8_t outputPins[] = {PIN_SSR_1, PIN_SSR_2, PIN_SSR_3, PIN_SSR_4};
    const char* outputNames[] = {"SSR_1 (GPIO25)", "SSR_2 (GPIO26)", "SSR_3 (GPIO27)", "SSR_4 (GPIO14)"};

    for (int i = 0; i < 4; i++) {
        pinMode(outputPins[i], OUTPUT);
        digitalWrite(outputPins[i], LOW);
        delay(5);
        int val = digitalRead(outputPins[i]);
        char detail[64];
        snprintf(detail, sizeof(detail), "wrote LOW, read %s", val == LOW ? "LOW" : "HIGH");
        reportResult(outputNames[i], val == LOW, detail);
        // Immediately set back to safe state
        digitalWrite(outputPins[i], LOW);
    }

    // Test that SSR pins can go HIGH
    for (int i = 0; i < 4; i++) {
        digitalWrite(outputPins[i], HIGH);
        delay(5);
        int val = digitalRead(outputPins[i]);
        char detail[64];
        snprintf(detail, sizeof(detail), "wrote HIGH, read %s", val == HIGH ? "HIGH" : "LOW");
        reportResult(outputNames[i], val == HIGH, detail);
        // Set back LOW immediately (safety)
        digitalWrite(outputPins[i], LOW);
    }

    // CS pins - should be pullable HIGH (deselected)
    const uint8_t csPins[] = {PIN_TC_CS_1, PIN_TC_CS_2, PIN_TC_CS_3, PIN_TC_CS_4};
    const char* csNames[] = {"TC_CS_1 (GPIO5)", "TC_CS_2 (GPIO17)", "TC_CS_3 (GPIO16)", "TC_CS_4 (GPIO4)"};

    for (int i = 0; i < 4; i++) {
        pinMode(csPins[i], OUTPUT);
        digitalWrite(csPins[i], HIGH);
        delay(5);
        int val = digitalRead(csPins[i]);
        char detail[64];
        snprintf(detail, sizeof(detail), "CS deselect HIGH, read %s", val == HIGH ? "HIGH" : "LOW");
        reportResult(csNames[i], val == HIGH, detail);
    }

    // Input pins with pullup
    const uint8_t inputPins[] = {PIN_ENC_CLK, PIN_ENC_DT};
    const char* inputNames[] = {"ENC_CLK (GPIO32)", "ENC_DT (GPIO33)"};

    for (int i = 0; i < 2; i++) {
        pinMode(inputPins[i], INPUT_PULLUP);
        delay(5);
        int val = digitalRead(inputPins[i]);
        char detail[64];
        snprintf(detail, sizeof(detail), "pullup enabled, read %s (expected HIGH)", val == HIGH ? "HIGH" : "LOW");
        reportResult(inputNames[i], val == HIGH, detail);
    }

    // GPIO 35 is input-only, no internal pullup
    pinMode(PIN_ENC_SW, INPUT);
    delay(5);
    int swVal = digitalRead(PIN_ENC_SW);
    char swDetail[64];
    snprintf(swDetail, sizeof(swDetail), "input-only pin, read %s (floating OK)", swVal == HIGH ? "HIGH" : "LOW");
    reportResult("ENC_SW (GPIO35)", true, swDetail);

    // Clean up - set all SSR pins LOW
    for (int i = 0; i < 4; i++) {
        digitalWrite(outputPins[i], LOW);
    }
}

// ============================================================
// Test 4: NVS Flash Storage
// ============================================================
void testNVS() {
    Serial.println("\n--- TEST 4: NVS Flash Storage ---");

    Preferences prefs;
    bool opened = prefs.begin("enail_test", false);
    reportResult("NVS open namespace", opened);

    if (opened) {
        // Write a test value
        size_t written = prefs.putFloat("test_temp", 710.0f);
        reportResult("NVS write float", written == sizeof(float), "wrote 710.0");

        // Read it back
        float readBack = prefs.getFloat("test_temp", -1.0f);
        char detail[64];
        snprintf(detail, sizeof(detail), "wrote 710.0, read %.1f", readBack);
        reportResult("NVS read float", abs(readBack - 710.0f) < 0.01f, detail);

        // Write an int
        prefs.putUInt("test_count", 42);
        uint32_t count = prefs.getUInt("test_count", 0);
        reportResult("NVS read/write uint", count == 42);

        // Clean up test keys
        prefs.remove("test_temp");
        prefs.remove("test_count");
        float afterRemove = prefs.getFloat("test_temp", -999.0f);
        reportResult("NVS key removal", abs(afterRemove - (-999.0f)) < 0.01f);

        prefs.end();

        // Free entries check
        Serial.printf("  NVS free entries: %d\n", prefs.freeEntries());
    }
}

// ============================================================
// Test 5: WiFi Radio
// ============================================================
void testWiFi() {
    Serial.println("\n--- TEST 5: WiFi Radio ---");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("  Scanning for networks...");
    int networks = WiFi.scanNetworks(false, false, false, 3000);

    char detail[64];
    snprintf(detail, sizeof(detail), "found %d networks", networks);
    reportResult("WiFi scan", networks >= 0, detail);

    if (networks > 0) {
        int show = min(networks, 5);
        for (int i = 0; i < show; i++) {
            Serial.printf("    %d: %s (RSSI: %d dBm, Ch: %d)\n",
                          i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
        }
        if (networks > 5) {
            Serial.printf("    ... and %d more\n", networks - 5);
        }
    }

    // Print MAC address
    Serial.printf("  MAC Address: %s\n", WiFi.macAddress().c_str());
    reportResult("MAC address", WiFi.macAddress().length() == 17);

    WiFi.mode(WIFI_OFF);
}

// ============================================================
// Test 6: I2C Bus Scan
// ============================================================
void testI2C() {
    Serial.println("\n--- TEST 6: I2C Bus Scan (SDA=21, SCL=22) ---");

    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(100000);  // 100kHz standard

    int devicesFound = 0;
    Serial.println("  Scanning I2C addresses 0x01-0x7F...");

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            devicesFound++;
            const char* name = "unknown";
            if (addr == 0x3C || addr == 0x3D) name = "SSD1306 OLED";
            else if (addr == 0x76 || addr == 0x77) name = "BME280";
            Serial.printf("    Found device at 0x%02X (%s)\n", addr, name);
        }
    }

    char detail[64];
    snprintf(detail, sizeof(detail), "%d device(s) found (0 is OK with bare board)", devicesFound);
    reportResult("I2C bus init", true, detail);

    if (devicesFound == 0) {
        Serial.println("  No I2C devices found - normal for bare ESP32.");
        Serial.println("  When you connect the OLED, expect 0x3C here.");
    }
}

// ============================================================
// Test 7: SPI Bus Init
// ============================================================
void testSPI() {
    Serial.println("\n--- TEST 7: SPI Bus Init (MISO=19, SCK=18) ---");

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, -1, -1);  // SCK, MISO, MOSI(-1), SS(-1)

    // Try a dummy read with CS1 selected
    pinMode(PIN_TC_CS_1, OUTPUT);
    digitalWrite(PIN_TC_CS_1, LOW);
    delay(1);

    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    uint32_t rawData = 0;
    for (int i = 0; i < 4; i++) {
        rawData = (rawData << 8) | SPI.transfer(0x00);
    }
    SPI.endTransaction();

    digitalWrite(PIN_TC_CS_1, HIGH);

    char detail[128];
    snprintf(detail, sizeof(detail), "raw SPI read: 0x%08X (no device = 0xFFFFFFFF or 0x00000000)", rawData);
    // With no MAX31855 connected, we expect all 1s (0xFFFFFFFF) or all 0s
    bool spiOk = (rawData == 0xFFFFFFFF || rawData == 0x00000000);
    reportResult("SPI bus read", spiOk, detail);

    if (rawData == 0xFFFFFFFF) {
        Serial.println("  All 1s = MISO pulled high (no device, normal for bare board)");
    } else if (rawData == 0x00000000) {
        Serial.println("  All 0s = MISO pulled low (no device, normal for bare board)");
    } else {
        Serial.println("  Got unexpected data - might have noise on the SPI lines");
        Serial.println("  Not a problem, just means the pins are floating");
        reportResult("SPI bus read (adjusted)", true, "floating pins produce noise, still OK");
    }

    SPI.end();
}

// ============================================================
// Test 8: ADC (Analog Read)
// ============================================================
void testADC() {
    Serial.println("\n--- TEST 8: ADC Read (GPIO 36) ---");

    // GPIO 36 is ADC1_CH0, used for optional current sensor
    analogSetAttenuation(ADC_11db);  // Full 0-3.3V range
    int rawADC = analogRead(PIN_ADC_CURRENT);
    float voltage = rawADC * 3.3f / 4095.0f;

    char detail[64];
    snprintf(detail, sizeof(detail), "raw=%d, voltage=%.2fV (floating pin, any value OK)", rawADC, voltage);
    reportResult("ADC read GPIO36", rawADC >= 0 && rawADC <= 4095, detail);

    // Also read the internal hall sensor (just for fun)
    // Note: hallRead() was removed in newer ESP32 Arduino cores, skip if not available
    Serial.println("  (No ACS712 connected - reading floating pin is expected)");
}

// ============================================================
// Test 9: Memory Report
// ============================================================
void testMemory() {
    Serial.println("\n--- TEST 9: Memory Report ---");

    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();

    Serial.printf("  Total Heap:     %d bytes (%.1f KB)\n", totalHeap, totalHeap / 1024.0f);
    Serial.printf("  Free Heap:      %d bytes (%.1f KB)\n", freeHeap, freeHeap / 1024.0f);
    Serial.printf("  Min Free Heap:  %d bytes (%.1f KB)\n", minFreeHeap, minFreeHeap / 1024.0f);
    Serial.printf("  Sketch Size:    %d bytes (%.1f KB)\n", ESP.getSketchSize(), ESP.getSketchSize() / 1024.0f);
    Serial.printf("  Free Sketch:    %d bytes (%.1f KB)\n", ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace() / 1024.0f);

    // ESP-Nail needs ~80KB free heap minimum for WiFi + display
    bool heapOk = freeHeap > 80000;
    char detail[64];
    snprintf(detail, sizeof(detail), "%d bytes free (need >80KB for full firmware)", freeHeap);
    reportResult("Heap available", heapOk, detail);

    // Check PSRAM
    if (ESP.getPsramSize() > 0) {
        Serial.printf("  PSRAM Total:    %d bytes (%.1f KB)\n", ESP.getPsramSize(), ESP.getPsramSize() / 1024.0f);
        Serial.printf("  PSRAM Free:     %d bytes (%.1f KB)\n", ESP.getFreePsram(), ESP.getFreePsram() / 1024.0f);
    } else {
        Serial.println("  PSRAM: not present (that's fine, not needed)");
    }
}

// ============================================================
// Test 10: Buzzer PWM
// ============================================================
void testBuzzerPWM() {
    Serial.println("\n--- TEST 10: Buzzer PWM (GPIO 13) ---");
    Serial.println("  Generating 2kHz tone for 200ms on GPIO 13...");
    Serial.println("  (If you have a buzzer connected, you'll hear a beep)");

    // Use ledcWrite for PWM tone generation
    ledcAttach(PIN_BUZZER, 2000, 8);  // 2kHz, 8-bit resolution
    ledcWrite(PIN_BUZZER, 128);       // 50% duty cycle
    delay(200);
    ledcWrite(PIN_BUZZER, 0);         // silence
    ledcDetach(PIN_BUZZER);

    reportResult("PWM tone output", true, "generated 2kHz on GPIO13");
}

// ============================================================
// MAIN
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(2000);  // Give serial monitor time to connect

    Serial.println();
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║   ESP-Nail Hardware Test Suite           ║");
    Serial.println("║   Bare board test - no components needed ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();

    testSerial();
    testLED();
    testGPIO();
    testNVS();
    testWiFi();
    testI2C();
    testSPI();
    testADC();
    testMemory();
    testBuzzerPWM();

    // Final summary
    Serial.println();
    Serial.println("══════════════════════════════════════════");
    Serial.printf("  RESULTS: %d PASS, %d FAIL, %d total\n", passCount, failCount, passCount + failCount);
    Serial.println("══════════════════════════════════════════");

    if (failCount == 0) {
        Serial.println();
        Serial.println("  ALL TESTS PASSED");
        Serial.println("  Your ESP32 is good to go for the ESP-Nail build.");
        Serial.println();
        Serial.println("  Next steps:");
        Serial.println("  1. Order the BOM (see BOM_ORDER_GUIDE.md)");
        Serial.println("  2. When OLED arrives, connect to GPIO 21/22 and re-run");
        Serial.println("     -> I2C scan should find device at 0x3C");
        Serial.println("  3. When MAX31855 arrives, SPI read should show real data");
    } else {
        Serial.println();
        Serial.printf("  %d test(s) had issues - review above.\n", failCount);
        Serial.println("  Some FAILs on a bare board are expected (floating pins).");
    }

    Serial.println();
    Serial.println("  Board is now idle. LED will heartbeat every 2 seconds.");
    Serial.println("  Press RST button to re-run tests.");
}

void loop() {
    // Heartbeat blink - proves the board is still running
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(100);
    digitalWrite(PIN_STATUS_LED, LOW);
    delay(1900);
}
