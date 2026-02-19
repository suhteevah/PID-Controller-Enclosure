# ESP-Nail v2 Wiring Guide

## Overview

This guide covers wiring for all ESP-Nail v2 models (S/D/Q). Each channel uses identical wiring — simply repeat the per-channel section for multi-channel builds.

## Safety Warnings

- **AC MAINS VOLTAGE** is present inside this enclosure. Exercise extreme caution.
- Always disconnect AC power before working inside the enclosure.
- Use properly rated wire gauges (16AWG minimum for AC/load paths).
- All solder joints on AC paths must be insulated with heat shrink tubing.
- The protective earth (PE) ground MUST be connected to the chassis and XLR shells.

## Pin Assignment Summary

| ESP32 GPIO | Function             | Wire Color (suggested) |
|------------|----------------------|------------------------|
| GPIO 21    | I2C SDA (OLED)       | Blue                   |
| GPIO 22    | I2C SCL (OLED)       | Yellow                 |
| GPIO 19    | SPI MISO (MAX31855)  | Orange                 |
| GPIO 18    | SPI SCK (MAX31855)   | Green                  |
| GPIO 5     | TC CS CH1            | White                  |
| GPIO 17    | TC CS CH2            | White/Stripe           |
| GPIO 16    | TC CS CH3            | Gray                   |
| GPIO 4     | TC CS CH4            | Gray/Stripe            |
| GPIO 25    | SSR CH1              | Red (signal)           |
| GPIO 26    | SSR CH2              | Red (signal)           |
| GPIO 27    | SSR CH3              | Red (signal)           |
| GPIO 14    | SSR CH4              | Red (signal)           |
| GPIO 32    | Encoder CLK          | Purple                 |
| GPIO 33    | Encoder DT           | Brown                  |
| GPIO 35    | Encoder SW (button)  | Black                  |
| GPIO 13    | Buzzer               | White                  |
| GPIO 2     | Status LED           | (built-in)             |
| GPIO 36    | ACS712 (optional)    | Yellow (analog)        |

## Power Supply Wiring

```
IEC C14 Inlet (Fused + Switched)
├── LINE (L) ──────┬──→ HLK-PM01 AC-IN (L)
│                   └──→ SSR Load Side (per channel)
├── NEUTRAL (N) ───┬──→ HLK-PM01 AC-IN (N)
│                   └──→ XLR Pin 4/5 via SSR (per channel)
└── EARTH (PE) ────┬──→ Chassis ground point
                    ├──→ XLR shell (each connector)
                    └──→ Ground bus bar

HLK-PM01 Output
├── +5V ──→ ESP32 VIN (via USB-C breakout or direct)
└── GND ──→ ESP32 GND
```

## Per-Channel Wiring

### Thermocouple (MAX31855)

```
MAX31855 Breakout
├── VCC ──→ ESP32 3.3V
├── GND ──→ ESP32 GND
├── DO  ──→ GPIO 19 (shared SPI MISO)
├── CLK ──→ GPIO 18 (shared SPI SCK)
└── CS  ──→ GPIO 5/17/16/4 (per channel)
```

### SSR Driver (with NPN buffer)

The v2 design uses an NPN transistor buffer to ensure the SSR gets a full 5V drive signal (ESP32 GPIO is 3.3V which is marginal for some SSRs).

```
ESP32 GPIO (25/26/27/14)
    │
    ├── 1kΩ Resistor ──→ 2N2222A Base
    │
2N2222A Emitter ──→ GND
2N2222A Collector ──→ SSR (-) Input
                      SSR (+) Input ──→ +5V (from HLK-PM01)

SSR Load Side:
    Input  ──→ AC LINE (from IEC inlet, post-fuse)
    Output ──→ XLR Pin 4 + Pin 5 (load)
```

### 5-Pin Mini-XLR (NC5FD-L-1)

Industry-standard pinout:
```
Pin 1 ──→ Thermocouple (-) ──→ MAX31855 T- terminal
Pin 2 ──→ Thermocouple (+) ──→ MAX31855 T+ terminal
Pin 3 ──→ Ground (PE)      ──→ Ground bus
Pin 4 ──→ Heater Load (+)  ──→ SSR output
Pin 5 ──→ Heater Load (+)  ──→ Tied to Pin 4 (redundant path)
Shell ──→ Chassis Ground    ──→ PE ground bus
```

## I2C Bus (OLED + Optional BME280)

```
ESP32 GPIO 21 (SDA) ──┬──→ OLED SDA
                       └──→ BME280 SDA (optional)
ESP32 GPIO 22 (SCL) ──┬──→ OLED SCL
                       └──→ BME280 SCL (optional)
3.3V ──→ OLED VCC, BME280 VCC
GND  ──→ OLED GND, BME280 GND
```

Note: SSD1306 address is 0x3C, BME280 is 0x76 or 0x77.

## Rotary Encoder (KY-040)

```
Encoder Module
├── CLK ──→ GPIO 32
├── DT  ──→ GPIO 33
├── SW  ──→ GPIO 35 (input-only pin, needs external 10kΩ pullup to 3.3V)
├── +   ──→ 3.3V
└── GND ──→ GND
```

Note: GPIO 35 is input-only and has no internal pullup. Add a 10kΩ external pullup resistor to 3.3V.

## Buzzer

```
ESP32 GPIO 13 ──→ Passive Buzzer (+)
GND           ──→ Passive Buzzer (-)
```

## USB-C Programming Port

```
USB-C Breakout (panel mount)
├── VCC ──→ ESP32 5V (USB)
├── D+  ──→ ESP32 D+ (USB)
├── D-  ──→ ESP32 D- (USB)
└── GND ──→ ESP32 GND (USB)
```

Wire directly to the ESP32 DevKit USB port pads, or use a short micro-USB to USB-C adapter internally.

## Optional: ACS712 Current Sensor

```
ACS712-05B Module (per channel, in series with SSR output)
├── VCC ──→ 5V
├── GND ──→ GND
├── OUT ──→ GPIO 36 (ADC input)
├── IP+ ──→ SSR output
└── IP- ──→ XLR Pin 4/5
```

## Ground Bus

All ground connections should terminate at a central ground bus:
- ESP32 GND
- HLK-PM01 DC GND
- Each MAX31855 GND
- Each SSR (-) input
- XLR Pin 3 (each connector)
- XLR shell (each connector)
- Chassis ground lug
- IEC inlet PE

Use ring terminals crimped + soldered to 16AWG wire for all ground bus connections.

## Wire Routing Tips

1. Keep AC wiring on one side of the enclosure, DC signal wiring on the other
2. Cross AC and DC wires at 90-degree angles when they must cross
3. Use cable ties or adhesive clips to secure wire bundles
4. Leave service loops for maintenance access
5. Label each channel's wiring with numbered heat shrink or cable markers
