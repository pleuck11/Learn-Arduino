/*
ESP32 + OLED 128x32 (SH1106 tuned, no right-edge line)
- ใช้ U8g2 + ไดรเวอร์ SH1106 (แก้ปัญหาเส้นขอบขวา)
- ปรับ Contrast, I2C clock, margin, และเลื่อนตาขวาเข้ามา 1–2px
- สุ่มอารมณ์ + สุ่มเวลาค้าง: HAPPY 3–5s, SLEEPY 7–10s, SURPRISED 1–2s
*/

#include <Wire.h>
#include <U8g2lib.h>

constexpr int SDA_PIN = 21;
constexpr int SCL_PIN = 22;

// ===== เลือก “ตัวเดียว” =====
// รุ่นจอที่พบเส้นขอบขวาบ่อยคือ SH1106 → ใช้อันนี้
U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// ถ้าจอคุณเป็น SSD1306 จริง ๆ ให้คอมเมนต์บรรทัดบน แล้วปลดบรรทัดล่าง
// U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== จอ & ระยะปลอดภัย =====
const int W = 128, H = 32;
const int MARGIN = 1;             // กันชนรอบจอ
const int MAXX = W - 1 - MARGIN;
const int MAXY = H - 1 - MARGIN;

// ===== พารามิเตอร์ดวงตา (สี่เหลี่ยมมุมโค้งสไตล์ EMO) =====
const int eyeW = 24, eyeH = 20, eyeY = 6;
const int leftX  = 28;
const int rightX = 74;   // จูน: เลื่อนเข้าซ้าย 2px กันขอบขวา
const int corner = 5;

// ===== ชุดอารมณ์ =====
enum Mood {
  NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY, WINK, LOVE,
  LAUGH, CRY, SHY, CONFUSED, COOL, THINKING, SCARED, SMIRK, DEAD, WOW,
  MOOD_COUNT
};
Mood mood = NEUTRAL;

// ===== ตัวแปรเวลาแบบสุ่ม =====
unsigned long lastMoodChange=0, nextMoodDelay=8000;
unsigned long lastBlink=0, nextBlinkDelay=1800, blinkDuration=110;
unsigned long lastNystag=0, nextNystagDelay=220;
int pupilOffset=0;

// ===== Utils (clamp & safe draw) =====
inline int clampi(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }
inline int inX(int x){ return clampi(x, MARGIN, MAXX); }
inline int inY(int y){ return clampi(y, MARGIN, MAXY); }

void safeLine(int x1,int y1,int x2,int y2){
  x1=inX(x1); y1=inY(y1); x2=inX(x2); y2=inY(y2);
  u8g2.drawLine(x1,y1,x2,y2);
}
void safeBox(int x,int y,int w,int h){
  x=inX(x); y=inY(y);
  w=clampi(w,1,MAXX-x+1);
  h=clampi(h,1,MAXY-y+1);
  u8g2.drawBox(x,y,w,h);
}
void safeRBox(int x,int y,int w,int h,int r){
  x=inX(x); y=inY(y);
  w=clampi(w,1,MAXX-x+1);
  h=clampi(h,1,MAXY-y+1);
  r=clampi(r,0,10);
  u8g2.drawRBox(x,y,w,h,r);
}
void safeDisc(int cx,int cy,int r){
  cx=inX(cx); cy=inY(cy);
  r=clampi(r,0, min(min(cx-MARGIN,MAXX-cx), min(cy-MARGIN,MAXY-cy)));
  if(r>0) u8g2.drawDisc(cx,cy,r);
}
void safeCircle(int cx,int cy,int r){
  cx=inX(cx); cy=inY(cy);
  r=clampi(r,0, min(min(cx-MARGIN,MAXX-cx), min(cy-MARGIN,MAXY-cy)));
  if(r>0) u8g2.drawCircle(cx,cy,r,U8G2_DRAW_ALL);
}
void safeXBMP8(int cx,int cy,const uint8_t bmp[8]){
  int x = inX(cx-4), y = inY(cy-4);
  if (x+8 <= MAXX && y+8 <= MAXY) u8g2.drawXBMP(x,y,8,8,bmp);
}

