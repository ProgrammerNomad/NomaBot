#pragma once

#include <cstdint>

class IRenderer;

// Estimate text width in pixels assuming 6px-wide monospace glyphs.
int textWidthEstimate(const char *text);

// Compute the largest text scale that fits within maxWidth, capped at preferredScale.
uint8_t fitTextScale(const char *text, int maxWidth, uint8_t preferredScale);

// Draw text centered horizontally on screen at the given scale.
void drawScaledCenteredLine(IRenderer &renderer, const char *text, int y, uint16_t color,
                            uint8_t scale);

// Draw up to 3 lines of large centered text (used by clock, weather, pomodoro, stats screens).
void drawLargeCenteredText(IRenderer &renderer, const char *line1, const char *line2,
                           const char *line3);
