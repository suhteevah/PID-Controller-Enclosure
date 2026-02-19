# ESP-Nail v2 OLED Controller Screen UI Mockup

## Display Specs
- **SSD1306** 128x64 pixel monochrome OLED
- **Font sizes**: 8px (small/labels), 12px (medium), 24px (large/temperature)
- **Navigation**: Rotary encoder (turn = scroll/adjust, press = select, long-press = back)

---

## Screen 1: Splash Screen (shown on boot, 2 seconds)

```
┌────────────────────────────────┐
│                                │
│      ╔═══╗  ╔═╗  ╔═══╗        │
│      ║╔══╝  ║║║  ║╔══╝        │
│      ║╚══╗  ║║║  ║╚══╗        │
│      ╚══╗║  ║║║  ╚══╗║        │
│      ╔══╝║  ║║║  ╔══╝║        │
│      ╚═══╝──╚═╝──╚═══╝        │
│        E S P - N a i l         │
│            v2.0.0              │
│                                │
│         Model S                │
└────────────────────────────────┘
```

## Screen 2: Main Dashboard - Single Channel

The primary screen. Large temperature readout, target, state, and a visual heat bar.

```
┌────────────────────────────────┐
│ CH1              ● HEATING     │  ← Channel label + state badge
│                                │
│         7 1 2 °F               │  ← Current temp (large 24px font)
│                                │
│ ▶ 710°F          ◐ 67%        │  ← Target temp + PID output %
│ ████████████████░░░░░░░░░░░░░ │  ← Progress bar (current/target)
│                                │
│ ⏱ 42:15         🔥 ON         │  ← Session timer + heater status
└────────────────────────────────┘
```

### State Indicators:
- `○ OFF` - Channel idle (hollow circle)
- `● HEATING` - Ramping up (filled circle, blinking)
- `◉ HOLDING` - At target temp (double circle, steady)
- `◐ COOLDOWN` - Cooling down (half circle)
- `⚠ FAULT` - Error state (warning triangle, blinking)

### Heat Bar Segments:
```
Progress bar fills left-to-right as temp approaches target:
  0-50%:   ████░░░░░░░░░░░░  (slow fill)
  50-90%:  ██████████░░░░░░  (accelerating)
  90-100%: ████████████████  (nearly full, bar pulses)
  >100%:   ████████████████  (solid, inverted if overshoot)
```

## Screen 3: Main Dashboard - Multi-Channel (2ch)

For Model D, shows both channels side by side in a compact layout.

```
┌────────────────────────────────┐
│ CH1            │ CH2           │
│   712°F        │   685°F      │  ← Current temps (medium font)
│   ▶710         │   ▶700       │  ← Targets
│ ████████████░░ │ ██████████░░ │  ← Mini progress bars
│ HEATING   67%  │ HEATING  52% │  ← State + output
│                                │
│ ⏱ 42:15                 WiFi  │  ← Timer + connectivity icon
└────────────────────────────────┘
```

## Screen 4: Main Dashboard - Multi-Channel (4ch)

For Model Q, ultra-compact 4-channel overview.

```
┌────────────────────────────────┐
│  1: 712°F ▶710 ████████░ HTG  │  ← CH1: temp, target, bar, state
│  2: 685°F ▶700 ██████░░░ HTG  │  ← CH2
│  3: 420°F ▶420 █████████ HLD  │  ← CH3 (holding = full bar)
│  4:  ---  ▶--- ░░░░░░░░░ OFF  │  ← CH4 (off = empty bar)
│                                │
│ ⏱ 42:15              WiFi BT  │  ← Timer + status icons
└────────────────────────────────┘
```

## Screen 5: Set Temperature

Entered by pressing encoder on main screen. Large target temp with adjustment arrows.