// ===== องค์ประกอบใบหน้า =====
void eyeOpenBoxSafe(int x,int y,int w,int h,int r){
  safeRBox(x,y,w,h,r);
  u8g2.setDrawColor(0);
  safeRBox(x+6,y+5, w-12, h-10, r/2); // เจาะช่องในให้ดูเรืองขอบ
  u8g2.setDrawColor(1);
}
void eyeClosedLineSafe(int x,int y,int w){
  int cy = y + eyeH/2;
  for(int dy=-1; dy<=1; dy++) safeLine(x,cy+dy,x+w,cy+dy);
}
void drawOneEye(int x,bool closed,int offsetX){
  if(closed){ eyeClosedLineSafe(x, eyeY, eyeW); return; }
  eyeOpenBoxSafe(x, eyeY, eyeW, eyeH, corner);
  safeBox(x + eyeW/2 - 3 + offsetX, eyeY + eyeH/2 - 3, 6, 6); // รูม่านตา
}
void eyebrow(int x1,int y1,int x2,int y2){
  for(int dy=0; dy<2; dy++) safeLine(x1,y1+dy,x2,y2+dy);
}
void heart8(int cx,int cy){
  static const uint8_t bmp[8]={0x00,0x18,0x3C,0x7E,0x7E,0x3C,0x18,0x00};
  safeXBMP8(cx,cy,bmp);
}
void tearDrop(int x,int y){ safeBox(x, y, 1, 4); }
void sweat(int x,int y){ safeBox(x, y, 2, 3); }
void starEye(int cx,int cy){
  safeLine(cx-3,cy,cx+3,cy);
  safeLine(cx,cy-3,cx,cy+3);
  safeLine(cx-2,cy-2,cx+2,cy+2);
  safeLine(cx-2,cy+2,cx+2,cy-2);
}
void sunglasses(){ safeRBox(26,10,76,12,3); safeLine(26,12,20,12); safeLine(102,12,108,12); }

// ===== วาดใบหน้าตามอารมณ์ =====
void drawFace(Mood m,bool blink,int s){
  u8g2.clearBuffer();

  switch(m){
    case NEUTRAL:
      drawOneEye(leftX,blink,s); drawOneEye(rightX,blink,s); break;

    case HAPPY:
      drawOneEye(leftX,blink,s); drawOneEye(rightX,blink,s);
      safeLine(50,26,78,26);
      eyebrow(leftX-6,eyeY-6,leftX+10,eyeY-8);
      eyebrow(rightX-6,eyeY-8,rightX+10,eyeY-6);
      break;

    case SAD:
      drawOneEye(leftX,blink,s); drawOneEye(rightX,blink,s);
      safeLine(52,27,76,25);
      eyebrow(leftX-6,eyeY-7,leftX+10,eyeY-5);
      eyebrow(rightX-6,eyeY-5,rightX+10,eyeY-7);
      tearDrop(leftX+2,eyeY+18); tearDrop(rightX+eyeW-4,eyeY+18);
      break;

    case ANGRY:
      drawOneEye(leftX,blink,s); drawOneEye(rightX,blink,s);
      eyebrow(leftX-6,eyeY-10,leftX+12,eyeY-6);
      eyebrow(rightX-6,eyeY-6, rightX+12,eyeY-10);
      break;

    case SURPRISED:
      safeDisc(leftX+eyeW/2,  eyeY+eyeH/2, 6);
      safeDisc(rightX+eyeW/2, eyeY+eyeH/2, 6);
      safeCircle(64,26,3);
      break;

    case SLEEPY:
      drawOneEye(leftX,true,0); drawOneEye(rightX,true,0);
      break;

    case WINK:
      drawOneEye(leftX,true,0); drawOneEye(rightX,false,s);
      eyebrow(rightX-6,eyeY-8,rightX+10,eyeY-10);
      break;

    case LOVE:
      drawOneEye(leftX,blink,0); drawOneEye(rightX,blink,0);
      heart8(46,8); heart8(82,8);
      safeLine(50,26,78,26);
      break;

    case LAUGH:
      drawOneEye(leftX,blink,s); drawOneEye(rightX,blink,s);
      safeRBox(50,22,28,6,2);
      break;

    case CRY:
      drawOneEye(leftX,false,0); drawOneEye(rightX,false,0);
      tearDrop(leftX+4, eyeY+18); tearDrop(rightX+eyeW-5, eyeY+18);
      safeLine(52,27,76,25);
      break;

    case SHY:
      drawOneEye(leftX,blink,s); drawOneEye(rightX,blink,s);
      safeBox(24,20,1,1); safeBox(26,21,1,1); safeBox(30,20,1,1);
      safeBox(100,20,1,1); safeBox(102,21,1,1); safeBox(106,20,1,1);
      break;

    case CONFUSED:
      drawOneEye(leftX,blink,-2); drawOneEye(rightX,blink, 2);
      eyebrow(leftX-6,eyeY-7,leftX+12,eyeY-9);
      eyebrow(rightX-6,eyeY-4,rightX+12,eyeY-2);
      safeLine(54,26,60,24); safeLine(60,24,68,27); safeLine(68,27,74,25);
      break;

    case COOL:
      sunglasses(); break;

    case THINKING:
      drawOneEye(leftX,false,-1); drawOneEye(rightX,false,-1);
      safeDisc(104,26,1); safeDisc(110,27,1); safeDisc(116,28,1);
      safeLine(56,26,72,26);
      break;

    case SCARED:
      safeDisc(leftX+eyeW/2,  eyeY+7, 5);
      safeDisc(rightX+eyeW/2, eyeY+7, 5);
      safeCircle(64,26,5);
      sweat(20,6); sweat(108,6);
      break;

    case SMIRK:
      drawOneEye(leftX,false,s); drawOneEye(rightX,false,0);
      safeLine(58,26,72,24);
      break;

    case DEAD:
      safeLine(leftX+4,eyeY+4, leftX+eyeW-4, eyeY+eyeH-4);
      safeLine(leftX+eyeW-4,eyeY+4, leftX+4, eyeY+eyeH-4);
      safeLine(rightX+4,eyeY+4, rightX+eyeW-4, eyeY+eyeH-4);
      safeLine(rightX+eyeW-4,eyeY+4, rightX+4, eyeY+eyeH-4);
      break;

    case WOW:
      starEye(leftX+eyeW/2,  eyeY+eyeH/2);
      starEye(rightX+eyeW/2, eyeY+eyeH/2);
      break;
  }

  u8g2.sendBuffer();
}

