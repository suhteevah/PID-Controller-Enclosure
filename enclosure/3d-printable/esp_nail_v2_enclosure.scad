// ============================================================
// ESP-Nail v2 Parametric Enclosure - OpenSCAD
// Supports Model S (1ch), Model D (2ch), Model Q (4ch)
// ============================================================

// --- Configuration ---
num_channels    = 1;    // 1, 2, or 4
wall_thickness  = 2.5;  // mm
corner_radius   = 3;    // mm
vent_slots      = true; // ventilation slots on sides
usb_port        = true; // USB-C programming port cutout
show_lid        = true; // render lid
explode         = 0;    // mm - explode view offset (0 = assembled)

// --- Derived Dimensions ---
// Width scales with channel count for XLR connectors
base_width      = (num_channels == 1) ? 100 :
                  (num_channels == 2) ? 140 : 200;
base_depth      = 80;
base_height     = 45;

lid_height      = 5;
total_height    = base_height + lid_height;

// --- Component Positions (relative to enclosure origin) ---
// IEC inlet - rear center
iec_width       = 27.5;  // IEC C14 fused+switched cutout
iec_height      = 20;
iec_y           = base_depth - wall_thickness;
iec_x           = base_width / 2;
iec_z           = base_height / 2;

// XLR connectors - front face, evenly spaced
xlr_diameter    = 17;    // 5-pin mini-XLR panel cutout
xlr_spacing     = 30;    // center-to-center
xlr_y           = wall_thickness;
xlr_z           = base_height / 2;

function xlr_x(i) = base_width / 2 - (num_channels - 1) * xlr_spacing / 2 + i * xlr_spacing;

// OLED display - lid top
oled_width      = 27;
oled_height     = 13;
oled_x          = base_width / 2;
oled_y          = 20;

// Encoder - lid top, right of OLED
encoder_diameter = 7;
encoder_x       = base_width / 2 + 25;
encoder_y       = 20;

// USB-C - left side
usbc_width      = 9;
usbc_height     = 3.5;
usbc_x          = wall_thickness;
usbc_y          = 15;
usbc_z          = 10;

// Mounting standoffs
standoff_h      = 5;
standoff_d      = 6;     // outer diameter
standoff_hole   = 3.2;   // M3 through hole
standoff_inset  = 8;     // from corner

// Vent slots
vent_width      = 1.5;
vent_length     = 20;
vent_spacing    = 4;
vent_count      = 6;

// --- Modules ---

module rounded_box(w, d, h, r) {
    hull() {
        for (x = [r, w-r], y = [r, d-r]) {
            translate([x, y, 0]) cylinder(r=r, h=h, $fn=32);
        }
    }
}

module enclosure_base() {
    difference() {
        // Outer shell
        rounded_box(base_width, base_depth, base_height, corner_radius);

        // Inner cavity
        translate([wall_thickness, wall_thickness, wall_thickness])
            rounded_box(base_width - 2*wall_thickness,
                        base_depth - 2*wall_thickness,
                        base_height, corner_radius);

        // IEC C14 inlet cutout (rear face)
        translate([iec_x - iec_width/2, iec_y - 1, iec_z - iec_height/2])
            cube([iec_width, wall_thickness + 2, iec_height]);

        // XLR cutouts (front face)
        for (i = [0 : num_channels - 1]) {
            translate([xlr_x(i), -1, xlr_z])
                rotate([-90, 0, 0])
                    cylinder(d=xlr_diameter, h=wall_thickness + 2, $fn=48);
        }

        // USB-C cutout (left side)
        if (usb_port) {
            translate([-1, usbc_y - usbc_width/2, usbc_z - usbc_height/2])
                cube([wall_thickness + 2, usbc_width, usbc_height]);
        }

        // Ventilation slots (right side)
        if (vent_slots) {
            for (i = [0 : vent_count - 1]) {
                translate([base_width - wall_thickness - 1,
                          base_depth/2 - (vent_count * vent_spacing)/2 + i * vent_spacing,
                          base_height - 15])
                    cube([wall_thickness + 2, vent_width, vent_length]);
            }
            // Left side vents
            for (i = [0 : vent_count - 1]) {
                translate([-1,
                          base_depth/2 - (vent_count * vent_spacing)/2 + i * vent_spacing,
                          base_height - 15])
                    cube([wall_thickness + 2, vent_width, vent_length]);
            }
        }

        // Lid screw holes (M3) at four corners
        for (pos = [[standoff_inset, standoff_inset],
                     [base_width - standoff_inset, standoff_inset],
                     [standoff_inset, base_depth - standoff_inset],
                     [base_width - standoff_inset, base_depth - standoff_inset]]) {
            translate([pos[0], pos[1], base_height - 1])
                cylinder(d=standoff_hole, h=wall_thickness + 2, $fn=24);
        }
    }

    // Internal standoffs for PCB mounting
    for (pos = [[standoff_inset, standoff_inset],
                 [base_width - standoff_inset, standoff_inset],
                 [standoff_inset, base_depth - standoff_inset],
                 [base_width - standoff_inset, base_depth - standoff_inset]]) {
        translate([pos[0], pos[1], wall_thickness])
            difference() {
                cylinder(d=standoff_d, h=standoff_h, $fn=24);
                cylinder(d=standoff_hole, h=standoff_h + 1, $fn=24);
            }
    }

    // SSR mounting shelf (raised platform along rear wall)
    translate([wall_thickness + 2, base_depth - wall_thickness - 18, wall_thickness])
        cube([base_width - 2*wall_thickness - 4, 16, 3]);
}

module enclosure_lid() {
    difference() {
        union() {
            // Lid plate
            rounded_box(base_width, base_depth, lid_height, corner_radius);

            // Inner lip (fits inside base)
            translate([wall_thickness + 0.3, wall_thickness + 0.3, -2])
                rounded_box(base_width - 2*wall_thickness - 0.6,
                            base_depth - 2*wall_thickness - 0.6,
                            2, corner_radius - 1);
        }

        // OLED display window
        translate([oled_x - oled_width/2, oled_y - oled_height/2, -1])
            cube([oled_width, oled_height, lid_height + 2]);

        // Encoder hole
        translate([encoder_x, encoder_y, -1])
            cylinder(d=encoder_diameter, h=lid_height + 2, $fn=32);

        // Screw holes
        for (pos = [[standoff_inset, standoff_inset],
                     [base_width - standoff_inset, standoff_inset],
                     [standoff_inset, base_depth - standoff_inset],
                     [base_width - standoff_inset, base_depth - standoff_inset]]) {
            translate([pos[0], pos[1], -1])
                cylinder(d=standoff_hole, h=lid_height + 2, $fn=24);
            // Countersink
            translate([pos[0], pos[1], lid_height - 1.5])
                cylinder(d1=standoff_hole, d2=6, h=1.6, $fn=24);
        }

        // Model label engraving on lid surface
        translate([base_width/2, base_depth - 15, lid_height - 0.4])
            linear_extrude(0.5)
                text(str("ESP-Nail ",
                    num_channels == 1 ? "S" :
                    num_channels == 2 ? "D" : "Q"),
                    size=5, halign="center", valign="center",
                    font="Liberation Sans:style=Bold");
    }
}

// --- Render ---
color("SlateGray") enclosure_base();

if (show_lid) {
    translate([0, 0, base_height + explode])
        color("DarkSlateGray") enclosure_lid();
}
