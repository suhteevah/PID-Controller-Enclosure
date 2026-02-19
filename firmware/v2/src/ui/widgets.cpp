#include "ui/widgets.h"
#include "config.h"

namespace ui {

// ---------------------------------------------------------------------------
// drawTempBar - filled portion proportional to currentTemp / targetTemp
// ---------------------------------------------------------------------------
void drawTempBar(DisplayDriver* d, int16_t x, int16_t y, int16_t w, int16_t h,
                 float currentTemp, float targetTemp) {
    // Outer frame
    d->drawRect(x, y, w, h);

    if (targetTemp <= 0.0f) {
        return; // avoid division by zero
    }

    float ratio = currentTemp / targetTemp;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    int16_t fillW = static_cast<int16_t>((w - 2) * ratio);
    if (fillW > 0) {
        d->fillRect(x + 1, y + 1, fillW, h - 2);
    }

    // Draw a single-pixel target marker at the right inner edge
    d->drawLine(x + w - 2, y + 1, x + w - 2, y + h - 2);
}

// ---------------------------------------------------------------------------
// drawStatusIcon - 3-char state mnemonic at (x, y), text size 1
// ---------------------------------------------------------------------------
void drawStatusIcon(DisplayDriver* d, int16_t x, int16_t y, ChannelState state) {
    const char* label;
    switch (state) {
        case ChannelState::OFF:       label = "OFF"; break;
        case ChannelState::HEATING:   label = "HEA"; break;
        case ChannelState::HOLDING:   label = "HLD"; break;
        case ChannelState::COOLDOWN:  label = "COL"; break;
        case ChannelState::AUTOTUNE:  label = "TUN"; break;
        case ChannelState::FAULT:     label = "FLT"; break;
        default:                      label = "???"; break;
    }

    d->setTextSize(1);
    d->setCursor(x, y);
    d->print(label);
}

// ---------------------------------------------------------------------------
// drawMenuItem - full-width row, inverted when selected
// ---------------------------------------------------------------------------
void drawMenuItem(DisplayDriver* d, int16_t y, const char* text, bool selected) {
    d->setTextSize(1);

    if (selected) {
        d->fillRect(0, y, static_cast<int16_t>(d->width()), 10);
        d->setInvertText(true);
        d->setCursor(2, y + 1);
        d->print(text);
        d->setInvertText(false);
    } else {
        d->setCursor(2, y + 1);
        d->print(text);
    }
}

// ---------------------------------------------------------------------------
// drawProgressBar - generic bar clamped to [0, 100]
// ---------------------------------------------------------------------------
void drawProgressBar(DisplayDriver* d, int16_t x, int16_t y, int16_t w, int16_t h,
                     float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    d->drawRect(x, y, w, h);

    int16_t fillW = static_cast<int16_t>((w - 2) * (percent / 100.0f));
    if (fillW > 0) {
        d->fillRect(x + 1, y + 1, fillW, h - 2);
    }
}

// ---------------------------------------------------------------------------
// drawHeader - inverted banner across the top 10 pixels
// ---------------------------------------------------------------------------
void drawHeader(DisplayDriver* d, const char* text) {
    int16_t screenW = static_cast<int16_t>(d->width());
    d->fillRect(0, 0, screenW, 10);

    // Approximate centring: each size-1 char is 6 px wide
    int16_t textLen = static_cast<int16_t>(strlen(text));
    int16_t textPixelW = textLen * 6;
    int16_t cx = (screenW - textPixelW) / 2;
    if (cx < 0) cx = 0;

    d->setTextSize(1);
    d->setInvertText(true);
    d->setCursor(cx, 1);
    d->print(text);
    d->setInvertText(false);
}

// ---------------------------------------------------------------------------
// drawFooter - inverted banner across the bottom 10 pixels
// ---------------------------------------------------------------------------
void drawFooter(DisplayDriver* d, const char* text) {
    int16_t screenW = static_cast<int16_t>(d->width());
    int16_t screenH = static_cast<int16_t>(d->height());
    int16_t footerY = screenH - 10;

    d->fillRect(0, footerY, screenW, 10);

    int16_t textLen = static_cast<int16_t>(strlen(text));
    int16_t textPixelW = textLen * 6;
    int16_t cx = (screenW - textPixelW) / 2;
    if (cx < 0) cx = 0;

    d->setTextSize(1);
    d->setInvertText(true);
    d->setCursor(cx, footerY + 1);
    d->print(text);
    d->setInvertText(false);
}

} // namespace ui
