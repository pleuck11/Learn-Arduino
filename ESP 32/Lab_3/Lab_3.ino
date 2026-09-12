#include <DHT.h>

#define DHTPIN   5        // DHT11 ต่อที่ D5 (GPIO5)
#define DHTTYPE  DHT11

#define LDR_PIN  22       // LDR Digital Out (DO)
#define RAIN_PIN 23       // Rain Sensor Digital Out (S)
#define RELAY_PIN 4       // ใช้ D4 (GPIO4) เปิดไฟ/LED

DHT dht(DHTPIN, DHTTYPE);

unsigned long prevMs = 0;
const unsigned long INTERVAL_MS = 1000;

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LDR_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.println("Lab3: ESP32 + DHT11 + LDR(DO) + Rain(DO)");
}

void loop() {
  unsigned long now = millis();
  if (now - prevMs >= INTERVAL_MS) {
    prevMs = now;

    // อ่านค่า DHT11
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float feel = dht.computeHeatIndex(t, h, false);

    if (isnan(h) || isnan(t)) {
      Serial.println("อ่านค่า DHT ไม่สำเร็จ");
      return;
    }

    Serial.println("----- UPDATE -----");
    Serial.print("Temp: "); Serial.print(t); Serial.print(" °C | ");
    Serial.print("Humidity: "); Serial.print(h); Serial.print(" % | ");
    Serial.print("Feel like: "); Serial.print(feel); Serial.println(" °C");

    // ตรวจฝน
    if (digitalRead(RAIN_PIN) == HIGH) {
      Serial.println("ฝนกำลังตก");
    } else {
      Serial.println("ไม่มีฝน");
    }

    // ตรวจกลางวัน/กลางคืน
    if (digitalRead(LDR_PIN) == LOW) {
      Serial.println("กลางคืน → เปิดไฟ D4");
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      Serial.println("กลางวัน → ปิดไฟ D4");
      digitalWrite(RELAY_PIN, LOW);
    }

    Serial.println("---------------------");
  }
}