// ===== ระยะเวลาค้างต่ออารมณ์ (สุ่ม) =====
unsigned long dwellMin(Mood m){
  switch(m){
    case HAPPY:     return 3000;   // 3–5s
    case SLEEPY:    return 7000;   // 7–10s
    case SURPRISED: return 1000;   // 1–2s
    default:        return 3000;   // อื่น ๆ 3–6s
  }
}
unsigned long dwellMax(Mood m){
  switch(m){
    case HAPPY:     return 5000;
    case SLEEPY:    return 10000;
    case SURPRISED: return 2000;
    default:        return 6000;
  }
}
void scheduleNextMoodHold(){
  nextMoodDelay = random(dwellMin(mood), dwellMax(mood));
  lastMoodChange = millis();
}
Mood randomMood(){ return (Mood)random(0, MOOD_COUNT); }

void setup(){
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);       // สายยาว/ไฟไม่นิ่งใช้ 100kHz (ถ้าสายสั้นนิ่ง ลอง 400kHz ได้)
  u8g2.begin();
  u8g2.setContrast(180);       // ลด ghost/เส้นขอบ (ลองช่วง 160–200 ตามจอ)

  randomSeed(analogRead(0));
  nextBlinkDelay  = random(1200, 3500);
  blinkDuration   = random(80, 160);
  nextNystagDelay = random(140, 280);
  scheduleNextMoodHold();

  drawFace(mood,false,0);
}

void loop(){
  unsigned long now = millis();

  // เปลี่ยนอารมณ์เมื่อครบเวลาค้าง
  if(now - lastMoodChange > nextMoodDelay){
    mood = randomMood();
    scheduleNextMoodHold();
    drawFace(mood,false,pupilOffset);
  }

  // กระพริบตา (เว้นบางอารมณ์)
  bool canBlink = !(mood==SLEEPY || mood==SURPRISED || mood==COOL || mood==DEAD || mood==WOW);
  if(canBlink && now - lastBlink > nextBlinkDelay){
    drawFace(mood,true,pupilOffset);
    delay(blinkDuration);
    drawFace(mood,false,pupilOffset);
    lastBlink = now;
    nextBlinkDelay = random(1200, 3500);
    blinkDuration  = random(80, 160);
  }

  // ขยับรูม่านตาเล็ก ๆ แบบสุ่ม
  if(now - lastNystag > nextNystagDelay &&
     (mood!=SLEEPY && mood!=SURPRISED && mood!=COOL && mood!=DEAD && mood!=WOW)){
    lastNystag = now;
    nextNystagDelay = random(140, 280);
    pupilOffset++; if(pupilOffset>2) pupilOffset=-2;
    drawFace(mood,false,pupilOffset);
  }
}
