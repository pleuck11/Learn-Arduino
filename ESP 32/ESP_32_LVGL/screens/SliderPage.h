#pragma once
#include "Base.h"

namespace PageSlider {
static int val = 50;
static int x0, y0, w=220, h=8, knob=10;

static void drawBar(TFT_eSPI& tft) {
  tft.fillRect(x0, y0- h/2, w, h, TFT_DARKGREY);
  int kx = x0 + (val * (w-1))/100;
  tft.fillCircle(kx, y0, knob, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(UI::COL_LABEL, UI::COL_BG);
  tft.drawString(String(val), x0+w/2, y0+26, 4);
}

static void show(TFT_eSPI& tft) {
  Base::header(tft, "Slider");
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  x0 = (tft.width()-w)/2; y0 = 100;
  drawBar(tft);
  UI::status(tft, "Slide anywhere on the bar");
}

static bool handle(TFT_eSPI& tft, int16_t x, int16_t y) {
  if (Base::hitBack(x,y)) return true;
  // แก้ค่าเมื่อแตะบริเวณสไลเดอร์
  if (y>=y0-20 && y<=y0+20 && x>=x0 && x<=x0+w) {
    int nv = (int)((long)(x-x0)*100/(w-1));
    nv = nv<0?0:nv>100?100:nv;
    if (nv!=val) { val=nv; tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG); drawBar(tft); }
  }
  return false;
}
} // namespace PageSlider
