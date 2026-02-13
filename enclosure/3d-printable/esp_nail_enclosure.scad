// ============================================================
// ESP-Nail Parametric Enclosure - OpenSCAD
// Slimline 3D-printable enclosure for ESP32 E-Nail Controller
// Supports 1, 2, or 4 channel configurations
// ============================================================

/* [Model Configuration] */
// Number of 5-pin mini-XLR channels
num_channels = 1; // [1, 2, 4]

/* [Wall Parameters] */
wall = 2.5;           // Wall thickness
corner_r = 3;         // Corner radius
vent_slot_w = 1.5;    // Ventilation slot width
vent_slot_l = 15;     // Ventilation slot length

/* [Component Dimensions] */
// ESP32 DevKit V1
esp32_w = 28;
esp32_l = 55;
esp32_h = 12;

// SSD1306 0.96" OLED
oled_w = 27;
oled_h = 19;
oled_pcb_w = 27;
oled_pcb_h = 28;
oled_cutout_w = 25;
oled_cutout_h = 14;

// SSR-25DA
ssr_w = 43;
ssr_l = 32;
ssr_h = 24;

// SSR Heatsink (compact flat fin)
hs_w = 50;
hs_l = 40;
hs_h = 20;

// MAX31855 breakout
max31855_w = 15;
max31855_l = 18;
max31855_h = 3;

// 5-pin mini-XLR panel mount
xlr_diameter = 17;     // Panel hole diameter
xlr_depth = 25;

// IEC C14 Power Inlet (fused)
iec_w = 27.5;
iec_h = 20;
iec_depth = 30;

// Rotary encoder
encoder_shaft_d = 7;   // Panel hole for shaft
encoder_body_d = 12;

// DC barrel jack (5V for ESP32)
dc_jack_d = 8;

/* [Internal Layout] */
pcb_standoff_h = 5;    // Height of PCB standoffs
internal_padding = 3;   // Padding between components

// ============================================================
// Calculated dimensions
// ============================================================

// Width: based on heatsink + SSR + padding
internal_w = hs_w + internal_padding * 2;

// Depth: components stacked front-to-back
internal_d = max(ssr_l + hs_l, esp32_l + oled_pcb_h) + internal_padding * 3;

// Height: SSR + heatsink stacked
internal_h = max(ssr_h + hs_h, 45) + pcb_standoff_h + internal_padding;

// Add width for extra channels
channel_extra_w = (num_channels > 1) ? (num_channels - 1) * (ssr_w + internal_padding) : 0;

// Final external dimensions
ext_w = internal_w + channel_extra_w + wall * 2;
ext_d = internal_d + wall * 2;
ext_h = internal_h + wall * 2;

// Lid split height
lid_h = 12;
base_h = ext_h - lid_h;

echo(str("Enclosure: ", ext_w, " x ", ext_d, " x ", ext_h, " mm"));
echo(str("Model: ", num_channels, " channel(s)"));

// ============================================================
// Modules
// ============================================================

module rounded_box(w, d, h, r) {
    hull() {
        for (x = [r, w-r]) {
            for (y = [r, d-r]) {
                translate([x, y, 0])
                    cylinder(h=h, r=r, $fn=32);
            }
        }
    }
}

module vent_slots(count, slot_w, slot_l, wall_t) {
    spacing = slot_w * 2.5;
    total_w = count * spacing;
    for (i = [0:count-1]) {
        translate([i * spacing - total_w/2, -slot_l/2, -0.1])
            cube([slot_w, slot_l, wall_t + 0.2]);
    }
}

module screw_post(h, outer_d, inner_d) {
    difference() {
        cylinder(h=h, d=outer_d, $fn=24);
        translate([0, 0, -0.1])
            cylinder(h=h+0.2, d=inner_d, $fn=24);
    }
}

module pcb_standoff(h) {
    screw_post(h, 6, 2.5);
}

