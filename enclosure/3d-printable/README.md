# ESP-Nail v2 3D Printable Enclosure

## Files

- `esp_nail_v2_enclosure.scad` - Parametric OpenSCAD design (v2, supports S/D/Q models)
- `esp_nail_enclosure.scad` - Original v1 enclosure (single channel only)

## Configuration

Open `esp_nail_v2_enclosure.scad` in OpenSCAD and adjust the parameters at the top:

| Parameter       | Values    | Description                          |
|-----------------|-----------|--------------------------------------|
| `num_channels`  | 1, 2, 4  | Number of heater channels            |
| `wall_thickness`| 2.0-3.0   | Wall thickness in mm                 |
| `corner_radius` | 2-5       | Corner rounding radius               |
| `vent_slots`    | true/false| Side ventilation slots               |
| `usb_port`      | true/false| USB-C programming port cutout        |
| `show_lid`      | true/false| Render the lid (disable for base-only export) |
| `explode`       | 0-20      | Exploded view offset for visualization |

## Generating STL Files

1. Open the `.scad` file in OpenSCAD
2. Set `num_channels` to your model (1 = Model S, 2 = Model D, 4 = Model Q)
3. Export base: set `show_lid = false`, then **File > Export as STL** → `base_modelX.stl`
4. Export lid: Comment out `enclosure_base();`, set `show_lid = true`, export → `lid_modelX.stl`

## Print Settings

| Setting          | Recommended Value |
|------------------|-------------------|
| Material         | PETG (preferred) or PLA+ |
| Layer Height     | 0.2 mm            |
| Infill           | 20-30%            |
| Perimeters       | 3                  |
| Top/Bottom Layers| 4                  |
| Supports         | Not needed (designed for supportless printing) |
| Orientation      | Base: open side up. Lid: flat side down. |

## Material Notes

- **PETG** is strongly recommended due to its higher heat deflection temperature (~80C). The enclosure will be near heat-producing SSRs.
- **PLA** is acceptable for prototyping but may soften near SSR heatsinks over extended use.
- **ABS/ASA** also work well for heat resistance but require an enclosed printer.

## Enclosure Dimensions

| Model   | Width   | Depth  | Height | Approx Print Time |
|---------|---------|--------|--------|-------------------|
| Model S | 100 mm  | 80 mm  | 50 mm  | ~3-4 hours        |
| Model D | 140 mm  | 80 mm  | 50 mm  | ~5-6 hours        |
| Model Q | 200 mm  | 80 mm  | 50 mm  | ~7-9 hours        |

## Hardware Required

- 4x M3x12 screws (lid fastening)
- 4x M3 nuts or heat-set inserts
- 4x M3x8 screws (PCB standoff mounting)

Heat-set inserts are recommended for the lid screw holes for repeated assembly/disassembly.

## Post-Processing

1. Clean up any stringing or artifacts
2. Test-fit all components before final assembly
3. For a premium finish, sand with 220 → 400 grit and apply a coat of spray paint
4. Apply rubber feet to the bottom (4x adhesive bumpers)
