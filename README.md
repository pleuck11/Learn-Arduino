# 🤖 Learn Arduino — รวม Lab โปรเจค IoT

> คลังโค้ดรวมโปรเจคย่อยจากคลาสเรียน Arduino & ESP32 ตั้งแต่พื้นฐานเซนเซอร์บน Arduino Uno ไปจนถึงระบบ IoT เต็มรูปแบบบน ESP32 พร้อม Web Dashboard, Firebase และ Blynk

---

## 📁 โครงสร้างโปรเจค

```
Learn-Arduino/
├── Uno_3/                  # Arduino Uno — เซนเซอร์พื้นฐาน
│   ├── Dht11/              # อ่านอุณหภูมิ (Analog)
│   └── Ultrasonic/         # วัดระยะด้วย Ultrasonic
│
└── ESP 32/                 # ESP32 — IoT & Web
    ├── Lab_3/              # ESP32 + DHT11 + LDR + Rain Sensor (Serial)
    ├── Lab_3-P/            # Lab_3 + Web Dashboard + Wi-Fi Manager
    ├── Lab_4/              # Lab_3-P + ส่งข้อมูลขึ้น PHP Server
    ├── Lab_4-P/            # Lab_3-P + Firebase Realtime Database
    ├── Lab_5-P/            # Lab_3-P + Blynk IoT Platform
    ├── Smart_Fire_Alarm_System/  # ระบบแจ้งเตือนไฟไหม้ + Web UI
    ├── WIFI/               # ESP32 Wi-Fi Manager (ตัวอย่างพื้นฐาน)
    ├── DHT-Display/        # DHT22 + OLED 128x64
    ├── OLED/               # ESP32 Robot Face บน OLED 128x32
    ├── Map/                # นำทาง Bluetooth + OLED Display
    ├── Room_status/        # สถานะห้อง Firebase + Web Portal
    ├── Web_app/            # ตัวอย่าง ESP32 WiFi Connect พื้นฐาน
    └── ESP_32_LVGL/        # TFT Touchscreen + LVGL UI
```

---

## 🔬 Arduino Uno — พื้นฐาน (`Uno_3/`)

### 📌 Dht11 — อ่านอุณหภูมิด้วย Analog Sensor
**ไฟล์:** `Uno_3/Dht11/Dht11.ino`

อ่านค่าอุณหภูมิจาก Analog Temperature Sensor (LM35 / TMP36) บนขา **A5** ของ Arduino Uno โดยอ่านค่าเฉลี่ยจาก 20 ครั้งแล้วแปลงเป็นโวลต์ และคำนวณเป็นองศาเซลเซียส แสดงผลผ่าน Serial Monitor

| รายละเอียด | ค่า |
|---|---|
| Board | Arduino Uno |
| เซนเซอร์ | LM35 / TMP36 (Analog) |
| ขาที่ใช้ | A5 |
| Baud Rate | 9600 |

---

### 📌 Ultrasonic — วัดระยะทางด้วย HC-SR04
**ไฟล์:** `Uno_3/Ultrasonic/Ultrasonic.ino`

ใช้ Ultrasonic Sensor (HC-SR04) วัดระยะทางโดยส่งสัญญาณ Trig ที่ขา **A1** และรับ Echo ที่ขา **A2** คำนวณระยะทางจากเวลาที่คลื่นเสียงใช้เดินทาง แสดงหน่วยเป็นเซนติเมตร

| รายละเอียด | ค่า |
|---|---|
| Board | Arduino Uno |
| เซนเซอร์ | HC-SR04 |
| Trig Pin | A1 |
| Echo Pin | A2 |
| Baud Rate | 9600 |

---

## 🌐 ESP32 — IoT Projects (`ESP 32/`)

### 🧪 Lab_3 — DHT11 + LDR + Rain Sensor (Serial Only)
**ไฟล์:** `ESP 32/Lab_3/Lab_3.ino`

Lab แรกบน ESP32 — อ่านเซนเซอร์สภาพอากาศหลายตัวพร้อมกัน และควบคุมรีเลย์อัตโนมัติตามแสง แสดงผลผ่าน Serial Monitor

**ฟีเจอร์:**
- อ่านอุณหภูมิ / ความชื้น / Heat Index จาก DHT11
- ตรวจสอบสภาวะฝน (Rain Sensor)
- ตรวจสภาวะกลางวัน/กลางคืน (LDR)
- ควบคุม Relay อัตโนมัติตามแสง (เปิดไฟกลางคืน ปิดกลางวัน)

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| เซนเซอร์ | DHT11 (D5), LDR (GPIO22), Rain (GPIO23) |
| Relay | GPIO4 |
| Library | DHT sensor library |

