# ESP-Nail v2 Brainstorm

## Competitive Landscape (2025-2026)

### What Exists
| Product | Price | Key Feature | Weakness |
|---------|-------|------------|----------|
| Fancier (DiscountEnails) | $70-90 | Cheap, reliable, compact | Dumb PID, no app, no profiles |
| Perfect Dab Tech | ~$200 | WiFi + Touchscreen | Expensive, closed-source |
| Disorderly Conduction | $150-250 | 3D-printed, dual output, aesthetic | No app, no profiles |
| Errlectric QUAD | $300+ | 4 independent outputs | Expensive, no smart features |
| MaxVapor + HomeKit | $100+ | HomeKit integration | Niche, requires Homebridge |
| Octo-Nail (OSS) | DIY | ESP32, web UI, OTA | Early prototype, incomplete |
| **ESP-Nail v1 (us)** | DIY/$85-160 | Multi-channel, OLED, open-source | No WiFi, no app, no profiles |

### Market Gap
Nobody is offering: **open-source + WiFi/BLE app + temperature profiles + multi-channel + affordable**. That's our v2.

---

## v2 Feature Tiers

### Tier 1: Must-Have (Ship Blockers)

#### 1. WiFi + BLE Companion App
- **BLE for direct control**: phone connects directly, no router needed
- **WiFi for dashboard**: web UI served from ESP32 (like CleverCoffee/PIDKiln)
- **Real-time temp graph**: live-updating chart in browser
- **Remote on/off + temp adjust**: control from couch
- **mDNS discovery**: `http://espnail.local` - no IP address hunting
- PWA (Progressive Web App) - installable on phone home screen, no app store needed

#### 2. Temperature Profiles / Presets
- **Save up to 8 presets** per channel (e.g., "Low Temp 500°F", "Medium 620°F", "Hot 710°F")
- **Named presets**: user-customizable names
- **Quick-switch**: cycle presets with encoder click from main screen
- **Per-coil-size profiles**: store PID tunings specific to 16mm, 20mm, 25mm, 30mm coils
- **Ramp/Soak mode**: heat to X°F over Y minutes, hold for Z minutes (for seasoning, cleaning)

#### 3. OTA Firmware Updates
- Upload new firmware via web interface (no USB cable needed)
- Firmware version display + check-for-update notification
- Dual-partition OTA with automatic rollback on failed update
- Settings preserved across updates

#### 4. Auto-Tune PID
- One-button auto-tune using relay feedback method (Ziegler-Nichols)
- Runs oscillation test, calculates Kp/Ki/Kd automatically
- Works per-channel and per-coil-size
- Saves tuned values with profile

#### 5. Fix v1 Bugs
- Memory leak in thermocouple (use stack allocation or RAII)
- Non-blocking buzzer (timer-based, not delay())
- SSR stuck detection (actually implement it)
- Multi-fault display (show all faulted channels, not just first)
- Input validation on all settings loads
- Bounds checking throughout

---

### Tier 2: Strong Differentiators

#### 6. Session Logging & Analytics
- **Session recording**: start time, duration, peak temp, avg temp, energy estimate
- **Session history**: last 50 sessions stored in SPIFFS/LittleFS
- **Export via web UI**: download CSV of session data
- **Usage statistics**: total hours, total sessions, coil age tracking
- **Coil lifecycle alerts**: "Coil on CH1 has 500+ hours - consider replacement"

#### 7. Surface Temperature Calibration
- **Offset calibration**: measure actual surface temp with IR gun, enter offset
- **Per-coil calibration**: different offsets for different coil types
- **Display shows calibrated temp**: "Surface: 620°F (Coil: 685°F)"
- Major differentiator - Micro High Five charges premium for this

#### 8. ESPHome / Home Assistant Integration
- **Native ESPHome YAML config** option (alternative firmware)
- **MQTT publishing**: temp, state, session data → any broker
- **Home Assistant auto-discovery**: shows up automatically
- **Automations**: "turn off e-nail when I leave home" via HA
- **Voice control**: "Hey Google, set the e-nail to 650"

