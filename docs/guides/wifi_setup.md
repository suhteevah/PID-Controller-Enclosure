# WiFi Setup Guide

## First Boot: AP Mode

On first boot (or after factory reset), ESP-Nail starts in **Access Point mode**:

1. Look for WiFi network: `ESPNail-XXXX` (XXXX = last 4 of MAC address)
2. Connect with password: `espnail42`
3. Open browser to: `http://192.168.4.1` or `http://espnail.local`
4. The web dashboard loads

## Connecting to Your Home WiFi (STA Mode)

1. Open the web dashboard (in AP mode)
2. Go to **Settings** tab
3. Under **WiFi Configuration**:
   - Enter your WiFi SSID
   - Enter your WiFi password
   - Click **Save & Connect**
4. ESP-Nail will restart and connect to your network
5. Find it at `http://espnail.local` (mDNS) or check your router for its IP

## Finding Your ESP-Nail on the Network

### Method 1: mDNS (Recommended)
Open browser to: `http://espnail.local`

Works on:
- macOS (built-in)
- Windows 10+ (built-in)
- Linux (install avahi-daemon)
- iOS Safari
- Android Chrome (varies by device)

### Method 2: Serial Monitor
Connect via USB, open serial monitor at 115200 baud. The IP address is printed on boot.

### Method 3: Router Admin
Check your router's DHCP client list for a device named "espnail".

## Switching Back to AP Mode

If you need to reconfigure WiFi:
1. On the OLED: Long press → Settings → WiFi → Reset
2. Or: hold encoder button for 10 seconds during boot
3. ESP-Nail restarts in AP mode

## PWA Installation (Phone Home Screen)

The web dashboard is a Progressive Web App:

### iOS:
1. Open `http://espnail.local` in Safari
2. Tap Share button → "Add to Home Screen"
3. It appears as a standalone app

### Android:
1. Open in Chrome
2. Tap "Add to Home Screen" banner (or Menu → Install App)

## Multiple ESP-Nail Units

Each unit has a unique mDNS name based on its MAC address:
- First unit: `espnail.local`
- Additional units: `espnail-XXXX.local`

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Can't find AP network | Wait 30 seconds after power on; check if already in STA mode |
| `espnail.local` not resolving | Use IP address instead; install Bonjour (Windows) |
| WiFi keeps disconnecting | Check signal strength on Info screen; move closer to router |
| Can't connect to home WiFi | Verify SSID/password; ESP32 supports 2.4GHz only (not 5GHz) |
| Forgot WiFi password | Factory reset from OLED menu or 10-second boot hold |
