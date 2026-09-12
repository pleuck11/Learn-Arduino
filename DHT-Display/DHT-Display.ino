#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// ---------- พิน ----------
#define I2C_SDA   21
#define I2C_SCL   22
#define DHTPIN    4
#define DHTTYPE   DHT22

// ถ้าขอบขวายังเพี้ยนมาก ลองสลับเป็น 1 (บางจอคือ SH1106)
#define DISPLAY_DRIVER 1
#define OLED_ADDR 0x3C

#if DISPLAY_DRIVER == 0
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#else
U8G2_SH1106_128X64_NONAME_F_HW_I2C   u8g2(U8G2_R0, U8X8_PIN_NONE);
#endif

DHT dht(DHTPIN, DHTTYPE);

// ---------- Layout ----------
const int MARGIN_X = 4;
const int TEXT_X   = MARGIN_X;
const int BAR_X    = MARGIN_X;
const int BAR_W    = 120;     // เว้นขอบขวา
const int BAR_H    = 8;

unsigned long lastMs = 0;
const unsigned long INTERVAL_MS = 2500;
float lastT = NAN, lastH = NAN;

// ====== แถบ "สองโทน" แบบไม่มีขอบขวา ======
void drawTwoToneBar_NoRight(int x, int y, int w, int h, int filled, uint8_t stripeGap = 2) {
  // จำกัดไม่ให้ชนพิกเซลขวาสุดของแท่ง
  int limit = w - 1;                      // เหลือช่องว่างด้านขวาไว้ 1 px
  int fillw = filled; if (fillw > limit) fillw = limit;

  // ลายแนวตั้ง (โทนอ่อน) — ไม่แตะขอบขวา
  for (int xi = x; xi < x + limit; xi += stripeGap) {
    u8g2.drawVLine(xi, y, h);
  }
  // ส่วนทึบ (โทนเข้มตามค่า) — ไม่แตะขอบขวา
  if (fillw > 0) u8g2.drawBox(x, y, fillw, h);

  // เส้นบน-ล่าง-ซ้าย (ไม่มีขวา)
  u8g2.drawHLine(x,       y,     limit);   // top
  u8g2.drawHLine(x,       y+h-1, limit);   // bottom
  u8g2.drawVLine(x,       y,     h);       // left only
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  u8g2.setI2CAddress(OLED_ADDR * 2);
  u8g2.begin();

  dht.begin();
  delay(2000);

  u8g2.clearBuffer();           // พื้นดำสนิท
  u8g2.setDrawColor(1);         // วาดเป็นสีขาว
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(TEXT_X, 12, "ESP32 + DHT22 + OLED");
  u8g2.drawStr(TEXT_X, 28, "No right border bars");
  u8g2.sendBuffer();
}

void drawScreen(float t, float h, bool valid) {
  u8g2.clearBuffer();           // ดำสนิท
  u8g2.setDrawColor(1);

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(TEXT_X, 10, "DHT22 Readout");

  char line[24];
  if (isnan(t)) snprintf(line, sizeof(line), "T: --.- C");
  else          snprintf(line, sizeof(line), "T: %.1f C", t);
  u8g2.drawStr(TEXT_X, 26, line);

  if (isnan(h)) snprintf(line, sizeof(line), "H: --.- %%");
  else          snprintf(line, sizeof(line), "H: %.1f %%", h);
  u8g2.drawStr(TEXT_X, 38, line);

  // map ค่าเป็นความยาวแท่ง โดยไม่แตะขวา
  int tBar = isnan(t) ? 0 : constrain(map((int)(t*10), 0, 500, 0, BAR_W), 0, BAR_W);
  int hBar = isnan(h) ? 0 : constrain(map((int)(h*10), 0, 1000, 0, BAR_W), 0, BAR_W);

  // วาดแถบสองโทน (ไม่มีขอบขวา)
  drawTwoToneBar_NoRight(BAR_X, 46, BAR_W, BAR_H, tBar, 2); // T: ลายถี่
  drawTwoToneBar_NoRight(BAR_X, 56, BAR_W, BAR_H, hBar, 3); // H: ลายห่าง

  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(100, 10, valid ? "OK" : "...");

  u8g2.sendBuffer();
}

void loop() {
  unsigned long now = millis();
  if (now - lastMs >= INTERVAL_MS) {
    lastMs = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    bool valid = !(isnan(h) || isnan(t));

    if (valid) { lastH = h; lastT = t; Serial.printf("T=%.1fC  H=%.1f%%\n", t, h); }
    else        { Serial.println("DHT read failed"); }

    drawScreen(lastT, lastH, valid);
  }
}
