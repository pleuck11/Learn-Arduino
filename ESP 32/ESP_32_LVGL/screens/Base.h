#pragma once
#include <TFT_eSPI.h>
#include "../Ui.h"

namespace Base {

// ขอบเขตปุ่มย้อนกลับ (มุมซ้ายบน)
static inline bool hitBack(int16_t x, int16_t y) {
  return (x >= 0 && x <= 64 && y >= 0 && y <= 24);
}

static inline void header(TFT_eSPI& tft, const char* title) {
  tft.fillRect(0, 0, tft.width(), 24, UI::COL_TOPBAR);
  // ปุ่มย้อนกลับ
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_WHITE, UI::COL_TOPBAR);
  tft.drawString("<  Home", 6, 12, 2);
  // ชื่อหน้าตรงกลาง
  tft.setTextDatum(MC_DATUM);
  tft.drawString(title, tft.width()/2, 12, 2);
}

} // namespace Base
