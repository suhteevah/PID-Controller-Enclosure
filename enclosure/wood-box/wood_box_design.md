# ESP-Nail Premium Wood Box Enclosure

## Overview

The premium wood box is an upcharge option providing an elegant, handcrafted enclosure for the ESP-Nail controller. It uses the same electronics as the 3D-printed version but housed in a CNC-milled or hand-crafted hardwood case.

## Design Philosophy

- **Slimline profile**: Matches the compact footprint of the 3D-printed version
- **Natural materials**: Walnut, cherry, maple, or bamboo
- **Passive cooling**: Machined aluminum heatsink plate integrated into the base
- **Minimal visible hardware**: Hidden internal mounting, magnetic lid closure
- **Craftsmanship**: Chamfered edges, satin finish, laser-engraved branding

## Dimensions

| Model | External (W x D x H) | Internal Cavity | Material Thickness |
|-------|----------------------|-----------------|-------------------|
| S (1ch) | 100 x 120 x 55 mm | 80 x 100 x 42 mm | 10mm sides, 6mm top/bottom |
| D (2ch) | 130 x 120 x 55 mm | 110 x 100 x 42 mm | 10mm sides, 6mm top/bottom |
| Q (4ch) | 180 x 120 x 55 mm | 160 x 100 x 42 mm | 10mm sides, 6mm top/bottom |

## Construction Method

### Option A: CNC Milled (Recommended for Production)

1. **Base block**: Mill from solid hardwood stock
   - Internal cavity pocket: 3mm corner radius, 42mm depth
   - Side ventilation channels: 2mm wide slots, 20mm long, 5 per side
   - Rear panel: through-holes for XLR, IEC inlet
   - Front panel: OLED window cutout, encoder shaft hole
   - Bottom: aluminum heatsink plate recess (2mm deep)

2. **Lid**: 6mm thick hardwood panel
   - Rabbet lip: 3mm x 3mm for alignment
   - Magnet recesses: 4x 6mm diameter x 3mm deep for neodymium magnets
   - Top ventilation: decorative pattern of small holes (2mm diameter)

3. **Heatsink plate**: 3mm aluminum sheet, CNC cut to fit base recess
   - Thermal pad between SSR and aluminum plate
   - Exposed on bottom for passive cooling

### Option B: Hand-Built (Small Batch / Artisan)

1. **Box construction**: Mitered corners, glued and clamped
   - 10mm hardwood for sides
   - 6mm plywood or hardwood for top/bottom
   - Rabbeted joints for clean lines

2. **CNC or drill press** for panel cutouts
3. **Hand-sanded** to 220 grit, finished with:
   - Tung oil (natural) or
   - Water-based polyurethane (durable)

## Component Mounting

```
┌────────────────────────────────────────┐
│              REAR PANEL                │
│  [IEC Power] [XLR 1] [XLR 2] ...      │
├────────────────────────────────────────┤
│                                        │
│   ┌──────────┐  ┌──────────────────┐   │
│   │ ESP32    │  │ SSR + Heatsink   │   │
│   │ DevKit   │  │  (per channel)   │   │
│   │          │  │                  │   │
│   └──────────┘  └──────────────────┘   │
│                                        │
│   ┌──────┐  ┌──────┐                  │
│   │MAX   │  │MAX   │  (per channel)   │
│   │31855 │  │31855 │                  │
│   └──────┘  └──────┘                  │
│                                        │
├────────────────────────────────────────┤
│              FRONT PANEL               │
│    [OLED Display]    [Encoder]         │
└────────────────────────────────────────┘
```

## Bill of Materials (Wood Box Additions)

| Item | Qty | Description | Est. Cost |
|------|-----|-------------|-----------|
| Hardwood blank (walnut) | 1 | 200x150x70mm block | $8-15 |
| Aluminum plate (3mm) | 1 | 100x120mm heatsink base | $3-5 |
| Thermal pad | 1 | 50x50x1mm silicone | $2 |
| Neodymium magnets | 8 | 6x3mm disc (4 lid, 4 base) | $3 |
| Brass threaded inserts | 4 | M3 heat-set inserts | $2 |
| M3x8 SS screws | 4 | Internal component mounting | $1 |
| Tung oil finish | - | 1 coat | $2 |
| **Total materials** | | | **$21-30** |

## Suggested Retail Pricing

| Model | 3D-Printed Kit | Wood Box Kit | Wood Box Assembled |
|-------|---------------|-------------|-------------------|
| S (1ch) | $15 | $45 | $85 |
| D (2ch) | $20 | $55 | $110 |
| Q (4ch) | $30 | $75 | $160 |

*Electronics sold separately or as complete bundles*

## Finishing Options

1. **Natural Walnut** - Dark, rich grain, satin tung oil finish
2. **Maple Blonde** - Light, clean look, water-based poly
3. **Cherry Classic** - Warm mid-tone, darkens with age
4. **Bamboo Modern** - Sustainable, light color, unique grain
5. **Custom** - Customer's choice of wood species (upcharge)

## Laser Engraving

- Top lid: "ESP-Nail" logo + model designation
- Bottom: Serial number, electrical ratings, safety certifications
- Optional: Custom text or graphics per customer request

## Safety Considerations

- All internal wiring routed away from wood surfaces
- Silicone wire sleeves at high-temperature areas
- Aluminum heatsink plate provides thermal isolation from wood
- Ventilation prevents heat buildup
- Internal surfaces treated with fire-retardant spray for added safety
