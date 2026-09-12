#pragma once
#include "Base.h"

namespace PageKeyboard {

static String buf;
static int16_t gx, gy, cellW=28, cellH=28;
static const char* keys[4] = {
  "1234567890",
  "qwertyuiop",
  "asdfghjkl",
  "<zxcvbnm OK"
};

static void drawKeys(TFT_eSPI& tft) {
  // กล่องข้อความ
  tft.fillRect(0,24, tft.width(), 36, UI::COL_BG);
  tft.drawRect(8, 28, tft.width()-16, 28, TFT_DARKGREY);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(UI::COL_LABEL, UI::COL_BG);
  tft.drawString(buf, 14, 42, 2);

  // แป้น
  gy = 70;
  for (int r=0; r<4; ++r) {
    const char* row = keys[r];
    int len = strlen(row);
    int x = 8;
    for (int i=0; i<len; ++i) {
      char c = row[i];
      int w = cellW;
      if (c==' ') { x += cellW/2; continue; }
      if (c=='O' && i+1<len && row[i+1]=='K') { // "OK" ปุ่มกว้าง 2 ช่อง
        w = cellW*2+4;
      }
      tft.drawRoundRect(x, gy, w, cellH, 4, TFT_WHITE);
      tft.setTextDatum(MC_DATUM); tft.setTextColor(TFT_WHITE, UI::COL_BG);
      String s; s += c;
      if (c=='<') s="Bksp";
      if (c=='O' && i+1<len && row[i+1]=='K') s="OK";
      tft.drawString(s, x+w/2, gy+cellH/2, 2);
      x += w + 4;
      if (c=='O' && i+1<len && row[i+1]=='K') { ++i; } // ข้าม 'K'
    }
    gy += cellH + 6;
  }
}

static void show(TFT_eSPI& tft) {
  Base::header(tft, "Keyboard");
  buf = "";
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);
  drawKeys(tft);
  UI::status(tft, "Type: tap keys / '<'=backspace / OK");
}

static bool handle(TFT_eSPI& tft, int16_t px, int16_t py) {
  if (Base::hitBack(px,py)) return true;

  // ตรวจจับการแตะแป้น
  int y = 70;
  for (int r=0; r<4; ++r) {
    const char* row = keys[r];
    int len = strlen(row);
    int x = 8;
    for (int i=0; i<len; ++i) {
      char c = row[i];
      if (c==' ') { x += cellW/2; continue; }
      int w = cellW;
      bool isOK = false;
      if (c=='O' && i+1<len && row[i+1]=='K') { w = cellW*2+4; isOK = true; }

      if (px>=x && px<=x+w && py>=y && py<=y+cellH) {
        if (c=='<') { if (!buf.isEmpty()) buf.remove(buf.length()-1); }
        else if (isOK) { UI::status(tft, ("Entered: " + buf).c_str()); }
        else { buf += c; }
        drawKeys(tft);
        return false;
      }
      x += w + 4;
      if (isOK) { ++i; }
    }
    y += cellH + 6;
  }
  return false;
}
} // namespace PageKeyboard
