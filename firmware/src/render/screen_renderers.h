#pragma once

#include "net/service_status.h"
#include "render/render_state.h"
#include "renderer/renderer.hpp"

void drawClockScreen(IRenderer &renderer, const RenderState &state, const ServiceStatus &status);
void drawWeatherScreen(IRenderer &renderer, const RenderState &state, const ServiceStatus &status);
void drawPomodoroScreen(IRenderer &renderer, const RenderState &state);
void drawStatsScreen(IRenderer &renderer, const RenderState &state);
void drawServiceStatusDots(IRenderer &renderer, const ServiceStatus &status);
void drawModeTransitionFade(IRenderer &renderer, uint8_t alpha);
