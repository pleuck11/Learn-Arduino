#pragma once
#include "Base.h"

namespace PagePointer {
static void show(TFT_eSPI& tft) {
  Base::header(tft, "Pointer");
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  tft.setTextColor(UI::COL_LABEL, UI::COL_BG);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Touch the screen to see coordinates", 8, 34, 2);
  UI::status(tft, "Waiting for touch...");
}

static bool handle(TFT_eSPI& tft, int16_t x, int16_t y) {
  if (Base::hitBack(x,y)) return true;
  if (y<24) return false;
  // ล้างพื้นที่กลาง
  tft.fillRect(0,60,tft.width(),tft.height()-76, UI::COL_BG);
  // เป้ากางเขน
  tft.drawFastHLine(0, y, tft.width(), TFT_DARKGREY);
  tft.drawFastVLine(x, 24, tft.height()-24, TFT_DARKGREY);
  tft.fillCircle(x,y,2,TFT_WHITE);
  // แสดงพิกัด
  tft.setTextColor(UI::COL_LABEL, UI::COL_BG);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(String("x=")+x+"  y="+y, tft.width()-6, 34, 2);
  UI::status(tft, "Touch to update");
  return false;
}
} // namespace PagePointer