```
┌────────────────────────────────┐
│ SET TEMPERATURE         CH1    │
│                                │
│        ◄  710  ►               │  ← Rotary adjusts ±5°F
│            °F                  │
│                                │
│ ┌──────┐ ┌──────┐ ┌────────┐  │
│ │Fine±1│ │ Save │ │ Cancel │  │  ← Long-press = fine ±1°F
│ └──────┘ └──────┘ └────────┘  │
└────────────────────────────────┘
```

## Screen 6: Profile Selection

Quick-load temperature profiles. Accessible from main menu.

```
┌────────────────────────────────┐
│ PROFILES                  CH1  │
│                                │
│  > Low Temp        550°F      │  ← Selected (inverted/highlighted)
│    Standard        710°F      │
│    High Temp       750°F      │
│    Quartz          600°F      │
│                                │
│ [Press=Load]    [Long=Edit]    │
└────────────────────────────────┘
```

## Screen 7: Channel Select (Multi-Channel Models)

Choose which channel to control. Shown before set-temp or profile screens.

```
┌────────────────────────────────┐
│ SELECT CHANNEL                 │
│                                │
│  > CH1   712°F  HEATING       │  ← Currently selected
│    CH2   685°F  HEATING       │
│    CH3   420°F  HOLDING       │
│    CH4    ---   OFF           │
│                                │
│ [Turn=Select]  [Press=Enter]   │
└────────────────────────────────┘
```

## Screen 8: Main Menu

Accessed via long-press from main dashboard.

```
┌────────────────────────────────┐
│ MENU                           │
│                                │
│  > Profiles                    │  ← Selected (highlighted)
│    PID Tuning                  │
│    Calibration                 │
│    WiFi Settings               │
│    System Info                 │
│    Idle Timeout                │
└────────────────────────────────┘
```

## Screen 9: PID Tuning

Manual PID parameter adjustment or auto-tune trigger.

```
┌────────────────────────────────┐
│ PID TUNING              CH1    │
│                                │
│   Kp:    8.00        [Edit]   │
│   Ki:    0.20        [Edit]   │
│   Kd:    2.00        [Edit]   │
│                                │
│ ┌─────────────────────────┐    │
│ │    ▶ RUN AUTO-TUNE      │    │
│ └─────────────────────────┘    │
└────────────────────────────────┘
```

## Screen 10: Auto-Tune Progress

Shown during PID auto-tune process.

```
┌────────────────────────────────┐
│ AUTO-TUNING...           CH1   │
│                                │
│    ╭──╮   ╭──╮   ╭──╮         │  ← Oscillation waveform animation
│ ───╯  ╰───╯  ╰───╯  ╰───     │
│                                │
│ Cycle: 3/5     Temp: 718°F    │
│ Ku: 12.4       Tu: 45s       │
│                                │
│ [Long-press = CANCEL]          │
└────────────────────────────────┘
```

## Screen 11: Calibration

Surface temperature offset calibration.

```
┌────────────────────────────────┐
│ CALIBRATION             CH1    │
│                                │
│ Probe reads:     712°F        │
│ Actual surface:  700°F        │
│                    ◄ 700 ►     │  ← Adjust with encoder
│                                │
│ Offset:          -12°F        │  ← Calculated automatically
│                                │
│ [Press=Save]  [Long=Cancel]    │
└────────────────────────────────┘
```

## Screen 12: WiFi Settings

```
┌────────────────────────────────┐
│ WIFI SETTINGS                  │
│                                │
│ Mode:    > AP  /  STA         │
│ SSID:    ESPNail-A4B2         │
│ IP:      192.168.4.1          │
│ Clients: 1                     │
│                                │
│ Web: http://espnail.local      │
└────────────────────────────────┘
```

## Screen 13: System Info

```
┌────────────────────────────────┐
│ SYSTEM INFO                    │
│                                │
│ Model:    ESP-Nail S           │
│ FW:       v2.0.0               │
│ Uptime:   2h 15m               │
│ Free RAM: 142KB                │
│ WiFi:     Connected            │
│ MAC:      AA:BB:CC:DD:EE:FF    │
└────────────────────────────────┘
```

## Screen 14: Idle Timeout Warning