#### 9. Custom PCB Design
- Move from ESP32 DevKit + breakout boards to single integrated PCB
- **Benefits**: smaller, more reliable, lower BOM cost, cleaner wiring
- **Include**: ESP32-WROOM-32, MAX31855(s), SSR driver circuits, USB-C, power regulation
- **KiCad open-source**: share gerbers for community manufacturing
- **JLCPCB assembly-ready**: pre-picked components from JLCPCB parts library
- Cost target: $8-15 per board (assembled, qty 10)

#### 10. RTOS Refactor
- Move from Arduino loop() to FreeRTOS tasks
- **Task 1**: PID computation + SSR control (highest priority, core 1)
- **Task 2**: Display update + UI input (core 0)
- **Task 3**: WiFi/BLE communication (core 0)
- **Task 4**: Safety watchdog (core 1, highest priority)
- Event queues between tasks (no global state)
- Eliminates blocking delays, improves responsiveness

---

### Tier 3: Premium / Future

#### 11. Color Display Upgrade
- **Option A**: 1.3" IPS TFT (ST7789, 240x240) - vivid color, fast refresh
- **Option B**: 1.54" OLED (SSD1351, 128x128) - true blacks, OLED aesthetic
- **Custom graphics**: temperature gauge, animated heating indicator, color-coded states
- **Themeable UI**: dark mode, neon mode, minimal mode
- Keep SSD1306 as budget option (v1 compatible)

#### 12. Induction Heating Module
- Replace resistive coil with induction heating
- **Benefits**: faster heat-up, more even distribution, longer lifespan
- **Challenges**: more complex power electronics, EMI, higher cost
- Phase 2 research item - significant R&D

#### 13. Group/Party Mode
- **Sync multiple ESP-Nail units** via ESP-NOW (mesh networking)
- One "master" controller, others follow
- **Shared session**: all units heat to same temp simultaneously
- **Individual override**: any unit can break away from group
- Cool for events and social settings

#### 14. Power Monitoring
- **ACS712 current sensor** on AC line
- Real-time wattage display
- Energy consumption per session (watt-hours)
- Coil health diagnostics (abnormal current = degraded coil)
- **Cost**: ~$2 per channel

#### 15. Ambient Sensor Suite
- **BME280**: ambient temperature, humidity, barometric pressure
- Display ambient conditions on info screen
- Compensate PID behavior based on ambient temp
- Data logged with sessions

---

## Enclosure v2 Ideas

### 3D-Printed Improvements
- **Snap-fit lid** (no screws) for easier access
- **Integrated cable management** clips inside
- **Light pipe** for status LED (visible through case)
- **Rubber feet** recesses in base
- **USB-C access port** on side for firmware updates
- **Modular front panel**: swap between OLED sizes or add display upgrades
- **Stackable design**: v2 units can physically stack for multi-unit setups

### Wood Box v2
- **Live-edge walnut** option (premium artisan tier)
- **Resin inlay** with LED underglow (RGB status indication through resin)
- **Sliding dovetail lid** instead of magnetic (more premium feel)
- **Leather pad** on bottom
- **Brass or copper accent hardware**
- **Qi wireless charging pad** built into lid (for phone while session is running)
- **Accessory drawer**: pull-out tray for tools/accessories

### New Enclosure: Aluminum Milled
- **6061 aluminum** CNC-milled enclosure
- Anodized black, silver, or custom color
- Excellent heat dissipation (no separate heatsink needed)
- Premium tier: $40-60 enclosure cost
- Target market: people who buy Disorderly Conduction

---

## Revenue Model v2

### Products
| SKU | Description | Target Price | Margin |
|-----|-------------|-------------|--------|
| ESP-Nail S v2 (3D) | Single channel, 3D printed, WiFi | $99 | ~60% |
| ESP-Nail D v2 (3D) | Dual channel, 3D printed, WiFi | $149 | ~55% |
| ESP-Nail Q v2 (3D) | Quad channel, 3D printed, WiFi | $229 | ~50% |
| ESP-Nail S v2 (Wood) | Single channel, walnut box | $179 | ~55% |
| ESP-Nail D v2 (Wood) | Dual channel, walnut box | $249 | ~50% |
| ESP-Nail Q v2 (Wood) | Quad channel, walnut box | $349 | ~45% |
| ESP-Nail S v2 (Aluminum) | Single channel, anodized aluminum | $199 | ~50% |
| PCB Kit | Bare PCB + components, no enclosure | $49 | ~65% |
| STL Files Only | Digital download, print yourself | $15 | ~95% |
| Coil Bundle | Compatible coil + XLR cable | $35-65 | ~40% |

