# ESP-Nail v2 PCB Design

## Overview

The ESP-Nail v2 PCB is an optional custom carrier board that replaces point-to-point wiring. It accepts the ESP32 DevKit V1 as a plug-in module and provides:

- Screw terminal connections for all channels
- Onboard NPN SSR driver circuits (1 per channel)
- MAX31855 breakout headers with shared SPI bus
- Rotary encoder header
- OLED display header (I2C)
- Buzzer driver
- Optional ACS712 current sensor footprint
- Optional BME280 environmental sensor header

## Directory Contents

- `schematic_notes.md` - Circuit design notes and component values
- `bom_pcb.csv` - PCB-specific bill of materials
- `kicad/` - KiCad project files (to be created)

## Design Rules

| Parameter         | Value        |
|-------------------|--------------|
| Board Size        | 80 x 60 mm  |
| Layers            | 2            |
| Trace width (sig) | 0.25 mm     |
| Trace width (pwr) | 0.5 mm      |
| Via size          | 0.8/0.4 mm  |
| Copper weight     | 1 oz (35um) |
| Min clearance     | 0.2 mm      |

## Important Notes

- **NO AC mains traces on the PCB.** All AC wiring uses external 16AWG wire.
- The PCB carries only DC signals (3.3V, 5V logic) and SSR control signals.
- SSR load switching is done externally via wire connections to the SSR modules.
- The PCB is designed for JLCPCB / PCBWay fabrication at standard 2-layer pricing.

## ESP32 DevKit V1 Footprint

The ESP32 DevKit V1 (38-pin) plugs into two rows of female pin headers:
- Left row: 19 pins (EN through GPIO23)
- Right row: 19 pins (GND through VIN)
- Mounting: 2.54mm pitch, 25.4mm row spacing

## Connector Pinout

### J1-J4: Channel Connectors (1x6 screw terminal, per channel)
| Pin | Signal      | Description              |
|-----|-------------|--------------------------|
| 1   | TC+         | Thermocouple positive    |
| 2   | TC-         | Thermocouple negative    |
| 3   | SSR_CTRL    | SSR control (5V output)  |
| 4   | SSR_GND     | SSR control ground       |
| 5   | CUR_SENSE   | ACS712 analog out (opt)  |
| 6   | GND         | Signal ground            |

### J5: OLED Display (1x4 pin header)
| Pin | Signal |
|-----|--------|
| 1   | GND    |
| 2   | VCC    |
| 3   | SCL    |
| 4   | SDA    |

### J6: Rotary Encoder (1x5 pin header)
| Pin | Signal |
|-----|--------|
| 1   | GND    |
| 2   | +3.3V  |
| 3   | SW     |
| 4   | DT     |
| 5   | CLK    |

### J7: Power Input (1x3 screw terminal)
| Pin | Signal |
|-----|--------|
| 1   | +5V    |
| 2   | GND    |
| 3   | PE     |
