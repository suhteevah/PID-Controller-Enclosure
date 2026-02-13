# ESP-Nail Assembly Guide

## Before You Begin

Read the complete [Wiring Guide](../hardware/wiring/wiring_guide.md) before starting assembly. This project involves **mains voltage (110-240VAC)** which can cause serious injury or death if handled improperly.

## Required Tools

- Soldering iron + solder (for XLR and wire connections)
- Wire strippers (16AWG and 22AWG)
- Multimeter (for continuity testing)
- Small Phillips screwdriver
- 3D printer (for enclosure) or wood shop (for wood box)
- Heat shrink tubing assortment

## Step 1: Source Components

Purchase all items from the [Bill of Materials](../hardware/BOM.csv). The core electronics are the same for all models:

**All Models:**
- 1x ESP32 DevKit V1
- 1x SSD1306 0.96" OLED display
- 1x KY-040 rotary encoder
- 1x IEC C14 fused power inlet
- 1x HLK-PM01 AC-DC converter (5V)
- 1x Passive buzzer

**Per Channel (1/2/4):**
- 1x MAX31855 thermocouple breakout
- 1x SSR-25DA solid state relay
- 1x Aluminum heatsink for SSR
- 1x 5-pin mini-XLR female panel connector (NC5FD-L-1)

## Step 2: Print or Build the Enclosure

### 3D-Printed Enclosure
1. Open `enclosure/3d-printable/esp_nail_enclosure.scad` in OpenSCAD
2. Set `num_channels` to match your build (1, 2, or 4)
3. Render the **base** and **lid** separately
4. Export as STL and slice with your preferred slicer
5. Print settings:
   - Material: **PETG** (recommended) or ABS for heat resistance
   - Layer height: 0.2mm
   - Infill: 30%
   - Supports: Yes (for panel cutouts)
   - Walls: 3 perimeters minimum

### Wood Box Enclosure
See [wood_box_design.md](../enclosure/wood-box/wood_box_design.md) for detailed CNC and hand-build instructions.

## Step 3: Flash the Firmware

1. Install [PlatformIO](https://platformio.org/)
2. Connect ESP32 via USB
3. Build and flash:
   ```bash
   cd firmware
   pio run -e single -t upload    # or -e dual / -e quad
   ```
4. Verify: open serial monitor at 115200 baud, confirm startup message

## Step 4: Prepare Wiring Harnesses

Cut and strip wires before installing in the enclosure:

| Wire | Gauge | Length | Qty | Purpose |
|------|-------|--------|-----|---------|
| AC Live (hot) | 16AWG | 150mm | 1 + per channel | IEC to SSR(s) |
| AC Neutral | 16AWG | 150mm | 1 + per channel | IEC to XLR return |
| SSR to XLR | 16AWG | 100mm | 2 per channel | SSR output to XLR pins 4+5 |
| Ground (PE) | 16AWG | 200mm | 1 | IEC PE to XLR shells |
| TC signal | 22AWG | 100mm | 2 per channel | XLR pins 1,2 to MAX31855 |
| SPI bus | 22AWG | 80mm | 2 shared + 1 CS per channel | MAX31855 to ESP32 |
| I2C | 22AWG | 80mm | 2 | OLED to ESP32 |
| Encoder | 22AWG | 80mm | 3 | Encoder to ESP32 |
| 5V power | 22AWG | 60mm | 2 | HLK-PM01 to ESP32 |

## Step 5: Install Components

### 5a. Mount panel components
1. Press-fit or screw the **IEC C14 inlet** into the rear panel cutout
2. Install **XLR connectors** in the rear panel holes
3. Mount the **OLED display** behind the front panel window (secure with hot glue or M2 screws)
4. Install the **rotary encoder** through the front panel hole (tighten panel nut)

### 5b. Mount internal components
1. Secure the **ESP32** to its standoffs using M3 screws
2. Mount **SSR(s)** with thermal paste onto heatsink(s)
3. Secure heatsink(s) to the base with M3 screws
4. Mount **MAX31855 breakout(s)** near their respective XLR connectors
5. Secure **HLK-PM01** away from low-voltage signals (adhesive or screw mount)
6. Mount **buzzer** (hot glue or double-sided tape)

## Step 6: Wire Everything

Follow the [Wiring Guide](../hardware/wiring/wiring_guide.md) for the complete circuit. General order:

1. **AC power path first**: IEC inlet → HLK-PM01, IEC → SSR(s)
2. **SSR outputs to XLR**: SSR AC-2 → XLR pins 4+5
3. **Neutral path**: IEC N → XLR return
4. **Ground bus**: IEC PE → all XLR shells
5. **5V power**: HLK-PM01 → ESP32 VIN/GND
6. **Thermocouple**: XLR pins 1,2 → MAX31855 T-/T+
7. **SPI bus**: MAX31855 → ESP32 (shared SCK/MISO, unique CS per channel)
8. **I2C**: OLED → ESP32 (SDA=GPIO21, SCL=GPIO22)
9. **Encoder**: CLK=GPIO32, DT=GPIO33, SW=GPIO35
10. **Buzzer**: GPIO13

Insulate all AC connections with heat shrink. Double-check no bare mains conductors.

## Step 7: Pre-Power Safety Checks

Before plugging in for the first time:

- [ ] Visual inspection: no loose wires, no exposed mains conductors
- [ ] Continuity test: PE terminal to each XLR shell
- [ ] Continuity test: IEC Live to SSR AC-1 input (through fuse)
- [ ] Isolation test: No continuity between AC Live/Neutral and ESP32 GND
- [ ] Isolation test: No continuity between AC and thermocouple signals
- [ ] Verify fuse is installed in IEC inlet
- [ ] All screws tightened, no components loose

## Step 8: First Power-On

1. Plug in the unit (no coil connected)
2. ESP32 should boot: OLED shows "ESP-Nail" splash screen
3. Main screen should show "OFF" state and "---.-" for temperature (no coil)
4. Rotate encoder: temperature setpoint should adjust
5. Connect a coil to XLR
6. Short press to enable the channel
7. Temperature should begin rising
8. Verify PID holds temperature within 5°F of setpoint

## Step 9: PID Tuning (Optional)

The default PID values (Kp=8, Ki=0.2, Kd=2) work well for most standard 100W barrel coils. If you need to tune:

1. Long press → Settings → PID Tuning
2. Start with Kp only (set Ki=0, Kd=0)
3. Increase Kp until temperature oscillates, then reduce by 30%
4. Add Ki slowly until steady-state error is eliminated
5. Add Kd to dampen overshoot
6. Save settings (they persist in flash)

## Troubleshooting

| Symptom | Possible Cause | Fix |
|---------|---------------|-----|
| OLED blank | I2C wiring, wrong address | Check SDA/SCL, verify 0x3C address |
| "OPEN" TC error | No coil, broken thermocouple wire | Check XLR pin 1,2 connections |
| Temperature reads wrong | TC polarity reversed | Swap XLR pins 1 and 2 |
| SSR not switching | GPIO too low for SSR | Add transistor buffer circuit |
| No heat | Fuse blown, SSR failed | Check fuse, test SSR independently |
| Temperature oscillates wildly | PID tuning, noisy TC signal | Retune PID, route TC away from AC wires |

---

**Project Maintainers:** Swoop-r, suhteevah
