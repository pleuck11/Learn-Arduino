#pragma once
#include "Base.h"

namespace PageDialog {
static int16_t x,y,w=200,h=100;
static void drawBox(TFT_eSPI& tft) {
  x=(tft.width()-w)/2; y=60;
  tft.fillRoundRect(x,y,w,h,8, TFT_NAVY);
  tft.drawRoundRect(x,y,w,h,8, TFT_WHITE);
  tft.setTextDatum(MC_DATUM); tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawString("Proceed?", x+w/2, y+30, 2);
  // buttons
  tft.fillRoundRect(x+18, y+60, 70, 26, 6, TFT_DARKGREEN);
  tft.fillRoundRect(x+w-18-70, y+60, 70, 26, 6, TFT_MAROON);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("OK",     x+18+35, y+60+13, 2);
  tft.drawString("Cancel", x+w-18-35, y+60+13, 2);
}

static void show(TFT_eSPI& tft) {
  Base::header(tft, "Dialog box");
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  drawBox(tft);
  UI::status(tft, "OK / Cancel");
}

static bool handle(TFT_eSPI& tft, int16_t px, int16_t py) {
  if (Base::hitBack(px,py)) return true;
  // OK
  if (px>=x+18 && px<=x+18+70 && py>=y+60 && py<=y+60+26) {
    UI::status(tft, "Result: OK");
  }
  // Cancel
  if (px>=x+w-18-70 && px<=x+w-18 && py>=y+60 && py<=y+60+26) {
    UI::status(tft, "Result: Cancel");
  }
  return false;
}
} // namespace PageDialog
