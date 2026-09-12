#pragma once
#include "Base.h"

namespace PageText {
static int lines = 0;

static void paint(TFT_eSPI& tft) {
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  tft.setTextColor(UI::COL_LABEL, UI::COL_BG);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Sample text view", 8, 34, 2);
  for (int i=0;i<lines; ++i) {
    tft.drawString(String("Line #")+String(i+1), 8, 56 + i*16, 2);
  }
}

static void show(TFT_eSPI& tft) {
  Base::header(tft, "Text");
  lines = 4;
  paint(tft);
  UI::status(tft, "Tap to add a line (max 9)");
}

static bool handle(TFT_eSPI& tft, int16_t x, int16_t y) {
  if (Base::hitBack(x,y)) return true;
  if (y>24) {
    if (lines<9) lines++;
    paint(tft);
  }
  return false;
}
} // namespace PageText
