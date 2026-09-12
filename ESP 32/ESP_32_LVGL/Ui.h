#pragma once
#include <TFT_eSPI.h>
#include <math.h>

// -------------------- THEME / COLORS --------------------
static inline uint16_t RGB565(uint8_t r,uint8_t g,uint8_t b){
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b) >> 3);
}

namespace UI {

static const uint16_t COL_BG      = RGB565(  8, 12, 18);
static const uint16_t COL_TOPBAR  = RGB565( 10, 15, 20);
static const uint16_t COL_LABEL   = RGB565(210,214,220);

// โทนสีปุ่ม 8 ปุ่ม
static const uint16_t C0 = RGB565( 79,195,247); // Button
static const uint16_t C1 = RGB565(246,103,103); // Keyboard
static const uint16_t C2 = RGB565(129,212,250); // Slider
static const uint16_t C3 = RGB565(116,173,255); // Pointer
static const uint16_t C4 = RGB565(255,179, 71); // Brightness
static const uint16_t C5 = RGB565(  0,188,212); // Settings
static const uint16_t C6 = RGB565(255,205, 64); // Dialog
static const uint16_t C7 = RGB565(255,111, 97); // Text

// -------------------- LAYOUT --------------------
struct Item {
  int16_t cx, cy, r;
  const char* label;
  uint16_t bg;
};
static Item items[8];

static void layout(TFT_eSPI& tft) {
  const int topBarH = 24;
  const int marginX = 40;
  const int dx = 80;
  const int y1 = topBarH + 50;
  const int y2 = y1 + 92;
  const int r  = 30;
  const int xs[4] = { marginX, marginX+dx, marginX+2*dx, marginX+3*dx };

  const char* names[8] = {
    "Button","Keyboard","Slider","Pointer",
    "Brightness","Settings","Dialog box","Text"
  };
  const uint16_t cols[8] = { C0,C1,C2,C3,C4,C5,C6,C7 };

  for (int i=0;i<4;i++){
    items[i]   = { (int16_t)xs[i], (int16_t)y1, (int16_t)r, names[i],   cols[i] };
    items[i+4] = { (int16_t)xs[i], (int16_t)y2, (int16_t)r, names[i+4], cols[i+4] };
  }
}

// -------------------- TOP BAR --------------------
static void drawWifi(TFT_eSPI& tft) {
  const int x = tft.width() - 26;
  const int y = 12;
  tft.drawCircle(x,y,10,TFT_WHITE);
  tft.drawCircle(x,y, 7,TFT_WHITE);
  tft.drawCircle(x,y, 4,TFT_WHITE);
  tft.fillRect(x-11, y, 22, 12, COL_TOPBAR);   // ปิดครึ่งล่างให้เหลือเป็นโค้ง
  tft.fillCircle(x, y+2, 2, TFT_WHITE);
}

static void drawTopBar(TFT_eSPI& tft) {
  tft.fillRect(0,0,tft.width(),24, COL_TOPBAR);
  drawWifi(tft);
}

// -------------------- ICONS --------------------
static void iconButton (TFT_eSPI& tft,int cx,int cy){
  tft.fillRoundRect(cx-11,cy-8,22,16,5,TFT_WHITE);
  tft.drawFastHLine(cx-6, cy-1, 12, RGB565(200,200,200));
}
static void iconKeyboard(TFT_eSPI& tft,int cx,int cy){
  for(int r=0;r<3;r++) for(int c=0;c<4;c++)
    tft.drawRect(cx-14 + c*7, cy-11 + r*7, 6,6, TFT_WHITE);
}
static void iconSlider  (TFT_eSPI& tft,int cx,int cy){
  tft.drawFastHLine(cx-18, cy, 36, TFT_WHITE);
  tft.fillCircle(cx+6, cy, 6, TFT_WHITE);
}
static void iconPointer (TFT_eSPI& tft,int cx,int cy){
  for(int r=0;r<2;r++) for(int c=0;c<2;c++)
    tft.drawRect(cx-12 + c*12, cy-12 + r*12, 10,10, TFT_WHITE);
}
static void iconSun     (TFT_eSPI& tft,int cx,int cy){
  tft.fillCircle(cx,cy,7,TFT_WHITE);
  for(int i=0;i<8;i++){
    float a = i*3.1415926f/4.0f;
    int x1 = cx + (int)(cosf(a)*12), y1 = cy + (int)(sinf(a)*12);
    int x2 = cx + (int)(cosf(a)*16), y2 = cy + (int)(sinf(a)*16);
    tft.drawLine(x1,y1,x2,y2,TFT_WHITE);
  }
}
static void iconGear    (TFT_eSPI& tft,int cx,int cy){
  tft.drawCircle(cx,cy,10,TFT_WHITE);
  for(int i=0;i<6;i++){
    float a = i*3.1415926f/3.0f;
    tft.fillCircle(cx + (int)(cosf(a)*12), cy + (int)(sinf(a)*12), 2, TFT_WHITE);
  }
  tft.fillCircle(cx,cy,3,TFT_WHITE);
}
static void iconDialog  (TFT_eSPI& tft,int cx,int cy){
  tft.drawRoundRect(cx-16, cy-10, 32, 18, 4, TFT_WHITE);
  tft.fillTriangle(cx-6,cy+8, cx-1,cy+8, cx-4,cy+14, TFT_WHITE);
}
static void iconText    (TFT_eSPI& tft,int cx,int cy){
  tft.drawRoundRect(cx-12, cy-14, 24, 28, 3, TFT_WHITE);
  tft.drawFastHLine(cx-8, cy-6, 16, TFT_WHITE);
  tft.drawFastHLine(cx-8, cy,   16, TFT_WHITE);
  tft.drawFastHLine(cx-8, cy+6, 10, TFT_WHITE);
}

// -------------------- DRAW ITEM --------------------
static void drawItem(TFT_eSPI& tft, const Item& it, int idx){
  tft.fillCircle(it.cx, it.cy, it.r, it.bg);
  tft.drawCircle(it.cx, it.cy, it.r, RGB565(20,20,30));

  switch(idx){
    case 0: iconButton(tft,it.cx,it.cy);  break;
    case 1: iconKeyboard(tft,it.cx,it.cy);break;
    case 2: iconSlider(tft,it.cx,it.cy);  break;
    case 3: iconPointer(tft,it.cx,it.cy); break;
    case 4: iconSun(tft,it.cx,it.cy);     break;
    case 5: iconGear(tft,it.cx,it.cy);    break;
    case 6: iconDialog(tft,it.cx,it.cy);  break;
    case 7: iconText(tft,it.cx,it.cy);    break;
  }

  tft.setTextColor(COL_LABEL, COL_BG);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(it.label, it.cx, it.cy + it.r + 12, 2);
}

// -------------------- PUBLIC API --------------------
static void showHome(TFT_eSPI& tft){
  tft.fillScreen(COL_BG);
  drawTopBar(tft);
  layout(tft);
  for (int i=0;i<8;i++) drawItem(tft, items[i], i);
}

static int hit(int16_t x, int16_t y){
  for (int i=0;i<8;i++){
    int dx = x - items[i].cx, dy = y - items[i].cy;
    if (dx*dx + dy*dy <= items[i].r * items[i].r) return i; // index 0..7
  }
  return -1;
}

// แถบสถานะมุมล่าง
static void status(TFT_eSPI& tft, const char* txt){
  tft.fillRect(0, tft.height()-16, tft.width(), 16, COL_BG);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(COL_LABEL, COL_BG);
  tft.drawString(txt, 4, tft.height()-2, 2);
}

// คืนชื่อเมนูจาก index
static const char* label(int idx){
  if (idx < 0 || idx > 7) return "";
  return items[idx].label;
}

// เอฟเฟกต์กดปุ่ม (วงแหวนกระพริบสั้น ๆ แล้ววาดปุ่มคืน)
static void flash(TFT_eSPI& tft, int idx){
  if (idx < 0 || idx > 7) return;
  tft.drawCircle(items[idx].cx, items[idx].cy, items[idx].r + 3, TFT_WHITE);
  delay(120);
  tft.drawCircle(items[idx].cx, items[idx].cy, items[idx].r + 3, COL_BG);
  drawItem(tft, items[idx], idx);
}

} // namespace UI