// ============================================================
// Base (bottom half)
// ============================================================
module base() {
    difference() {
        // Outer shell
        rounded_box(ext_w, ext_d, base_h, corner_r);

        // Hollow interior
        translate([wall, wall, wall])
            rounded_box(ext_w - wall*2, ext_d - wall*2, base_h, corner_r - 0.5);

        // === REAR PANEL CUTOUTS ===
        rear_y = ext_d - wall/2;

        // IEC Power inlet (centered)
        translate([ext_w/2 - iec_w/2, rear_y - 1, wall + internal_h/2 - iec_h/2])
            cube([iec_w, wall + 2, iec_h]);

        // XLR connectors
        xlr_start_x = ext_w/2 - (num_channels * (xlr_diameter + 5)) / 2 + (xlr_diameter + 5)/2;
        for (i = [0:num_channels-1]) {
            translate([xlr_start_x + i * (xlr_diameter + 5), rear_y - 1, base_h/2 + 5])
                rotate([-90, 0, 0])
                    cylinder(d=xlr_diameter, h=wall+2, $fn=32);
        }

        // === FRONT PANEL CUTOUTS ===

        // OLED display window
        translate([ext_w/2 - oled_cutout_w/2, -0.1, base_h - lid_h/2 - oled_cutout_h/2])
            cube([oled_cutout_w, wall + 0.2, oled_cutout_h]);

        // Rotary encoder hole
        translate([ext_w - wall - 15, -0.1, base_h/2])
            rotate([-90, 0, 0])
                cylinder(d=encoder_shaft_d, h=wall+0.2, $fn=24);

        // === SIDE VENTILATION ===
        // Left side
        translate([wall/2, ext_d/2, base_h/2])
            rotate([0, 90, 0])
                vent_slots(5, vent_slot_w, vent_slot_l, wall);

        // Right side
        translate([ext_w - wall/2, ext_d/2, base_h/2])
            rotate([0, 90, 0])
                vent_slots(5, vent_slot_w, vent_slot_l, wall);

        // === BOTTOM VENTILATION ===
        translate([ext_w/2, ext_d/2, 0])
            vent_slots(8, vent_slot_w, vent_slot_l, wall);

        // === LEFT SIDE: DC jack for ESP32 power ===
        translate([-0.1, ext_d/3, base_h/2])
            rotate([0, 90, 0])
                cylinder(d=dc_jack_d, h=wall+0.2, $fn=24);
    }

    // === INTERNAL MOUNTING ===

    // Screw posts for lid attachment (4 corners)
    post_h = base_h - wall;
    post_inset = 8;
    translate([post_inset, post_inset, wall])
        screw_post(post_h, 7, 2.5);
    translate([ext_w - post_inset, post_inset, wall])
        screw_post(post_h, 7, 2.5);
    translate([post_inset, ext_d - post_inset, wall])
        screw_post(post_h, 7, 2.5);
    translate([ext_w - post_inset, ext_d - post_inset, wall])
        screw_post(post_h, 7, 2.5);

    // ESP32 standoffs
    esp_x = wall + internal_padding;
    esp_y = wall + internal_padding;
    translate([esp_x + 2.5, esp_y + 2.5, wall])
        pcb_standoff(pcb_standoff_h);
    translate([esp_x + esp32_w - 2.5, esp_y + 2.5, wall])
        pcb_standoff(pcb_standoff_h);
    translate([esp_x + 2.5, esp_y + esp32_l - 2.5, wall])
        pcb_standoff(pcb_standoff_h);
    translate([esp_x + esp32_w - 2.5, esp_y + esp32_l - 2.5, wall])
        pcb_standoff(pcb_standoff_h);

    // SSR mounting platform
    ssr_x = wall + internal_padding + esp32_w + internal_padding;
    ssr_y = wall + internal_padding;
    translate([ssr_x, ssr_y, wall])
        cube([ssr_w + 2, ssr_l + 2, 2]); // Raised platform
}

// ============================================================
// Lid (top half)
// ============================================================
module lid() {
    difference() {
        rounded_box(ext_w, ext_d, lid_h, corner_r);

        // Hollow interior (lip for stacking on base)
        translate([wall + 0.2, wall + 0.2, -0.1])
            rounded_box(ext_w - wall*2 - 0.4, ext_d - wall*2 - 0.4, lid_h - wall + 0.1, corner_r - 1);

        // Screw holes
        post_inset = 8;
        for (pos = [[post_inset, post_inset],
                     [ext_w - post_inset, post_inset],
                     [post_inset, ext_d - post_inset],
                     [ext_w - post_inset, ext_d - post_inset]]) {
            translate([pos[0], pos[1], -0.1])
                cylinder(d=3, h=lid_h+0.2, $fn=16);
            // Countersink
            translate([pos[0], pos[1], lid_h - 2])
                cylinder(d=6, h=2.1, $fn=16);
        }

        // Top ventilation
        translate([ext_w/2, ext_d/2, lid_h - wall])
            vent_slots(6, vent_slot_w, vent_slot_l * 1.2, wall + 0.1);

        // Model label engraving (0.5mm deep)
        translate([ext_w/2, ext_d - 15, lid_h - 0.4])
            linear_extrude(0.5)
                text(str("ESP-Nail ", num_channels == 1 ? "S" : num_channels == 2 ? "D" : "Q"),
                     size=5, halign="center", valign="center", font="Liberation Sans:style=Bold");
    }
}

// ============================================================
// Assembly / Render
// ============================================================

// Uncomment the part you want to export:

// Full assembly preview
base();
translate([0, 0, base_h + 10])  // Exploded view offset
    lid();

// For printing - uncomment one at a time:
// base();
// translate([0, 0, lid_h]) rotate([180, 0, 0]) lid();  // Lid flipped for printing