### Recurring / Add-on Revenue
- **Premium presets pack**: curated temperature profiles ($5 digital)
- **Custom wood species**: cherry, maple, bamboo ($20-40 upcharge)
- **Custom laser engraving**: name/logo on lid ($10-15)
- **Extended warranty**: 2-year ($15)
- **Coil subscription**: replacement coil every 6 months ($25/shipment)

### Volume Targets
| Phase | Timeline | Units/Month | Revenue/Month |
|-------|----------|-------------|---------------|
| Launch | Month 1-3 | 10-20 | $1,500-3,000 |
| Growth | Month 4-8 | 30-50 | $4,500-8,000 |
| Scale | Month 9-12 | 80-150 | $12,000-25,000 |

---

## Architecture: v2 Firmware Refactor

### Module Map
```
firmware/
├── platformio.ini
├── include/
│   ├── config.h           # Pin defs, constants
│   └── version.h          # Semantic versioning
├── src/
│   ├── main.cpp           # RTOS task setup only
│   ├── core/
│   │   ├── pid.cpp/h          # PID algorithm (unchanged, proven)
│   │   ├── channel.cpp/h      # Channel state machine
│   │   ├── safety.cpp/h       # Safety manager
│   │   └── autotune.cpp/h     # PID auto-tuner
│   ├── drivers/
│   │   ├── thermocouple.cpp/h # MAX31855 (RAII, no leaks)
│   │   ├── ssr.cpp/h          # SSR driver with stuck detection
│   │   ├── display_ssd1306.cpp/h  # SSD1306 OLED
│   │   ├── display_st7789.cpp/h   # ST7789 color TFT (optional)
│   │   ├── encoder.cpp/h      # Rotary encoder
│   │   └── buzzer.cpp/h       # Non-blocking buzzer
│   ├── ui/
│   │   ├── screen_manager.cpp/h   # Screen state machine
│   │   ├── screen_main.cpp/h      # Main temp display
│   │   ├── screen_settings.cpp/h  # Settings menu
│   │   ├── screen_profiles.cpp/h  # Profile management
│   │   ├── screen_wifi.cpp/h      # WiFi setup
│   │   └── widgets.cpp/h          # Reusable UI components
│   ├── network/
│   │   ├── wifi_manager.cpp/h     # WiFi AP/STA management
│   │   ├── web_server.cpp/h       # Async web server
│   │   ├── ble_service.cpp/h      # BLE GATT service
│   │   ├── mqtt_client.cpp/h      # MQTT publishing
│   │   ├── ota_updater.cpp/h      # OTA firmware update
│   │   └── mdns_service.cpp/h     # mDNS discovery
│   ├── data/
│   │   ├── storage.cpp/h          # NVS settings (versioned)
│   │   ├── profiles.cpp/h         # Temperature presets
│   │   ├── session_log.cpp/h      # Session recording
│   │   └── calibration.cpp/h      # Surface temp calibration
│   └── web/                       # Embedded web UI (gzipped)
│       ├── index.html
│       ├── app.js
│       └── style.css
├── data/                          # SPIFFS/LittleFS data partition
│   └── www/                       # Web UI assets
└── test/                          # Unit tests
    ├── test_pid.cpp
    ├── test_channel.cpp
    └── test_safety.cpp
```

### FreeRTOS Task Layout
```
Core 0 (Application):
  ├── TaskUI        (10ms loop) - encoder, display, menus
  ├── TaskNetwork   (50ms loop) - WiFi, BLE, web server, MQTT
  └── TaskLogger    (1s loop)   - session recording, analytics

Core 1 (Real-time):
  ├── TaskPID       (250ms loop) - thermocouple read, PID compute, SSR output
  └── TaskSafety    (100ms loop) - fault monitoring, watchdog feed

Inter-task communication:
  ├── xQueueTemperature  - PID → UI/Network (current temps)
  ├── xQueueCommand      - UI/Network → PID (setpoint changes, on/off)
  ├── xQueueFault        - Safety → UI (fault notifications)
  └── xSemaphoreStorage  - Mutex for NVS access
```

