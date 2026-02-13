# ESP-Nail: Open-Source ESP32 E-Nail Controller

## Overview

**ESP-Nail** is an open-source, ESP32-powered electronic nail (e-nail) temperature controller. It replaces off-the-shelf PID controller boxes with a custom microcontroller solution featuring an OLED display, rotary encoder interface, and support for **1 to 4 independent heating channels** via industry-standard 5-pin mini-XLR connectors.

The project provides both **3D-printable enclosure files** (parametric OpenSCAD) and a **premium wood box design** for builders who want to sell finished units or create personal builds.

## Product Line

| Model | Channels | 5-Pin XLR | Use Case |
|-------|----------|-----------|----------|
| **ESP-Nail S** | 1 | Single | Personal use, entry-level |
| **ESP-Nail D** | 2 | Dual | Shared sessions, two rigs |
| **ESP-Nail Q** | 4 | Quad | Multi-station, commercial |

## Features

- **ESP32 microcontroller** with PID temperature control algorithm
- **0.96" OLED display** (SSD1306) with clear temperature readout
- **Rotary encoder** for intuitive menu navigation and temperature adjustment
- **1-4 independent channels** with individual PID loops and thermocouple inputs
- **5-pin mini-XLR** connectors (industry-standard pinout, Fancier-compatible)
- **K-type thermocouple** via MAX31855 amplifier with fault detection
- **SSR time-proportioning** output for smooth AC power control
- **Safety systems**: over-temperature cutoff, idle auto-off, hardware watchdog, thermocouple fault detection
- **Persistent settings** stored in ESP32 flash (NVS)
- **Parametric 3D-printable enclosure** (OpenSCAD) for all model variants
- **Premium wood box** option for upcharge sales

## Specifications

| Parameter | Value |
|-----------|-------|
| Input Voltage | AC 110-240V, 50/60Hz |
| Temperature Range | Room temp to 999°F |
| Temperature Stability | ~3-5°F (PID controlled) |
| Thermocouple | K-type via MAX31855 |
| Coil Compatibility | 10mm, 16mm, 20mm, 25mm, 30mm barrel |
| Display | 128x64 OLED (SSD1306) |
| Interface | Rotary encoder + push button |
| Connector | 5-pin mini-XLR (Neutrik NC5FD-L-1) |
| Idle Auto-Off | Configurable, default 60 minutes |
| Firmware | ESP32 Arduino (PlatformIO) |

## 5-Pin Mini-XLR Pinout

```
Pin 1: Thermocouple (-)     Pin 2: Thermocouple (+)
Pin 3: Ground / Shield
Pin 4: Heater Load (+)      Pin 5: Heater Load (+)
```

Compatible with Fancier, Auber, and most standard e-nail coils.

## Project Structure

```
├── firmware/                    # ESP32 PlatformIO project
│   ├── platformio.ini           # Build config (single/dual/quad targets)
│   ├── include/config.h         # Pin definitions and constants
│   └── src/
│       ├── main.cpp             # Application entry point
│       ├── pid_controller.*     # PID algorithm with anti-windup
│       ├── thermocouple.*       # MAX31855 K-type interface
│       ├── channel.*            # Per-channel state machine
│       ├── display.*            # OLED UI rendering
│       ├── safety.*             # Safety manager (watchdog, faults)
│       ├── storage.*            # NVS persistent settings
│       └── ui.*                 # Rotary encoder input handling
├── enclosure/
│   ├── 3d-printable/            # Parametric OpenSCAD design
│   │   └── esp_nail_enclosure.scad
│   └── wood-box/                # Premium wood enclosure plans
│       └── wood_box_design.md
├── hardware/
│   ├── BOM.csv                  # Bill of materials (all models)
│   └── wiring/
│       └── wiring_guide.md      # Complete wiring instructions
├── cad/                         # Legacy CAD files (original project)
├── docs/                        # Documentation
└── images/                      # Reference images and schematics
```

## Quick Start

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- ESP32 DevKit V1 board
- Components from [BOM](../hardware/BOM.csv)

### Build & Flash

```bash
# Clone the repo
git clone https://github.com/suhteevah/PID-Controller-Enclosure.git
cd PID-Controller-Enclosure/firmware

# Build for single channel (default)
pio run -e single

# Build for dual channel
pio run -e dual

# Build for quad channel
pio run -e quad

# Flash to ESP32
pio run -e single -t upload

# Monitor serial output
pio device monitor
```

### 3D Print the Enclosure

1. Open `enclosure/3d-printable/esp_nail_enclosure.scad` in [OpenSCAD](https://openscad.org/)
2. Set `num_channels` to 1, 2, or 4
3. Render and export STL
4. Print in **PETG** (recommended for heat resistance) at 0.2mm layer height

## User Interface

### Controls

| Action | Function |
|--------|----------|
| **Rotate** encoder | Adjust temperature (main) / Navigate menus |
| **Short press** | Toggle channel on/off (main) / Confirm selection |
| **Long press** (>1s) | Enter settings menu / Toggle fine adjust mode |

### Menu Structure

```
Main Screen (temperature display)
├── Short Press → Toggle channel ON/OFF
├── Rotate → Adjust temperature / Select channel
└── Long Press → Settings
    ├── PID Tuning (Kp, Ki, Kd per channel)
    ├── Idle Timeout (0-120 minutes)
    ├── System Info
    ├── Factory Reset
    └── Back
```

## Safety Features

1. **Over-temperature cutoff**: Immediate shutdown at 1050°F absolute maximum
2. **Thermocouple fault detection**: Shuts down channel after 10 consecutive read errors (open circuit, short to GND/VCC)
3. **Idle auto-off**: Configurable timer (default 60 min) with 5-minute warning beep
4. **Hardware watchdog**: ESP32 watchdog timer restarts system if firmware hangs
5. **Zero-cross SSR switching**: Minimizes electrical noise and extends SSR life
6. **Fused AC input**: 10A fuse protects against overcurrent

## Enclosure Options

### 3D-Printable (DIY / Budget)

- Parametric OpenSCAD design adapts to 1/2/4 channels
- PETG filament recommended for heat resistance
- Ventilation slots for passive cooling
- Estimated material cost: $5-15

### Premium Wood Box (Upcharge)

- CNC-milled hardwood (walnut, cherry, maple, bamboo)
- Magnetic lid closure
- Integrated aluminum heatsink plate
- Laser-engraved branding
- See [wood_box_design.md](../enclosure/wood-box/wood_box_design.md) for full plans

## Pricing Guide (For Sellers)

| Model | 3D-Print Kit | Wood Box Kit | Wood Box Assembled |
|-------|-------------|-------------|-------------------|
| S (1ch) | $15 | $45 | $85 |
| D (2ch) | $20 | $55 | $110 |
| Q (4ch) | $30 | $75 | $160 |

*Electronics BOM sold separately or as bundles. See [BOM.csv](../hardware/BOM.csv).*

## Contributing

This is an open-source project under the MIT License. Contributions welcome:

- Submit pull requests for firmware improvements
- Share enclosure remixes and modifications
- Report issues and suggest features
- Share your build photos

## License

MIT License - Copyright 2025 Swoop-r, suhteevah

Free to use, modify, and distribute with attribution.

---

**Project Maintainers:** Swoop-r, suhteevah
