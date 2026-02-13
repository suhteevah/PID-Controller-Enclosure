# ESP-Nail Wiring Guide

## Safety Warnings

- **MAINS VOLTAGE**: This project involves 110-240VAC mains electricity. Improper wiring can cause **electrocution, fire, or death**.
- Only work on wiring with the unit **unplugged**.
- Use appropriate gauge wire for current ratings.
- Always include a **fuse** in the AC circuit.
- Ensure all connections are **mechanically secure** and properly insulated.
- If you are not experienced with mains wiring, have a qualified electrician review your work.

## 5-Pin Mini-XLR Pinout

The 5-pin mini-XLR connector carries both thermocouple signals and heater power to each coil. This is the industry-standard pinout compatible with most e-nail coils:

```
    Pin 1: Thermocouple (-)
    Pin 2: Thermocouple (+)
    Pin 3: Ground / Shield
    Pin 4: Heater Load (+)
    Pin 5: Heater Load (+)

  Looking at the FEMALE panel connector (solder side):

        5   1
      4   *   2
          3

  Pin 3 (bottom center) = Ground
  Pins 4,5 = Heater (paralleled for current sharing)
  Pins 1,2 = K-type thermocouple pair
```

## System Wiring Diagram

```
                    ┌─────────────────────────────────┐
                    │          AC MAINS INPUT           │
                    │   ┌──────────────────────────┐   │
                    │   │  IEC C14 Fused Inlet     │   │
                    │   │  L ──── (10A Fuse) ──┐   │   │
                    │   │  N ──────────────────┐│  │   │
                    │   │  PE ─── Chassis GND  ││  │   │
                    │   └──────────────────────┼┤──┘   │
                    │                          ││      │
                    │   ┌──────────────────────┼┤──┐   │
                    │   │  HLK-PM01 (AC→5VDC)  ││  │   │
                    │   │  AC-L ────────────── L│  │   │
                    │   │  AC-N ────────────── N│  │   │
                    │   │  +5V ──→ ESP32 VIN    │  │   │
                    │   │  GND ──→ ESP32 GND    │  │   │
                    │   └──────────────────────┘│  │   │
                    │                           │  │   │
                    │  For EACH channel (1-4):  │  │   │
                    │   ┌───────────────────────┤──┤   │
                    │   │  SSR-25DA             │  │   │
                    │   │  DC+ ← ESP32 GPIO     │  │   │
                    │   │  DC- ← ESP32 GND      │  │   │
                    │   │  AC-1 ←── AC LINE ────┘  │   │
                    │   │  AC-2 ──→ XLR Pin 4,5 ──┼──→│ COIL HEATER
                    │   └───────────────────────┘  │   │
                    │                              │   │
                    │   AC NEUTRAL ──────────────────── COIL RETURN
                    │                              │   │
                    │   ┌───────────────────────┐  │   │
                    │   │  MAX31855 (per ch)     │  │   │
                    │   │  T+ ←── XLR Pin 2     │  │   │
                    │   │  T- ←── XLR Pin 1     │  │   │
                    │   │  VCC ← 3.3V           │  │   │
                    │   │  GND ← GND            │  │   │
                    │   │  SCK ← GPIO18         │  │   │
                    │   │  SO  → GPIO19         │  │   │
                    │   │  CS  ← GPIO (per ch)  │  │   │
                    │   └───────────────────────┘  │   │
                    │                              │   │
                    └──────────────────────────────┘   │
```

## ESP32 Pin Mapping

