# Over-the-Air (OTA) Firmware Update Guide

## Overview

ESP-Nail v2 supports wireless firmware updates through the web dashboard. No USB cable needed after initial setup.

## How It Works

The ESP32 flash has two firmware slots (app0 and app1). The running firmware occupies one slot; OTA writes to the other. On successful update, the boot pointer switches to the new slot. If the new firmware fails to boot, it automatically rolls back.

## Performing an Update

1. Download the latest firmware `.bin` file from the releases page
2. Open the ESP-Nail web dashboard (`http://espnail.local`)
3. Go to **Settings** → **Firmware Update**
4. Click **Choose File** and select the `.bin` file
5. Click **Upload & Update**
6. Progress bar shows upload status
7. ESP-Nail automatically restarts with new firmware
8. Verify version on OLED Info screen or web dashboard

## Building Firmware for OTA

```bash
cd firmware/v2

# Build the firmware binary
pio run -e single

# The .bin file is at:
# .pio/build/single/firmware.bin
```

## Safety

- **Settings are preserved** across updates (stored in NVS, separate from firmware)
- **Automatic rollback** if new firmware crashes during first boot
- **Version validation** prevents uploading incompatible firmware
- **All channels are disabled** during the update process
- Update takes approximately 15-30 seconds

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Upload fails at 0% | Check WiFi connection; try refreshing dashboard |
| Upload fails mid-way | Retry; the old firmware remains intact |
| ESP-Nail doesn't reboot | Wait 60 seconds; if stuck, power cycle |
| Wrong version after update | Verify you selected the correct .bin file |
| Bricked after bad update | Connect via USB and flash with `pio run -t upload` |

## USB Fallback

If OTA is unavailable, connect USB-C cable and flash directly:
```bash
pio run -e single -t upload
```