Shown 5 minutes before auto-shutoff. Beeps every 30 seconds.

```
┌────────────────────────────────┐
│ ╔══════════════════════════╗   │
│ ║    ⚠  IDLE WARNING  ⚠   ║   │
│ ╠══════════════════════════╣   │
│ ║                          ║   │
│ ║  Auto-off in  4:32       ║   │  ← Countdown timer
│ ║                          ║   │
│ ║  [Press encoder to       ║   │
│ ║   reset timer]           ║   │
│ ╚══════════════════════════╝   │
└────────────────────────────────┘
```

## Screen 15: Fault Screen

Shown when a safety fault occurs. All outputs disabled.

```
┌────────────────────────────────┐
│ ╔══════════════════════════╗   │
│ ║   ⚠ FAULT - CH1  ⚠      ║   │  ← Blinking border
│ ╠══════════════════════════╣   │
│ ║                          ║   │
│ ║  THERMOCOUPLE OPEN       ║   │  ← Fault description
│ ║                          ║   │
│ ║  All outputs DISABLED    ║   │
│ ║  [Press to acknowledge]  ║   │
│ ╚══════════════════════════╝   │
└────────────────────────────────┘
```

## Screen 16: OTA Update Progress

```
┌────────────────────────────────┐
│ FIRMWARE UPDATE                │
│                                │
│ Receiving...                   │
│                                │
│ ████████████████░░░░░░░░░░░░░ │  ← Progress bar
│           54%                  │
│                                │
│ DO NOT POWER OFF               │
└────────────────────────────────┘
```

---

## UI/UX Flow Diagram

```
                    ┌─────────┐
                    │  BOOT   │
                    │ Splash  │
                    └────┬────┘
                         │ (2s)
                    ┌────▼────┐
              ┌─────│  MAIN   │─────┐
              │     │DASHBOARD│     │
              │     └────┬────┘     │
              │          │          │
         [Long-press]  [Press]  [Turn on multi-ch]
              │          │          │
         ┌────▼────┐ ┌──▼───┐ ┌───▼────┐
         │  MENU   │ │ SET  │ │CHANNEL │
         │         │ │ TEMP │ │ SELECT │
         └────┬────┘ └──┬───┘ └───┬────┘
              │          │         │
    ┌─────────┼──────────┤         │
    │         │          │         │
┌───▼───┐ ┌──▼──┐ ┌─────▼─┐ ┌────▼───┐
│PROFILE│ │ PID │ │CALIBR.│ │  WIFI  │
│SELECT │ │TUNE │ │       │ │SETTINGS│
└───────┘ └──┬──┘ └───────┘ └────────┘
             │
        ┌────▼────┐
        │AUTOTUNE │
        │PROGRESS │
        └─────────┘

  Overlays (interrupt any screen):
  ┌──────────┐  ┌──────────┐  ┌──────────┐
  │  FAULT   │  │  IDLE    │  │   OTA    │
  │  ALERT   │  │ WARNING  │  │ PROGRESS │
  └──────────┘  └──────────┘  └──────────┘
```

## Visual Design Language

### Typography Hierarchy
- **Temperature readout**: 24px, bold (largest element, always visible)
- **State/labels**: 12px, regular
- **Secondary info**: 8px, regular

### Iconography (pixel art, 8x8)
- `●` Heater ON
- `○` Heater OFF
- `▶` Target indicator
- `⚠` Warning/Fault
- `⏱` Timer
- `◐` Percentage/output

### Animation
- **Heating state**: Temperature digits blink slowly (1Hz) while heating
- **Holding state**: Steady display, subtle pulse on progress bar
- **Fault state**: Entire screen border inverts at 2Hz
- **Auto-tune**: Sine wave animation scrolls across mid-screen

### Contrast & Readability
- White-on-black for maximum OLED contrast
- Selected menu items use inverted (black-on-white) highlight
- Important warnings use double-border box drawing characters
- All text is positioned with 2px padding from screen edges