| Function | GPIO | Notes |
|----------|------|-------|
| **I2C SDA** (OLED) | 21 | SSD1306 data |
| **I2C SCL** (OLED) | 22 | SSD1306 clock |
| **SPI MISO** (TC) | 19 | MAX31855 data out (shared) |
| **SPI SCK** (TC) | 18 | MAX31855 clock (shared) |
| **TC CS - CH1** | 5 | MAX31855 chip select, channel 1 |
| **TC CS - CH2** | 17 | MAX31855 chip select, channel 2 |
| **TC CS - CH3** | 16 | MAX31855 chip select, channel 3 |
| **TC CS - CH4** | 4 | MAX31855 chip select, channel 4 |
| **SSR - CH1** | 25 | SSR control output, channel 1 |
| **SSR - CH2** | 26 | SSR control output, channel 2 |
| **SSR - CH3** | 27 | SSR control output, channel 3 |
| **SSR - CH4** | 14 | SSR control output, channel 4 |
| **Encoder CLK** | 32 | Rotary encoder A signal |
| **Encoder DT** | 33 | Rotary encoder B signal |
| **Encoder SW** | 35 | Encoder push button (input only) |
| **Buzzer** | 13 | Piezo buzzer PWM |
| **Status LED** | 2 | Built-in LED |

## AC Power Path (Per Channel)

```
AC LINE (Hot) → Fuse → IEC Switch → SSR Input Terminal
SSR Output Terminal → XLR Pin 4 + Pin 5 (paralleled)

Coil returns to AC NEUTRAL through the heating element.

The SSR switches the HOT side; NEUTRAL is continuous to the coil.
```

### Heater Circuit Detail

```
L (Live) ──[Fuse]──[Switch]──[SSR AC-1]
                                  │
                              SSR AC-2 ──→ XLR Pin 4 ──┐
                                          XLR Pin 5 ──┤
                                                       │
                                                  Heating Coil
                                                       │
N (Neutral) ──────────────────────────────────────────┘
```

Note: XLR Pins 4 and 5 are wired in parallel to share the heater current load across two pins. Both carry the switched AC hot to the coil.

## Thermocouple Wiring

- The K-type thermocouple embedded in the heating coil connects via XLR pins 1 (-) and 2 (+)
- These low-voltage signals route to the MAX31855 thermocouple amplifier
- Keep thermocouple wiring away from AC power lines to minimize noise
- XLR Pin 3 (ground/shield) connects to chassis ground

## SSR Drive Circuit

The ESP32 GPIO outputs 3.3V which is at the minimum threshold for SSR-25DA (rated 3-32VDC). For reliable triggering:

**Option 1 - Direct drive** (simplest): Connect GPIO directly to SSR DC+ terminal, GND to DC-. Works with most genuine SSR-25DA units but is at the edge of spec.

**Option 2 - Transistor buffer** (recommended):
```
GPIO ──[1kΩ]──┤ Base (NPN 2N2222)
               │
5V ─────[SSR DC+]───┤ Collector
               │
GND ────[SSR DC-]───┤ Emitter ── GND
```
This drives the SSR with a full 5V signal for reliable switching.

## Ground / Safety Earth

- The IEC C14 inlet's protective earth (PE) terminal connects to XLR Pin 3 shells
- All XLR connector shells must be connected to protective earth
- The ESP32 circuit GND is isolated from AC mains via the HLK-PM01 converter
- In the wood box version, a ground bus bar connects all XLR shells to the PE terminal

## Assembly Checklist

- [ ] IEC inlet wired: L, N, PE connected
- [ ] Fuse installed in IEC inlet holder
- [ ] HLK-PM01 AC input connected to L and N
- [ ] HLK-PM01 5V output to ESP32 VIN
- [ ] HLK-PM01 GND to ESP32 GND
- [ ] SSR DC+ wired to correct ESP32 GPIO (per channel)
- [ ] SSR DC- wired to ESP32 GND
- [ ] SSR AC-1 wired to switched AC Live
- [ ] SSR AC-2 wired to XLR pins 4+5
- [ ] AC Neutral continuous to coil return path
- [ ] MAX31855 T+/T- connected to XLR pins 2/1
- [ ] MAX31855 SPI bus connected (shared SCK/MISO, unique CS)
- [ ] OLED I2C connected (SDA=21, SCL=22)
- [ ] Rotary encoder connected (CLK=32, DT=33, SW=35)
- [ ] Buzzer connected to GPIO13
- [ ] All XLR shells grounded to PE
- [ ] All connections mechanically secured and insulated
- [ ] No exposed mains wiring
- [ ] Fuse rating appropriate for load (10A for single, verify for multi)
- [ ] Continuity test on all connections before first power-on
