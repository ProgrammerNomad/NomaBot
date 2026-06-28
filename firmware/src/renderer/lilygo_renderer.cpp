#include "lilygo_renderer.h"
#include <LovyanGFX.hpp>

// LILYGO T-Display S3 - ST7789 on 8-bit parallel bus (not SPI).
// Pin map: https://wiki.lilygo.cc/products/t-display-series/t-display-s3/
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_Parallel8 _bus;
  lgfx::Light_PWM _light;

public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.freq_write = 20000000;
      cfg.pin_wr = 8;
      cfg.pin_rd = 9;
      cfg.pin_rs = 7;
      cfg.pin_d0 = 39;
      cfg.pin_d1 = 40;
      cfg.pin_d2 = 41;
      cfg.pin_d3 = 42;
      cfg.pin_d4 = 45;
      cfg.pin_d5 = 46;
      cfg.pin_d6 = 47;
      cfg.pin_d7 = 48;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = 6;
      cfg.pin_rst = 5;
      cfg.pin_busy = -1;
      cfg.panel_width = 170;
      cfg.panel_height = 320;
      cfg.offset_x = 35;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel.config(cfg);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl = 38;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 0;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    setPanel(&_panel);
  }
};

static LGFX _gfx;
static constexpr int PIN_POWER_ON = 15;

bool LilygoRenderer::begin() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  delay(50);

  _gfx.init();
  _gfx.setRotation(0);
  _gfx.setBrightness(255);
  _gfx.fillScreen(0x0000);
  return true;
}

void LilygoRenderer::fillScreen(uint16_t color) { _gfx.fillScreen(color); }

void LilygoRenderer::blitRGB565(const uint16_t *pixels, int x, int y, int w, int h) {
  if (!pixels || w <= 0 || h <= 0) {
    return;
  }
  _gfx.pushImage(x, y, w, h, pixels);
}

void LilygoRenderer::blitRGB565ColorKey(const uint16_t *pixels, int x, int y, int w, int h,
                                        uint16_t colorKey) {
  if (!pixels || w <= 0 || h <= 0) {
    return;
  }
  _gfx.pushImage(x, y, w, h, pixels, colorKey);
}

void LilygoRenderer::fillRect(int x, int y, int w, int h, uint16_t color) {
  _gfx.fillRect(x, y, w, h, color);
}

void LilygoRenderer::drawText(int x, int y, const char *text, uint16_t color) {
  _gfx.setTextColor(color);
  _gfx.setTextSize(1);
  _gfx.setCursor(x, y);
  _gfx.print(text);
}

int LilygoRenderer::width() const { return _gfx.width(); }

int LilygoRenderer::height() const { return _gfx.height(); }
