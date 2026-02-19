# REST API Reference

The ESP-Nail v2 web server exposes a REST API for programmatic control. All endpoints return JSON. Base URL: `http://espnail.local` or device IP.

## Endpoints

### GET /api/status
Returns current state of all channels.

**Response:**
```json
{
  "model": "ESP-Nail_S",
  "version": "2.0.0-alpha",
  "uptime": 3600,
  "channels": [
    {
      "id": 0,
      "state": "HOLDING",
      "currentTemp": 710.5,
      "targetTemp": 710.0,
      "pidOutput": 42.3,
      "tcStatus": "OK",
      "profile": "Standard"
    }
  ],
  "safety": {
    "faults": 0,
    "idleRemaining": 45,
    "shutdown": false
  },
  "wifi": {
    "mode": "STA",
    "ssid": "MyNetwork",
    "rssi": -45,
    "ip": "192.168.1.100"
  }
}
```

### POST /api/channel/{n}/enable
Enable channel `n` (0-indexed).

**Response:** `{"ok": true}`

### POST /api/channel/{n}/disable
Disable channel `n`.

**Response:** `{"ok": true}`

### POST /api/channel/{n}/settemp
Set target temperature for channel `n`.

**Body:** `{"temp": 710}`

**Response:** `{"ok": true, "temp": 710}`

### POST /api/channel/{n}/autotune
Start PID auto-tune on channel `n`.

**Response:** `{"ok": true, "state": "AUTOTUNE"}`

### GET /api/profiles/{n}
Get all profiles for channel `n`.

**Response:**
```json
{
  "channel": 0,
  "active": 2,
  "profiles": [
    {"index": 0, "name": "Low Temp", "temp": 500, "hasCustomPID": false},
    {"index": 1, "name": "Medium", "temp": 620, "hasCustomPID": false},
    {"index": 2, "name": "Standard", "temp": 710, "hasCustomPID": false},
    {"index": 3, "name": "Hot", "temp": 800, "hasCustomPID": false}
  ]
}
```

### POST /api/profiles/{n}/{idx}
Update profile `idx` for channel `n`.

**Body:** `{"name": "My Preset", "temp": 650, "kp": 8.0, "ki": 0.2, "kd": 2.0}`

### POST /api/profiles/{n}/active
Set active profile.

**Body:** `{"index": 2}`

### GET /api/settings
Get global settings.

**Response:**
```json
{
  "idleTimeout": 60,
  "fahrenheit": true,
  "displayBrightness": 255,
  "wifiMode": 1,
  "wifiSSID": "MyNetwork",
  "mqttHost": "",
  "mqttPort": 1883
}
```

### POST /api/settings
Update global settings.

**Body:** `{"idleTimeout": 90, "fahrenheit": true}`

### POST /api/wifi/connect
Connect to a WiFi network.

**Body:** `{"ssid": "MyNetwork", "password": "secret"}`

**Response:** `{"ok": true, "message": "Connecting..."}`

### POST /api/wifi/ap
Switch to AP mode.

**Response:** `{"ok": true, "ssid": "ESPNail-A1B2"}`

### GET /api/session/log
Get session history.

**Response:**
```json
{
  "sessions": [
    {
      "start": 1708099200,
      "duration": 1800,
      "channel": 0,
      "peakTemp": 715.3,
      "avgTemp": 709.8,
      "targetTemp": 710.0,
      "energyWh": 15.2
    }
  ]
}
```

### DELETE /api/session/log
Clear all session history.

### GET /api/calibration/{n}
Get calibration for channel `n`.

### POST /api/calibration/{n}
Set calibration offset.

**Body:** `{"offset": -25.0, "coilLabel": "25mm Barrel"}`

### POST /api/ota/upload
Upload firmware binary (multipart/form-data).

**Form field:** `firmware` = .bin file

**Response (on completion):** `{"ok": true, "message": "Update complete, rebooting..."}`

### POST /api/factory-reset
Factory reset all settings.

**Response:** `{"ok": true, "message": "Resetting..."}`

## WebSocket: /ws

Real-time temperature streaming. Connect via WebSocket and receive JSON messages every 500ms:

```json
{
  "type": "temp",
  "channels": [
    {"id": 0, "temp": 710.3, "target": 710, "output": 42.1, "state": "HOLDING"}
  ],
  "uptime": 3605,
  "idleRemaining": 44
}
```

### Sending Commands via WebSocket

```json
{"cmd": "enable", "ch": 0}
{"cmd": "disable", "ch": 0}
{"cmd": "settemp", "ch": 0, "temp": 710}
```

## CORS

All API endpoints include CORS headers allowing requests from any origin:
```
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS
Access-Control-Allow-Headers: Content-Type
```

## Error Responses

```json
{
  "ok": false,
  "error": "Invalid channel index"
}
```

HTTP status codes: 200 (success), 400 (bad request), 404 (not found), 500 (server error).
