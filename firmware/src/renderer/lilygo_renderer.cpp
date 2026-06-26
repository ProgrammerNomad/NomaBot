#include "lilygo_renderer.h"
#include <LovyanGFX.hpp>

// LILYGO T-Display S3 — LovyanGFX pin configuration
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 80000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 39;
      cfg.pin_mosi = 38;
      cfg.pin_miso = -1;
      cfg.pin_dc = 42;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = 41;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.panel_width = 170;
      cfg.panel_height = 320;
      cfg.offset_x = 35;
      cfg.offset_y = 0;
      cfg.offset_rotation = 1;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel.config(cfg);
    }
    setPanel(&_panel);
  }
};

static LGFX _gfx;

bool LilygoRenderer::begin() {
  _gfx.init();
  _gfx.setRotation(0);
  _gfx.setBrightness(128);
  _gfx.fillScreen(TFT_BLACK);
  return true;
}

void LilygoRenderer::beginFrame() {}

void LilygoRenderer::fillScreen(uint16_t color) { _gfx.fillScreen(color); }

void LilygoRenderer::fillRect(int x, int y, int w, int h, uint16_t color) {
  _gfx.fillRect(x, y, w, h, color);
}

void LilygoRenderer::drawText(int x, int y, const char *text, uint16_t color) {
  _gfx.setTextColor(color);
  _gfx.setTextSize(1);
  _gfx.setCursor(x, y);
  _gfx.print(text);
}

void LilygoRenderer::endFrame() {}

int LilygoRenderer::width() const { return _gfx.width(); }

int LilygoRenderer::height() const { return _gfx.height(); }