---

### 🧪 Lab_3-P — Web Dashboard + Wi-Fi Manager
**ไฟล์:** `ESP 32/Lab_3-P/Lab_3-P.ino`

ต่อยอดจาก Lab_3 โดยเพิ่ม Web Server บน ESP32 ให้ดูค่าเซนเซอร์ผ่านเบราว์เซอร์ได้ พร้อม Wi-Fi Manager ผ่าน Captive Portal

**ฟีเจอร์:**
- Web Dashboard แสดงค่าเซนเซอร์แบบ Real-time (`/home`)
- Wi-Fi Manager — ตั้งค่า SSID/Password ผ่าน Captive Portal (`/manager`)
- บันทึก WiFi credentials ไว้ใน NVS (Preferences)
- REST API `/status.json` และ `/act` สำหรับควบคุม Relay / Auto Mode
- โหมด Auto (LDR) และ Manual (ผ่าน Web UI)

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| AP SSID | `ESP32-Setup` / Password: `12345678` |
| Library | WiFi, WebServer, DNSServer, Preferences |

---

### 🧪 Lab_4 — Web Dashboard + ส่งข้อมูลขึ้น PHP Server
**ไฟล์:** `ESP 32/Lab_4/Lab_4.ino`

ต่อยอดจาก Lab_3-P โดยเพิ่มการส่งข้อมูลเซนเซอร์ขึ้น PHP Backend ทุก 5 วินาที ผ่าน HTTP GET Request

**ฟีเจอร์:**
- ทุกฟีเจอร์จาก Lab_3-P
- อัปโหลดข้อมูล (temp, humid, ldr, rainfall) ขึ้น PHP Endpoint ทุก 5 วินาที
- รีเฟรชสถานะเซนเซอร์ทุก 15 วินาที

**การตั้งค่า:**
```cpp
String PHP_ENDPOINT = "http://192.168.x.x:8088/php/insert.php";
```

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| Library | WiFi, WebServer, DNSServer, Preferences, HTTPClient |
| Upload Interval | 5 วินาที |

---

### 🧪 Lab_4-P — Web Dashboard + Firebase Realtime Database
**ไฟล์:** `ESP 32/Lab_4-P/Lab_4-P.ino`

ต่อยอดจาก Lab_3-P โดยเปลี่ยนจาก PHP เป็นการส่งข้อมูลขึ้น Firebase Realtime Database พร้อมรับคำสั่งควบคุม Relay จาก Firebase ได้

**ฟีเจอร์:**
- ทุกฟีเจอร์จาก Lab_3-P
- Push ข้อมูลเซนเซอร์ขึ้น Firebase RTDB ทุก 10 วินาที
- Poll "desired state" จาก Firebase เพื่อควบคุม Relay ทุก 1 วินาที
- ตั้งค่า Firebase ผ่าน `firebase_secrets.h`

| ไฟล์ | คำอธิบาย |
|---|---|
| `Lab_4-P.ino` | โค้ดหลัก |
| `sensors.h` | ไดรเวอร์เซนเซอร์ |
| `firebase_secrets.h` | API Key / Database URL (ไม่อัปโหลด Git) |
| `index_html.h` | HTML Web Dashboard |
| `manager_html.h` | HTML Wi-Fi Manager |

---

### 🧪 Lab_5-P — Web Dashboard + Blynk IoT
**ไฟล์:** `ESP 32/Lab_5-P/Lab_5-P.ino`

ต่อยอดจาก Lab_3-P โดยเพิ่มการเชื่อมต่อ Blynk IoT Platform เพื่อดูข้อมูลและควบคุม Relay ผ่านแอป Blynk บนมือถือ

**ฟีเจอร์:**
- ทุกฟีเจอร์จาก Lab_3-P
- เชื่อมต่อ Blynk Cloud และส่งข้อมูลเซนเซอร์
- ควบคุม Relay ผ่าน Blynk App ได้

| ไฟล์ | คำอธิบาย |
|---|---|
| `Lab_5-P.ino` | โค้ดหลัก |
| `blynk_glue.h` | Blynk configuration และ Auth Token |
| `sensors.h` | ไดรเวอร์เซนเซอร์ |

---

### 🔥 Smart Fire Alarm System — ระบบแจ้งเตือนไฟไหม้อัจฉริยะ
**ไฟล์:** `ESP 32/Smart_Fire_Alarm_System/Smart_Fire_Alarm_System.ino`

