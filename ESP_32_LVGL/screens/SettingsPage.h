#pragma once
#include "Base.h"

namespace PageSettings {

static int16_t b1x,b1y,bw=120,bh=40;
static int16_t b2x,b2y;

static void draw(TFT_eSPI& tft) {
  Base::header(tft, "Settings");
  tft.fillRect(0,24,tft.width(),tft.height()-24, UI::COL_BG);

  b1x = (tft.width()-bw)/2; b1y = 70;
  b2x = b1x;                b2y = b1y + 60;

  tft.fillRoundRect(b1x,b1y,bw,bh,8, UI::C5);
  tft.drawRoundRect(b1x,b1y,bw,bh,8,TFT_WHITE);
  tft.fillRoundRect(b2x,b2y,bw,bh,8, UI::C6);
  tft.drawRoundRect(b2x,b2y,bw,bh,8,TFT_WHITE);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("RGB cycle", b1x+bw/2, b1y+bh/2, 2);
  tft.drawString("Beep",      b2x+bw/2, b2y+bh/2, 2);

  UI::status(tft, "Tap a setting");
}

static void rgbCycle() {
  pinMode(4,OUTPUT); pinMode(16,OUTPUT); pinMode(17,OUTPUT);
  for (int i=0;i<2;i++) {
    digitalWrite(4,HIGH);  delay(120); digitalWrite(4,LOW);
    digitalWrite(16,HIGH); delay(120); digitalWrite(16,LOW);
    digitalWrite(17,HIGH); delay(120); digitalWrite(17,LOW);
  }
}

// NOTE: อย่าประกาศ extern beep() ใน namespace นี้
// ถ้าต้องการประกาศล่วงหน้า สามารถประกาศไว้ "นอก namespace" ก็ได้:
// extern void beep(uint16_t freq, uint16_t ms);

static bool handle(TFT_eSPI& tft, int16_t x, int16_t y) {
  if (Base::hitBack(x,y)) return true;

  if (x>=b1x && x<=b1x+bw && y>=b1y && y<=b1y+bh) {
    rgbCycle();
    UI::status(tft,"RGB cycled");
  }
  if (x>=b2x && x<=b2x+bw && y>=b2y && y<=b2y+bh) {
    ::beep(2000,80);                 // <<< เรียกฟังก์ชัน global จาก .ino
    UI::status(tft,"Beep!");
  }
  return false;
}
} // namespace PageSettings
