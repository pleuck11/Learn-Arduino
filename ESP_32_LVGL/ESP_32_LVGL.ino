#include <Arduino.h>
#include "User_Setup.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "Ui.h"

// ---------- Touch pins ----------
#define T_CS   33
#define T_IRQ  36
#define T_SCLK 25
#define T_MOSI 32
#define T_MISO 39

SPIClass SPI_T(VSPI);
XPT2046_Touchscreen ts(T_CS, T_IRQ);

// ---------- microSD (optional) ----------
#define SD_CS 5

// ---------- RGB LED ----------
#define LED_R 4
#define LED_G 16
#define LED_B 17

// ---------- LDR ----------
#define PIN_LDR 34

// ---------- Speaker (LEDC) ----------
#define SPK_PIN 26
#define SPK_RES 10
#define SPK_CH  0

TFT_eSPI tft;

// touch calibration
int16_t TS_MINX = 200,  TS_MAXX = 3800;
int16_t TS_MINY = 200,  TS_MAXY = 3800;
const uint8_t TFT_ROT = 1;

// LEDC wrappers
#if ESP_ARDUINO_VERSION_MAJOR >= 3
static inline void spkInit(uint32_t initFreq){ ledcAttach(SPK_PIN, initFreq, SPK_RES); }
static inline void spkTone(double f){ ledcWriteTone(SPK_PIN, f); }
static inline void spkNoTone(){ ledcWriteTone(SPK_PIN, 0); }
#else
static inline void spkInit(uint32_t initFreq){ ledcSetup(SPK_CH, initFreq, SPK_RES); ledcAttachPin(SPK_PIN, SPK_CH); }
static inline void spkTone(double f){ ledcWriteTone(SPK_CH, f); }
static inline void spkNoTone(){ ledcWriteTone(SPK_CH, 0); }
#endif

void beep(uint16_t freq = 1800, uint16_t ms = 60) {
  spkTone(freq);
  delay(ms);
  spkNoTone();
}

// ---------- include Sub Pages ----------
#include "screens/Base.h"
#include "screens/ButtonPage.h"
#include "screens/KeyboardPage.h"
#include "screens/SliderPage.h"
#include "screens/PointerPage.h"
#include "screens/BrightnessPage.h"
#include "screens/SettingsPage.h"
#include "screens/DialogPage.h"
#include "screens/TextPage.h"

// ---------- simple screen manager ----------
enum Screen { HOME, BTN, KBD, SLD, PTR, BRI, SETT, DLG, TXT };
Screen screen = HOME;

#ifndef TFT_BL
#define TFT_BL 21
#endif

void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);   // backlight on

  pinMode(LED_R, OUTPUT); pinMode(LED_G, OUTPUT); pinMode(LED_B, OUTPUT);
  digitalWrite(LED_R, LOW); digitalWrite(LED_G, LOW); digitalWrite(LED_B, LOW);

  tft.init();
  tft.setRotation(TFT_ROT);

  SPI_T.begin(T_SCLK, T_MISO, T_MOSI, T_CS);
  ts.begin(SPI_T);
  ts.setRotation(TFT_ROT);

  spkInit(2000);
  beep();

  UI::showHome(tft);
  UI::status(tft, "Ready");
  screen = HOME;
}

void loop() {
  // route touch
  if (ts.touched()) {
    TS_Point p = ts.getPoint();      // raw 0..4095
    int16_t x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    int16_t y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    x = constrain(x, 0, tft.width()-1);
    y = constrain(y, 0, tft.height()-1);

    if (screen == HOME) {
      int id = UI::hit(x, y);
      if (id >= 0) {
        UI::flash(tft, id);
        beep(2200, 40);
        UI::status(tft, UI::label(id));

        switch (id) {
          case 0: PageButton::show(tft);    screen = BTN;  break;
          case 1: PageKeyboard::show(tft);  screen = KBD;  break;
          case 2: PageSlider::show(tft);    screen = SLD;  break;
          case 3: PagePointer::show(tft);   screen = PTR;  break;
          case 4: PageBrightness::show(tft);screen = BRI;  break;
          case 5: PageSettings::draw(tft);  screen = SETT; break;
          case 6: PageDialog::show(tft);    screen = DLG;  break;
          case 7: PageText::show(tft);      screen = TXT;  break;
        }
      }
    } else {
      bool back = false;
      switch (screen) {
        case BTN:  back = PageButton::handle(tft, x, y);      break;
        case KBD:  back = PageKeyboard::handle(tft, x, y);    break;
        case SLD:  back = PageSlider::handle(tft, x, y);      break;
        case PTR:  back = PagePointer::handle(tft, x, y);     break;
        case BRI:  back = PageBrightness::handle(tft, x, y);  break;
        case SETT: back = PageSettings::handle(tft, x, y);    break;
        case DLG:  back = PageDialog::handle(tft, x, y);      break;
        case TXT:  back = PageText::handle(tft, x, y);        break;
        default: break;
      }
      if (back) {
        UI::showHome(tft);
        UI::status(tft, "Ready");
        screen = HOME;
        delay(120); // กันแตะซ้ำ
      }
    }
  }

  // ตัวอย่างอ่าน LDR (log เฉย ๆ)
  static uint32_t ldrTick = 0;
  if (millis() - ldrTick > 400) {
    ldrTick = millis();
    int v = analogRead(PIN_LDR);
    Serial.printf("LDR=%d\n", v);
  }
}
