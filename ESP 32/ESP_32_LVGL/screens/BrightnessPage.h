#pragma once
#include "Base.h"

namespace PageBrightness {

#ifndef TFT_BL
#define TFT_BL 21
#endif

// ใช้ LEDC คุมความสว่าง (0..255)
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static inline void blInit(uint32_t freq=5000, uint8_t res=8) { ledcAttach(TFT_BL, freq, res); }
static inline void blWrite(uint32_t duty) { ledcWrite(TFT_BL, duty); }  // duty: 0..255 (res=8)
#else
static const uint8_t BL_CH = 1;
static inline void blInit(uint32_t freq=5000, uint8_t res=8) { ledcSetup(BL_CH, freq, res); ledcAttachPin(TFT_BL, BL_CH); }
static inline void blWrite(uint32_t duty) { ledcWrite(BL_CH, duty); }
#endif

static int val = 255;
static int x0, y0, w=220, h=8, knob=10;

static void drawBar(TFT_eSPI& tft) {
  tft.fillRect(x0, y0- h/2, w, h, TFT_DARKGREY);
  int kx = x0 + (val * (w-1))/255;
  tft.fillCircle(kx, y0, knob, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(UI::COL_LABEL, UI::COL_BG);
  tft.drawString(String(val), x0+w/2, y0+26, 4);
}

static void show(TFT_eSPI& tft) {
  Base::header(tft, "Brightness");
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  blInit(); blWrite(val);
  x0 = (tft.width()-w)/2; y0 = 100;
  drawBar(tft);
  UI::status(tft, "Slide to change backlight (0..255)");
}

static bool handle(TFT_eSPI& tft, int16_t x, int16_t y) {
  if (Base::hitBack(x,y)) return true;
  if (y>=y0-20 && y<=y0+20 && x>=x0 && x<=x0+w) {
    int nv = (int)((long)(x-x0)*255/(w-1));
    nv = nv<0?0:nv>255?255:nv;
    if (nv!=val) { val=nv; blWrite(val); tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG); drawBar(tft); }
  }
  return false;
}
} // namespace PageBrightness
