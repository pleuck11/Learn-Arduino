// Arduino Uno: Analog temp sensor on A5
// Wiring: VCC→5V, GND→GND, OUT→A5

const int SENSOR_PIN = A5;   // ใช้ A5 ตามที่ขอ
const int SAMPLES    = 20;   // อ่านหลายครั้งเพื่อเฉลี่ยให้ค่านิ่ง
// #define USE_INTERNAL_1V1   // เปิดถ้าอยากละเอียดขึ้น (วัดได้ถึง ~110°C สำหรับ LM35/TMP36)

float readADCavg() {
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) { sum += analogRead(SENSOR_PIN); delay(5); }
  return (float)sum / SAMPLES;
}

void setup() {
  Serial.begin(9600);
#ifdef USE_INTERNAL_1V1
  analogReference(INTERNAL); // ระวัง: Vout ต้องไม่เกิน ~1.1V
#endif
}

void loop() {
  float adc =
#ifdef USE_INTERNAL_1V1
    readADCavg(), Vref = 1.1;
#else
    readADCavg(), Vref = 5.0;
#endif

  float voltage = adc * (Vref / 1023.0);

  // ===== เลือกสูตรให้ตรงกับเซนเซอร์ =====
  float temperatureC = voltage * 100.0;         // LM35 → 10mV/°C
  // float temperatureC = (voltage - 0.5) * 100; // TMP36 → (Vout-0.5V)*100

  Serial.print("ADC: "); Serial.print(adc,1);
  Serial.print("  V: ");  Serial.print(voltage,3); Serial.print(" V  ");
  Serial.print("Temp: ");  Serial.print(temperatureC,2); Serial.println(" C");

  delay(500);
}