---

## v2 Web Dashboard Mockup

```
┌─────────────────────────────────────────────────────────┐
│  ESP-Nail v2                      [WiFi: Connected] ⚡  │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌─── Channel 1 ─────────────────────────────────────┐  │
│  │  🔥 HOLDING at 710°F          Target: 710°F       │  │
│  │  ████████████████████████████████░░░░ 98%          │  │
│  │  [ON/OFF]  [Set: 710▲▼]  [Profile: Low Temp ▼]   │  │
│  │                                                    │  │
│  │  ╭────────────────────────────────────────╮        │  │
│  │  │  720°F ─┐                              │        │  │
│  │  │         └──────────────────────── 710°F│        │  │
│  │  │  700°F                                 │        │  │
│  │  │  0:00        5:00       10:00    15:00 │        │  │
│  │  ╰────────────────────────────────────────╯        │  │
│  └────────────────────────────────────────────────────┘  │
│                                                         │
│  Session: 14m 32s  |  Energy: ~12 Wh  |  Idle: 45m    │
│                                                         │
│  [Profiles]  [Settings]  [Session Log]  [Update FW]    │
└─────────────────────────────────────────────────────────┘
```

---

## Implementation Priority (Phased Rollout)

### Phase 1: v2.0-alpha (4-6 weeks)
- [ ] Fix all v1 bugs (memory leak, blocking buzzer, multi-fault display)
- [ ] RTOS refactor (tasks, queues, no global state)
- [ ] WiFi AP mode + basic web dashboard (temp display, on/off)
- [ ] OTA firmware update via web UI
- [ ] Temperature presets (8 per channel, stored in NVS)
- [ ] Non-blocking everything

### Phase 2: v2.0-beta (4-6 weeks)
- [ ] BLE GATT service for phone control
- [ ] PWA web app (installable, responsive)
- [ ] Real-time temperature graphing (web)
- [ ] PID auto-tune
- [ ] Surface temperature calibration
- [ ] Session logging to SPIFFS
- [ ] mDNS discovery

### Phase 3: v2.0-release (2-4 weeks)
- [ ] MQTT + Home Assistant auto-discovery
- [ ] Custom PCB design (KiCad, JLCPCB-ready)
- [ ] Color display driver option (ST7789)
- [ ] Unit test suite
- [ ] Enclosure v2 (snap-fit, USB-C port, modular front)
- [ ] Documentation + build guide updates

### Phase 4: v2.x (ongoing)
- [ ] ESP-NOW group/party mode
- [ ] Power monitoring (ACS712)
- [ ] Induction heating research
- [ ] Aluminum enclosure option
- [ ] Community preset marketplace

---

## Key Differentiators vs Competition

| Feature | Fancier | Perfect Dab | Disorderly | **ESP-Nail v2** |
|---------|---------|-------------|------------|-----------------|
| Price (single) | $70 | ~$200 | $150 | **$99** |
| WiFi Control | No | Yes | No | **Yes** |
| BLE Phone App | No | No | No | **Yes** |
| Temp Profiles | No | Unknown | No | **Yes (8/ch)** |
| Auto-Tune PID | No | No | No | **Yes** |
| Surface Calibration | No | No | No | **Yes** |
| Multi-Channel | No | No | Dual | **1/2/4** |
| OTA Updates | No | Unknown | No | **Yes** |
| Home Assistant | No | No | No | **Yes** |
| Session Logging | No | No | No | **Yes** |
| Open Source | No | No | No | **Yes** |
| 3D Printable | No | No | Yes | **Yes** |
| Wood Enclosure | No | No | No | **Yes** |
| Custom PCB | N/A | Yes | No | **Yes (v2)** |

**Tagline**: *"The only open-source e-nail with WiFi, profiles, and auto-tune. Starting at $99."*
