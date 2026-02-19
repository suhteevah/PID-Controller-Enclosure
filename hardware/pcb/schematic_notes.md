# ESP-Nail v2 Schematic Notes

## SSR Driver Circuit (per channel)

NPN transistor buffer to drive SSR from 3.3V ESP32 GPIO:

```
GPIO (25/26/27/14)
  │
  ├─── R1 (1kΩ) ──→ Q1 Base (2N2222A)
  │
  Q1 Emitter ──→ GND
  Q1 Collector ──→ SSR(-) terminal
                   SSR(+) terminal ──→ +5V rail

  R1 = 1kΩ (base current limiter)
  Ib = (3.3V - 0.7V) / 1kΩ = 2.6mA
  β(min) = 75 → Ic(max) = 195mA (SSR input draws ~10-15mA)
  Saturation guaranteed.
```

Component values:
- Q1: 2N2222A (TO-92) NPN transistor
- R1: 1kΩ 1/4W carbon film or metal film

## MAX31855 SPI Bus

Shared SPI bus with individual chip select per channel:

```
  MISO (GPIO 19) ──→ all MAX31855 DO pins (directly, no buffer needed)
  SCK  (GPIO 18) ──→ all MAX31855 CLK pins
  CS1  (GPIO 5)  ──→ MAX31855 #1 CS
  CS2  (GPIO 17) ──→ MAX31855 #2 CS
  CS3  (GPIO 16) ──→ MAX31855 #3 CS
  CS4  (GPIO 4)  ──→ MAX31855 #4 CS
```

Each CS line has a 10kΩ pullup to 3.3V (keep deselected by default).

Decoupling: 100nF ceramic capacitor on each MAX31855 VCC pin, close to the IC.

## I2C Bus

```
  SDA (GPIO 21) ──→ OLED + optional BME280
  SCL (GPIO 22) ──→ OLED + optional BME280

  Pullups: 4.7kΩ to 3.3V on both SDA and SCL
  (most OLED modules include onboard pullups, but add footprints on PCB)
```

## Encoder Interface

```
  CLK (GPIO 32) ──→ Encoder A output
  DT  (GPIO 33) ──→ Encoder B output
  SW  (GPIO 35) ──→ Encoder push button

  GPIO 35 is input-only (no internal pullup).
  Add 10kΩ external pullup to 3.3V on the PCB.

  Optional: 100nF debounce caps on CLK and DT lines.
```

## Buzzer Driver

```
  GPIO 13 ──→ Passive piezo buzzer (+)
  GND     ──→ Passive piezo buzzer (-)

  ESP32 GPIO can source ~12mA at 3.3V.
  Most passive piezo buzzers work fine at this level.
  For louder output, add an NPN driver (same circuit as SSR driver).
```

## Power Distribution

```
  HLK-PM01 +5V ──→ +5V rail (SSR drivers, optional ACS712)
                 ──→ ESP32 VIN pin (onboard 3.3V regulator)

  ESP32 3.3V pin ──→ 3.3V rail (MAX31855, OLED, encoder, pullups)
```

Decoupling:
- 100uF electrolytic + 100nF ceramic on 5V rail entry
- 100uF electrolytic + 100nF ceramic on 3.3V rail
- 100nF ceramic on each IC VCC pin

## Optional: ACS712 Current Sensor

```
  ACS712-05B analog output ──→ GPIO 36 (ADC1_CH0)

  Voltage divider not needed: ACS712 output is 0.5-4.5V centered at 2.5V.
  ESP32 ADC range: 0-3.3V (use 11dB attenuation for full range).

  Note: GPIO 36 is input-only, ADC1 (safe to use with WiFi active).
```

## PCB Layout Considerations

1. Place ESP32 module centrally with headers on both sides
2. Group channel connectors along one edge
3. Keep SPI traces short and parallel
4. Ground plane on bottom layer for EMI reduction
5. Route I2C and SPI buses with ground return paths nearby
6. Thermal relief on all ground pads for easier soldering
7. Add test points for +5V, +3.3V, GND, and each SSR control signal
8. Include mounting holes (M3) at four corners matching enclosure standoff positions