ระบบแจ้งเตือนไฟไหม้แบบครบวงจร ตรวจจับควัน อุณหภูมิ และควบคุมปั๊มน้ำ / Buzzer / RGB LED พร้อม Web Dashboard สำหรับตรวจสอบสถานะและควบคุมระยะไกล

**ฟีเจอร์:**
- ตรวจจับควัน (Smoke Sensor)
- วัดอุณหภูมิด้วย DHT — เปิดปั๊มน้ำอัตโนมัติเมื่ออุณหภูมิ >= 35°C
- ควบคุม Buzzer (Passive) และ RGB LED
- Web Dashboard แสดงสถานะและควบคุม Manual/Auto
- Wi-Fi Manager ผ่าน AP Portal (SSID: `SMART-FIRE-SETUP`)
- โหมด Auto และ Manual แยกจากกัน

**เกณฑ์การทำงาน:**
```cpp
#define PUMP_HOT_C 35.0f   // >= 35°C → ปั๊ม ON
```

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| AP SSID | `SMART-FIRE-SETUP` / Password: `12345678` |
| Library | WiFi, WebServer, Preferences |

---

### 📡 WIFI — ESP32 Wi-Fi Manager ตัวอย่างพื้นฐาน
**ไฟล์:** `ESP 32/WIFI/WIFI.ino`

ตัวอย่างการทำ Wi-Fi Manager บน ESP32 แบบครบถ้วน ประกอบด้วย Captive Portal สำหรับตั้งค่า SSID/Password, Web UI, Scan Networks, Reset และ Reconfigure

**ฟีเจอร์:**
- Captive Portal สำหรับตั้งค่า Wi-Fi (`/manager`)
- สแกนเครือข่ายที่ใกล้เคียง (`/scan`)
- บันทึกและ Reset credentials ผ่าน NVS
- Reconfig Portal (เปลี่ยน Wi-Fi โดยไม่ต้อง Reset)
- Status Page แสดงข้อมูลการเชื่อมต่อ

---

### 📊 DHT-Display — อุณหภูมิ/ความชื้น บน OLED 128x64
**ไฟล์:** `ESP 32/DHT-Display/DHT-Display.ino`

อ่านค่าอุณหภูมิและความชื้นจาก DHT22 แล้วแสดงบนจอ OLED 128x64 พร้อม Progress Bar สองโทนสวยงาม รองรับทั้ง SSD1306 และ SH1106

**ฟีเจอร์:**
- แสดงอุณหภูมิ (°C) และความชื้น (%RH) บน OLED
- Progress Bar แบบ Two-Tone สำหรับ Humidity
- รองรับ SSD1306 และ SH1106 (เปลี่ยนได้ผ่าน `DISPLAY_DRIVER`)
- อัปเดตทุก 2.5 วินาที

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| เซนเซอร์ | DHT22 (GPIO4) |
| จอ | OLED 128x64 I2C (SDA=21, SCL=22) |
| Library | U8g2, DHT sensor library |

---

### 😊 OLED — Robot Face Animation บน OLED 128x32
**ไฟล์:** `ESP 32/OLED/OLED.ino`

แสดงอารมณ์ต่าง ๆ ผ่านดวงตาสไตล์ Robot บนจอ OLED 128x32 มีอารมณ์ให้สุ่มถึง 18 แบบ พร้อม Animation กะพริบตาและการเคลื่อนไหวของม่านตา

**อารมณ์ที่รองรับ (18 แบบ):**  
`NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY, WINK, LOVE, LAUGH, CRY, SHY, CONFUSED, COOL, THINKING, SCARED, SMIRK, DEAD, WOW`

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| จอ | OLED 128x32 I2C (SH1106) |
| Library | U8g2 |

---

### 🗺️ Map — Bluetooth Navigation บน OLED
**ไฟล์:** `ESP 32/Map/Map.ino`

รับข้อมูลนำทางผ่าน Bluetooth จากมือถือและแสดงบนจอ OLED 128x64 จำลองระบบนำทางบนกระจกหน้ารถหรืออุปกรณ์พกพา

**ฟีเจอร์:**
- รับคำสั่งนำทางผ่าน Classic Bluetooth (`ESP32-Map`)
- แสดงข้อมูลนำทาง (เลี้ยวซ้าย/ขวา, ตรงไป, ถึงจุดหมาย)
- รองรับ OLED 128x64 (SH1106) สแกน address อัตโนมัติ

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| Bluetooth | Classic BT (SerialBT) |
| BT Name | `ESP32-Map` |
| จอ | OLED 128x64 I2C (SDA=21, SCL=22) |
| Library | U8g2, BluetoothSerial |

---

