#pragma once
#include "Base.h"

namespace PageButton {
static bool on = false;
static int16_t bx, by, bw=140, bh=56, r=12;

static void draw(TFT_eSPI& tft) {
  Base::header(tft, "Button");
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  bx = (tft.width() - bw)/2;
  by = 60;
  uint16_t col = on ? TFT_GREEN : TFT_DARKGREY;
  tft.fillRoundRect(bx,by,bw,bh,r,col);
  tft.drawRoundRect(bx,by,bw,bh,r,TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE,col);
  tft.drawString(on?"ON":"OFF", bx+bw/2, by+bh/2, 4);
  UI::status(tft, on?"Button: ON":"Button: OFF");
}

static void show(TFT_eSPI& tft) { draw(tft); }

static bool handle(TFT_eSPI& tft, int16_t x, int16_t y) {
  if (Base::hitBack(x,y)) return true;
  if (x>=bx && x<=bx+bw && y>=by && y<=by+bh) { on = !on; draw(tft); }
  return false;
}
} // namespace PageButton
