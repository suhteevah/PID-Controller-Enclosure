#pragma once

#include <Arduino.h>
#include "drivers/display_ssd1306.h"
#include "core/channel.h"

namespace ui {

// Draw a horizontal temperature bar showing current vs target.
// The bar outline spans (x,y) to (x+w, y+h). The filled portion
// represents currentTemp relative to targetTemp.
void drawTempBar(DisplayDriver* d, int16_t x, int16_t y, int16_t w, int16_t h,
                 float currentTemp, float targetTemp);

// Draw a small status icon/label for the given channel state at (x,y).
// Renders a 3-character mnemonic: OFF, HEA, HLD, COL, TUN, FLT.
void drawStatusIcon(DisplayDriver* d, int16_t x, int16_t y, ChannelState state);

// Draw a single menu row at vertical position y, full display width.
// If selected is true the row is drawn with inverted colors.
void drawMenuItem(DisplayDriver* d, int16_t y, const char* text, bool selected);

// Draw a generic horizontal progress bar at (x,y) with dimensions w x h.
// percent is clamped to [0, 100].
void drawProgressBar(DisplayDriver* d, int16_t x, int16_t y, int16_t w, int16_t h,
                     float percent);

// Draw a header bar across the top of the screen with centred text.
// Fills a 10-pixel-high band and renders text inverted.
void drawHeader(DisplayDriver* d, const char* text);

// Draw a footer bar across the bottom of the screen with centred text.
// Fills a 10-pixel-high band at the bottom and renders text inverted.
void drawFooter(DisplayDriver* d, const char* text);

} // namespace ui
