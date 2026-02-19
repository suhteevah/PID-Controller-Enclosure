# BLE Reference

## Overview

ESP-Nail v2 exposes a BLE GATT service for direct phone control without WiFi. This is useful for quick adjustments when you don't want to open the web dashboard.

## BLE Identifiers

| Property | Value |
|----------|-------|
| Device Name | `ESPNail-XXXX` (last 4 of MAC) |
| Service UUID | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` |
| Characteristic: Temperature | `beb5483e-36e1-4688-b7f5-ea07361b26a8` (Read, Notify) |
| Characteristic: Command | `beb5483f-36e1-4688-b7f5-ea07361b26a8` (Write) |
| Characteristic: State | `beb54840-36e1-4688-b7f5-ea07361b26a8` (Read, Notify) |

## Temperature Characteristic (Read/Notify)

Returns JSON with all channel temperatures. Notifications sent every 2 seconds.

```json
{
  "ch": [
    {"t": 710.5, "s": 710, "o": 42.3}
  ]
}
```

Fields: `t` = current temp, `s` = setpoint, `o` = PID output %

## Command Characteristic (Write)

Write JSON commands:

```json
{"cmd": "enable", "ch": 0}
{"cmd": "disable", "ch": 0}
{"cmd": "settemp", "ch": 0, "temp": 710}
{"cmd": "profile", "ch": 0, "idx": 2}
```

## State Characteristic (Read/Notify)

Returns channel states. Notifications on state change.

```json
{
  "ch": [
    {"state": "HOLD", "tc": "OK"}
  ],
  "idle": 45,
  "fault": 0
}
```

## Connecting from Phone

### Generic BLE Scanner (nRF Connect, LightBlue)
1. Scan for devices starting with "ESPNail-"
2. Connect and browse services
3. Read Temperature characteristic for current data
4. Write to Command characteristic for control

### Custom App Development
Use Web Bluetooth API or platform-native BLE libraries to build a custom control app. The GATT interface is designed to be simple enough for direct integration.

## Limitations

- BLE range: ~10m typical (line of sight)
- BLE and WiFi share the ESP32 radio - both work simultaneously but throughput may be reduced
- BLE notifications are limited to 20 bytes per characteristic update (JSON is kept compact)
- Only one BLE connection at a time
