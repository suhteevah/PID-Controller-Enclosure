# Troubleshooting Guide

## Display Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| OLED blank / no display | I2C wiring wrong | Verify SDA=GPIO21, SCL=GPIO22; check solder joints |
| OLED shows garbage | Wrong I2C address | Confirm display is 0x3C (some are 0x3D - change in config.h) |
| Display flickers | Loose connection | Reseat I2C wires; add 4.7kΩ pullups on SDA/SCL |
| Text too dim | Display aging or low power | Check 3.3V rail; try different OLED module |

## Temperature Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| Shows "OPEN" | No coil connected; broken TC wire | Connect coil; check XLR pin 1,2 continuity |
| Shows "SHORT-GND" | TC wire touching ground | Inspect XLR connector; check for damaged insulation |
| Shows "SHORT-VCC" | TC wire touching power | Check for solder bridges on MAX31855 board |
| Shows "ERROR" | SPI communication failure | Verify SPI wiring (SCK=18, MISO=19, CS per channel) |
| Reads wrong temperature | TC polarity reversed | Swap XLR pins 1 and 2 |
| Reads ~32°F constantly | TC not connected but no fault | Check MAX31855 solder joints |
| Temperature jumps/noisy | EMI from AC wires | Route TC wires away from AC; add ferrite bead on TC lines |
| Offset from surface temp | Normal (TC reads coil, not surface) | Use calibration feature in Settings |

## Heating Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| No heat at all | Fuse blown | Check/replace 10A fuse in IEC inlet |
| No heat at all | SSR not switching | Check GPIO to SSR DC+ wiring; try transistor buffer |
| No heat at all | AC wiring disconnected | Verify IEC inlet → SSR → XLR wiring with multimeter |
| Heats but doesn't reach temp | Coil rated too low | Check coil wattage; larger coil may be needed |
| Heats slowly | Poor heatsink contact | Apply thermal paste between SSR and heatsink |
| Oscillates wildly | PID tuning wrong | Run auto-tune or manual tune (see PID Tuning Guide) |
| Overheats past setpoint | SSR stuck ON | Check SSR; replace if failed short-circuit |
| FAULT: OVERTEMP | Temp exceeded 1050°F | Likely SSR failure or PID issue; inspect hardware |

## WiFi Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| AP network not visible | Boot not complete | Wait 30s; check serial output for errors |
| Can't connect to AP | Wrong password | Default: `espnail42` |
| Dashboard won't load | Wrong IP | Try `http://192.168.4.1` (AP) or `http://espnail.local` |
| WiFi keeps disconnecting | Weak signal; antenna blocked | Move closer to router; ensure ESP32 antenna area is clear |
| Can't connect to home WiFi | 5GHz network | ESP32 only supports 2.4GHz WiFi |
| mDNS not resolving | OS doesn't support mDNS | Use IP address directly; install Bonjour on Windows |

## BLE Issues

| Symptom | Cause | Solution |
|---------|-------|----------|
| Device not discoverable | BLE disabled in build | Verify `ENABLE_BLE=1` in build flags |
| Can't pair | Out of range | Move within 10m; remove obstacles |
| BLE and WiFi conflicts | ESP32 shared radio | Both should work but may have reduced throughput |

## Safety Faults

| Fault | Meaning | Resolution |
|-------|---------|------------|
| OVERTEMP | Temperature exceeded 1050°F | Inspect SSR and coil; press encoder to clear |
| TC_ERROR | 10+ consecutive thermocouple read failures | Check XLR connection and wiring |
| IDLE_TIMEOUT | No user interaction for configured time | Press any button; adjust timeout in Settings |
| SSR_STUCK | SSR appears to be permanently on | Replace SSR; check drive circuit |
| POWER_ERROR | AC power anomaly detected | Check mains connection and fuse |

## Factory Reset

If all else fails:
1. **From OLED**: Long press → Settings → Factory Reset → Confirm
2. **From web**: Settings → Factory Reset
3. **Hardware**: Hold encoder button during power-on for 10 seconds
4. **Serial**: Connect USB, send `FACTORY_RESET` command

Factory reset clears: WiFi credentials, PID tunings, profiles, idle timeout, calibration. Session logs are preserved (stored separately).

## Serial Debug Output

Connect USB and open serial monitor at 115200 baud for diagnostic info:
```
===================================
  ESP-Nail_S  v2.0.0-alpha
  Channels: 1 | Build: Feb 16 2026
===================================
  CH1: 710F  PID(8.0, 0.20, 2.0)

All tasks launched. System running.
```

## Getting Help

- Open an issue: https://github.com/suhteevah/PID-Controller-Enclosure/issues
- Include: model, firmware version, serial output, and photos if hardware issue
