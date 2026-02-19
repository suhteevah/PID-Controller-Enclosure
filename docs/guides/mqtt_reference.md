# MQTT Reference

## Overview

ESP-Nail v2 publishes temperature, state, and control data via MQTT. Supports Home Assistant auto-discovery for zero-configuration integration.

## Configuration

Configure MQTT via the web dashboard (Settings → MQTT) or REST API:

| Setting | Default | Description |
|---------|---------|-------------|
| Host | (empty) | MQTT broker hostname/IP |
| Port | 1883 | MQTT broker port |
| Username | (empty) | MQTT auth username |
| Password | (empty) | MQTT auth password |

MQTT is disabled when host is empty. Enable with `ENABLE_MQTT=1` build flag.

## Topics

### Published (by ESP-Nail)

| Topic | Payload | Interval |
|-------|---------|----------|
| `espnail/status` | `online` / `offline` | On connect / LWT |
| `espnail/ch{n}/temp` | `710.5` | Every 5s |
| `espnail/ch{n}/target` | `710` | On change |
| `espnail/ch{n}/state` | `OFF`/`HEAT`/`HOLD`/`COOL`/`TUNE`/`FAULT` | On change |
| `espnail/ch{n}/output` | `42.3` | Every 5s |
| `espnail/ch{n}/tc_status` | `OK`/`OPEN`/`ERROR` | On change |
| `espnail/idle_remaining` | `45` | Every 60s |

### Subscribed (commands to ESP-Nail)

| Topic | Payload | Action |
|-------|---------|--------|
| `espnail/ch{n}/cmd/enable` | (any) | Enable channel |
| `espnail/ch{n}/cmd/disable` | (any) | Disable channel |
| `espnail/ch{n}/cmd/settemp` | `710` | Set target temperature |
| `espnail/cmd/idle_reset` | (any) | Reset idle timer |

## Home Assistant Auto-Discovery

On MQTT connect, ESP-Nail publishes HA discovery messages to `homeassistant/` prefix:

### Climate Entity (per channel)
```json
{
  "name": "ESP-Nail CH1",
  "unique_id": "espnail_XXXX_ch0",
  "modes": ["off", "heat"],
  "temperature_command_topic": "espnail/ch0/cmd/settemp",
  "temperature_state_topic": "espnail/ch0/target",
  "current_temperature_topic": "espnail/ch0/temp",
  "mode_command_topic": "espnail/ch0/cmd/mode",
  "mode_state_topic": "espnail/ch0/state",
  "min_temp": 0,
  "max_temp": 999,
  "temp_step": 5,
  "device": {
    "identifiers": ["espnail_XXXX"],
    "name": "ESP-Nail S",
    "model": "ESP-Nail_S",
    "sw_version": "2.0.0-alpha",
    "manufacturer": "ESP-Nail Open Source"
  }
}
```

### Sensor Entities
- Current temperature sensor per channel
- PID output percentage sensor per channel
- Idle time remaining sensor

### Binary Sensor
- Fault status (on = faulted)

## Home Assistant Automations

Example automations:

```yaml
# Turn off e-nail when leaving home
automation:
  - trigger:
      platform: state
      entity_id: person.me
      to: "not_home"
    action:
      service: climate.turn_off
      target:
        entity_id: climate.esp_nail_ch1

# Notification when at temperature
automation:
  - trigger:
      platform: template
      value_template: "{{ state_attr('climate.esp_nail_ch1', 'current_temperature') >= state_attr('climate.esp_nail_ch1', 'temperature') }}"
    action:
      service: notify.mobile_app
      data:
        message: "ESP-Nail is at temperature!"

# Voice control (Google Home via HA)
# "Hey Google, set the ESP-Nail to 650 degrees"
# Works automatically through HA climate entity
```

## Mosquitto Broker Setup (Quick)

```bash
# Install
sudo apt install mosquitto mosquitto-clients

# Test subscription
mosquitto_sub -h localhost -t "espnail/#" -v

# Test publishing
mosquitto_pub -h localhost -t "espnail/ch0/cmd/settemp" -m "710"
```
