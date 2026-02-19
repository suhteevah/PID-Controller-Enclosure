# ESP-Nail v2 Firmware Guide

## Architecture Overview

ESP-Nail v2 uses FreeRTOS on the ESP32's dual-core processor to separate real-time PID control from UI and networking:

```
Core 1 (Real-time):
  TaskPID     (250ms) - Thermocouple read, PID compute, SSR output
  TaskSafety  (100ms) - Fault monitoring, hardware watchdog

Core 0 (Application):
  TaskUI      (100ms) - Encoder input, OLED display rendering
  TaskNetwork  (50ms) - WiFi, BLE, web server, MQTT, OTA
  TaskLogger    (1s)  - Session recording
```

Tasks communicate via FreeRTOS queues (no shared global state):
- `queueTemp` - PID publishes temperature readings for UI/Network
- `queueCommand` - UI/Network sends control commands to PID
- `queueFault` - Safety sends fault events to UI
- `mutexStorage` - Mutex for NVS flash access

## Build System

The project uses PlatformIO with multiple build environments:

```bash
pio run -e single       # Model S (1 channel, OLED, WiFi+BLE)
pio run -e dual         # Model D (2 channels)
pio run -e quad         # Model Q (4 channels)
pio run -e single_color # Model S Pro (1 channel, color TFT, MQTT)
pio run -e minimal      # Lite (1 channel, no WiFi/BLE)
pio run -e test         # Native unit tests
```

### Build Flags

| Flag | Values | Description |
|------|--------|-------------|
| `NUM_CHANNELS` | 1, 2, 4 | Number of XLR outputs |
| `MODEL_NAME` | string | Display name on OLED/web |
| `ENABLE_WIFI` | 0, 1 | WiFi stack |
| `ENABLE_BLE` | 0, 1 | Bluetooth Low Energy |
| `ENABLE_OTA` | 0, 1 | Over-the-air updates |
| `ENABLE_MQTT` | 0, 1 | MQTT publishing |
| `DISPLAY_TYPE_SSD1306` | 0, 1 | SSD1306 OLED driver |
| `DISPLAY_TYPE_ST7789` | 0, 1 | ST7789 color TFT driver |

## Module Structure

```
firmware/v2/src/
├── main.cpp                    # RTOS task creation, startup
├── core/
│   ├── pid.h/cpp               # PID algorithm
│   ├── channel.h/cpp           # Channel state machine
│   ├── safety.h/cpp            # Safety manager
│   └── autotune.h/cpp          # PID auto-tuner (Ziegler-Nichols)
├── drivers/
│   ├── thermocouple.h/cpp      # MAX31855 K-type interface
│   ├── ssr.h/cpp               # SSR time-proportioning driver
│   ├── display_ssd1306.h/cpp   # SSD1306 OLED driver
│   ├── encoder.h/cpp           # Rotary encoder with ISR
│   └── buzzer.h/cpp            # Non-blocking buzzer
├── ui/
│   ├── screen_manager.h/cpp    # Screen state machine + event routing
│   ├── screen_main.h/cpp       # Main temperature display
│   ├── screen_settings.h/cpp   # Settings menu
│   ├── screen_profiles.h/cpp   # Profile management
│   ├── screen_wifi.h/cpp       # WiFi status display
│   └── widgets.h/cpp           # Reusable UI components
├── network/
│   ├── wifi_manager.h/cpp      # WiFi AP/STA management
│   ├── web_server.h/cpp        # Async REST API + WebSocket
│   ├── ble_service.h/cpp       # BLE GATT service
│   ├── mqtt_client.h/cpp       # MQTT with HA auto-discovery
│   ├── ota_updater.h/cpp       # OTA firmware updates
│   └── mdns_service.h/cpp      # mDNS hostname registration
└── data/
    ├── storage.h/cpp           # Versioned NVS settings
    ├── profiles.h/cpp          # Temperature presets
    ├── session_log.h/cpp       # Session recording (LittleFS)
    └── calibration.h/cpp       # Surface temp calibration
```

## PID Controller

The PID algorithm (`core/pid.h`) features:
- **Anti-windup**: Integral term clamped to output limits
- **Derivative-on-measurement**: Avoids setpoint kick
- **Low-pass derivative filter**: Configurable alpha (0.01-1.0)
- **Bumpless transfer**: Reset on enable to prevent integral bump
- **Time-proportioning output**: SSR switches at 1Hz with variable duty cycle

### Auto-Tune

The auto-tuner (`core/autotune.h`) uses relay feedback (Ziegler-Nichols):
1. Applies full power until setpoint is reached
2. Switches between 0% and 100% output at setpoint crossings
3. Measures oscillation period (Tu) and amplitude
4. Computes ultimate gain (Ku) from relay amplitude and process response
5. Applies Z-N tuning rules: Kp = 0.6*Ku, Ki = 1.2*Ku/Tu, Kd = 0.075*Ku*Tu

## Channel State Machine

```
         enable()
  OFF ──────────────→ HEATING
   ↑                    │
   │ temp<150F          │ within 10°F of target
   │                    ↓
COOLDOWN ←── disable() ── HOLDING
                          │ error > 20°F
                          ↓
                        HEATING (re-enters)

  Any active state → FAULT (on TC error or overtemp)
  Any active state → AUTOTUNE (on startAutotune())
```

## Flash Partitions

```
Offset    Size      Name      Purpose
0x9000    20KB      nvs       Settings, profiles, calibration
0xe000    8KB       otadata   OTA boot tracking
0x10000   1.6MB     app0      Firmware slot A
0x1B0000  1.6MB     app1      Firmware slot B (OTA)
0x350000  704KB     spiffs    LittleFS (web UI, session logs)
```

## Memory Usage Estimates

| Component | RAM | Flash |
|-----------|-----|-------|
| ESP32 base + FreeRTOS | ~50KB | ~200KB |
| WiFi + BLE stack | ~80KB | ~400KB |
| Adafruit display + GFX | ~8KB | ~60KB |
| AsyncWebServer | ~10KB | ~50KB |
| ArduinoJson | ~2KB | ~30KB |
| Application code | ~15KB | ~150KB |
| LittleFS (web assets) | - | ~200KB |
| **Total** | **~165KB** | **~1.1MB** |
| Available | ~155KB | ~500KB headroom |

## Adding a New Screen

1. Create `ui/screen_myscreen.h` and `.cpp`
2. Add enum value to `Screen` in `ui/screen_manager.h`
3. Add render case in `ScreenManager::render()`
4. Add event handler in `ScreenManager::handleEvent()`
5. Add navigation entry point (menu item, button combo, etc.)

## Adding a New Sensor

1. Create driver in `drivers/my_sensor.h/cpp`
2. Add initialization in `setup()` (main.cpp)
3. Read sensor in appropriate RTOS task
4. Publish data via queue or direct access
5. Add display element in UI screen
6. Add web API endpoint if needed
