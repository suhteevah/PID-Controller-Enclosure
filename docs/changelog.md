# Changelog

All notable changes to this project will be documented in this file.

Format based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [2.0.0-alpha] - 2026-02-16

### Added
- FreeRTOS dual-core architecture (PID on Core 1, UI/Network on Core 0)
- WiFi support: AP mode (captive portal) + STA mode (home network)
- Web dashboard: responsive PWA with real-time temperature graphing
- WebSocket for live temperature streaming
- REST API for programmatic control (14 endpoints)
- BLE GATT service for direct phone control
- MQTT publishing with Home Assistant auto-discovery
- OTA (Over-the-Air) firmware updates via web UI
- Temperature profiles: 8 presets per channel with named presets
- PID auto-tune (Ziegler-Nichols relay feedback method)
- Surface temperature calibration (per-channel offset)
- Session logging: records duration, peak/avg temp, energy estimate
- Non-blocking buzzer with queued note patterns
- Separated SSR driver with stuck detection
- Abstract display interface (supports SSD1306 + ST7789)
- Modular UI screen system with screen manager
- Widget library for reusable UI components
- Custom PCB design documentation (KiCad + JLCPCB)
- Aluminum enclosure design option
- Snap-fit 3D-printed enclosure (no screws)
- USB-C access port on enclosure
- Unit test suite for PID, channel, and safety modules
- Comprehensive documentation: 10+ guides

### Changed
- Refactored from single-loop Arduino to FreeRTOS multi-task
- Channel state machine now includes AUTOTUNE state
- Storage system versioned (auto-migrate from v1)
- PID derivative filter configurable (was hardcoded)
- Safety manager uses FreeRTOS queue for fault events
- BOM updated with v2 components (transistor buffer, USB-C, etc.)

### Fixed
- Memory leak in thermocouple driver (now uses RAII)
- Blocking delay in buzzer (now non-blocking queue)
- Multi-fault display (shows all faults, not just first)
- Idle timer rounding (shows 1 when < 60s remains, not 0)
- Input validation on all NVS settings loads

## [1.0.0] - 2026-02-16

### Added
- Initial ESP32 PID temperature controller
- SSD1306 OLED display with multi-screen UI
- Rotary encoder input with press/long-press
- 1-4 channel support via 5-pin mini-XLR
- MAX31855 K-type thermocouple interface
- SSR time-proportioning output
- Over-temperature cutoff (1050°F)
- Thermocouple fault detection
- Idle auto-off timer (configurable)
- ESP32 hardware watchdog
- NVS persistent settings
- Parametric OpenSCAD 3D-printable enclosure
- Wood box enclosure design
- Bill of materials and wiring guide
