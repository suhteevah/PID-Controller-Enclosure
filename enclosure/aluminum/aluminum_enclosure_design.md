# ESP-Nail v2 Aluminum Enclosure Design

## Overview

The aluminum enclosure is the top-tier option, offering superior thermal management (the enclosure itself acts as a heatsink for SSRs), EMI shielding, and a professional industrial aesthetic. It targets pro users and commercial resellers.

## Material

- **6061-T6 Aluminum** - excellent machinability, good thermal conductivity (167 W/mK)
- Wall thickness: 3mm (provides structural rigidity and thermal mass)
- Finish: Type II anodized (black, silver, or custom color)

## Dimensions

| Model   | Outer (WxDxH)       | Inner Cavity        | Weight (approx) |
|---------|---------------------|---------------------|------------------|
| Model S | 115 x 95 x 50 mm   | 103 x 83 x 40 mm   | ~350g            |
| Model D | 155 x 95 x 50 mm   | 143 x 83 x 40 mm   | ~480g            |
| Model Q | 215 x 95 x 50 mm   | 203 x 83 x 40 mm   | ~650g            |

## Construction

### Two-Piece Design (Base + Lid)

**Base** - CNC milled from solid 6061-T6 billet:
- Pocketed interior cavity (3mm walls, 3mm floor)
- Integrated SSR mounting pads (thermally coupled to enclosure body)
- Threaded M3 holes for lid fastening (tapped, no inserts needed)
- Front face: XLR connector holes (17mm diameter, countersunk)
- Rear face: IEC C14 inlet cutout (rectangular pocket)
- Left face: USB-C port cutout
- Sides: Machined ventilation slots (aesthetic pattern)
- Bottom: Four M3 threaded holes for rubber feet

**Lid** - CNC milled flat panel:
- OLED display aperture with 0.5mm recess for acrylic window
- Rotary encoder shaft bore (7mm)
- Countersunk M3 fastener holes (4 corners)
- Surface engraving: model name + logo
- Underside: 1mm rabbet lip for alignment

### Thermal Design

The SSRs mount directly to the aluminum base floor via thermal pads:
- SSR heatsink interface: thermal pad (1mm silicone, 3 W/mK)
- Heat conducts through aluminum floor to entire enclosure body
- Ventilation slots provide convective cooling
- Estimated thermal resistance: ~5 C/W (SSR junction to ambient)
- This eliminates the need for separate SSR heatsinks (cost savings)

## CNC Manufacturing Notes

### Tooling

| Operation     | Tool                    | Parameters                |
|---------------|-------------------------|---------------------------|
| Roughing      | 6mm 3-flute flat end mill | 10000 RPM, 1500mm/min, 1mm DOC |
| Finishing     | 3mm 2-flute flat end mill | 12000 RPM, 800mm/min, 0.3mm DOC |
| Through-holes | Drill + boring bar      | Standard                   |
| Tapping       | M3x0.5 spiral tap       | 600 RPM                    |
| Engraving     | 0.2mm V-bit engraver    | 15000 RPM, 500mm/min       |

### Fixturing

- Soft jaw vise for base exterior (first op)
- Custom fixture plate for interior pocket (second op)
- Lid: Vacuum fixture or double-sided tape on spoilboard

## Anodizing

| Type     | Thickness | Colors Available          | Notes                    |
|----------|-----------|---------------------------|--------------------------|
| Type II  | 12-25um   | Black, Silver, Red, Blue  | Standard decorative      |
| Type III | 25-75um   | Black, Dark Bronze        | Hardcoat, extra durable  |

Recommended: Type II black anodize with laser-etched branding (white lettering on black).

## Grounding

The aluminum enclosure serves as the primary chassis ground:
- IEC inlet PE wire connects to enclosure via M3 ground stud
- All XLR shells are chassis-grounded through panel mount contact
- Internal star-ground point on base floor

## Bill of Materials (Aluminum-Specific)

| Item                        | Qty | Estimated Cost |
|-----------------------------|-----|----------------|
| 6061-T6 billet blank        | 1   | $10-25         |
| CNC machining (outsourced)  | 1   | $40-80         |
| Type II anodizing           | 1   | $15-30         |
| Laser engraving             | 1   | $5-10          |
| Thermal pads (per SSR)      | 1-4 | $1-2 ea        |
| Rubber feet (adhesive)      | 4   | $2             |
| Clear acrylic OLED window   | 1   | $1             |
| M3x8 socket head cap screws | 4   | $1             |

## Pricing Guidance

| Model   | COGS (Material+CNC+Finish) | Suggested Retail |
|---------|----------------------------|------------------|
| Model S | $80-120                    | $150-200         |
| Model D | $100-150                   | $200-275         |
| Model Q | $130-190                   | $275-375         |

Volume discounts: 10+ units reduces CNC cost by ~30%. 50+ units enables die-casting consideration.

## Manufacturing Partners

For prototyping:
- **PCBWay** CNC service (competitive pricing, good finish)
- **Xometry** (instant quotes, fast turnaround in US)
- **SendCutSend** (for simple operations)

For volume:
- Source locally for faster iteration
- Consider die-casting tooling at 500+ unit volumes ($2000-5000 tooling cost)