### 🏠 Room Status — ระบบตรวจสอบสถานะห้อง + Firebase
**ไฟล์:** `ESP 32/Room_status/Room_status.ino`

ระบบตรวจสอบสภาพแวดล้อมในห้อง ส่งข้อมูลขึ้น Firebase Realtime Database ทุก 5 วินาที และรับคำสั่งควบคุม Relay จาก Firebase

**ฟีเจอร์:**
- Wi-Fi Portal สำหรับตั้งค่า (AP SSID: `RoomStatus-Setup`)
- ส่งข้อมูลเซนเซอร์ขึ้น Firebase ทุก 5 วินาที
- รับคำสั่ง `relayOn` จาก Firebase `/settings/<DEVICE_ID>/relayOn`
- Claim ความเป็นเจ้าของอุปกรณ์ใน Firebase (Owner claim)
- รองรับ mDNS
- Relay GPIO26 (Active-LOW)

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| AP SSID | `RoomStatus-Setup` |
| Library | Firebase_ESP_Client, WiFi, WebServer, Preferences, ESPmDNS |

---

### 🌐 Web App — ตัวอย่าง ESP32 WiFi Connection
**ไฟล์:** `ESP 32/Web_app/Web_app.ino`

โค้ดตัวอย่างพื้นฐานสำหรับการเชื่อมต่อ WiFi และอ่านค่าเซนเซอร์บน ESP32 เหมาะสำหรับเป็น Template เริ่มต้น

**การตั้งค่า:**
```cpp
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

---

### 🖥️ ESP32 LVGL — TFT Touchscreen UI
**ไฟล์:** `ESP 32/ESP_32_LVGL/ESP_32_LVGL.ino`

UI บน TFT Touchscreen ด้วย TFT_eSPI + XPT2046 Touch พร้อม RGB LED, LDR และ Speaker ผ่าน LEDC

**ฟีเจอร์:**
- TFT Display (Landscape) พร้อม Touch (XPT2046)
- RGB LED (R=GPIO4, G=GPIO16, B=GPIO17)
- LDR Sensor (GPIO34)
- Speaker/Buzzer ผ่าน LEDC PWM (GPIO26)
- UI แยกอยู่ใน `Ui.h` และ `screens/`

| รายละเอียด | ค่า |
|---|---|
| Board | ESP32 |
| Display | TFT SPI (TFT_eSPI) |
| Touch | XPT2046 (SPI) |
| Library | TFT_eSPI, XPT2046_Touchscreen |

---

## 🛠️ Library ที่ใช้

| Library | ใช้ใน |
|---|---|
| DHT sensor library (Adafruit) | Dht11, Lab_3, Lab_3-P, Lab_4, Lab_4-P, Lab_5-P |
| U8g2 | DHT-Display, OLED, Map |
| Firebase_ESP_Client (Mobizt) | Lab_4-P, Room_status |
| BluetoothSerial (built-in) | Map |
| TFT_eSPI | ESP_32_LVGL |
| XPT2046_Touchscreen | ESP_32_LVGL |
| WiFi / WebServer / DNSServer (built-in) | Lab_3-P, Lab_4, Lab_4-P, Lab_5-P, WIFI, Smart_Fire_Alarm, Room_status |

---

## 🚀 วิธีใช้งาน

1. **เปิดโปรเจคด้วย Arduino IDE** — เปิดไฟล์ `.ino` ใน folder ที่ต้องการ
2. **ติดตั้ง Library** ผ่าน Arduino Library Manager ตามตารางด้านบน
3. **เลือก Board** → `ESP32 Dev Module` (สำหรับโปรเจค ESP32) หรือ `Arduino Uno`
4. **อัปโหลดโค้ด** และเปิด Serial Monitor ที่ Baud Rate ที่กำหนด
5. **สำหรับโปรเจคที่มี Wi-Fi Manager** — เชื่อมต่อ Wi-Fi ชื่อ `ESP32-Setup` และเปิดเบราว์เซอร์ไปที่ `192.168.4.1`

---

## ⚠️ หมายเหตุ

- ไฟล์ `firebase_secrets.h` ไม่ถูกอัปโหลดขึ้น Git — ต้องสร้างเองโดยใส่ Firebase API Key และ Database URL
- สำหรับ Lab_5-P ต้องมี Blynk Auth Token ใน `blynk_glue.h`
- โปรเจค ESP32 ทั้งหมดรองรับ ESP32 Arduino Core เวอร์ชัน 2.x และ 3.x

---

*โปรเจคนี้เป็นส่วนหนึ่งของวิชาเรียน Arduino & IoT*
