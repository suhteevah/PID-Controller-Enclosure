# Safety and Compliance Notes

## Disclaimer

ESP-Nail is an open-source DIY project. It has **not** been certified by UL, CSA, CE, FCC, or any regulatory body. Building and using this device is at your own risk. If you plan to sell assembled units commercially, you are responsible for obtaining appropriate certifications for your market.

## Design Safety Features

### Electrical Safety
- **Fused AC input**: 10A fuse in IEC C14 inlet protects against overcurrent
- **Zero-cross SSR**: Minimizes inrush current and EMI
- **Protective earth**: All XLR connector shells grounded to PE
- **Isolated low-voltage**: HLK-PM01 provides galvanic isolation between AC mains and ESP32 circuit
- **AC/DC separation**: PCB design maintains clearance between high and low voltage zones

### Thermal Safety
- **Hard temperature cutoff**: 1050°F absolute maximum (firmware + hardware)
- **Over-temperature fault**: Immediate SSR shutdown, fault state, alarm
- **Heatsink required**: SSR-25DA must be mounted to appropriate heatsink
- **Ventilation**: Enclosure design includes ventilation slots

### Software Safety
- **Hardware watchdog**: ESP32 WDT restarts system if firmware hangs (10s timeout)
- **Idle auto-off**: Configurable timer (default 60 min) with warning
- **Thermocouple fault detection**: Open circuit, short to GND, short to VCC
- **SSR stuck detection**: Monitors for output without temperature change
- **Bumpless PID transfer**: Prevents output spikes on enable/disable
- **Settings validation**: All loaded values checked against sane ranges

### Mechanical Safety
- **Enclosure**: Fully enclosed design prevents contact with live components
- **Strain relief**: Cable entry points designed for strain relief
- **Rubber feet**: Prevents sliding on surfaces

## For Commercial Sellers

If you plan to sell assembled ESP-Nail units, consider:

### Required (most jurisdictions)
- **Electrical safety testing** (UL, CSA, or equivalent for your market)
- **EMC testing** (FCC Part 15 in US, CE marking in EU)
- **Proper labeling**: Voltage rating, current rating, manufacturer info
- **User manual**: Include safety warnings and operating instructions
- **Liability insurance**: Product liability coverage

### Recommended
- **RoHS compliance**: Lead-free solder and compliant components
- **WEEE marking**: For EU sales
- **Thermal testing**: Verify enclosure temperatures under sustained operation
- **Drop/impact testing**: Ensure enclosure integrity
- **Burn-in testing**: Run each unit for 24h before shipping

### Labeling Requirements (Typical)

Bottom label should include:
```
ESP-Nail [Model]
Input: AC 110-240V, 50/60Hz
Max Load: [watts per channel]W
Fuse: 10A / 250V (5x20mm)
[Your company name]
[Your address]
[Serial number]
```

## Fire Safety

- **Never leave unattended** while heating
- Use on heat-resistant, non-flammable surfaces
- Keep away from flammable materials
- The idle auto-off timer is a backup, not a substitute for supervision
- Wood enclosure: internal surfaces should be treated with fire-retardant spray
- 3D-printed enclosure: use PETG or ABS (higher heat resistance than PLA)

## Known Limitations

1. ESP32 GPIO outputs 3.3V - at minimum SSR trigger threshold; transistor buffer recommended for reliability
2. MAX31855 accuracy is ±2°C (±3.6°F) - adequate for this application but not precision instrument grade
3. WiFi operation may introduce minor delays in PID computation if both run on same core (mitigated by RTOS design)
4. Open-source firmware has not been through formal code review for safety-critical applications
