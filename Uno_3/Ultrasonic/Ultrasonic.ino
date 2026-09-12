// กำหนดขาที่ใช้กับ Ultrasonic
const int trigPin = A1;  // ใช้ A1 เป็น Trig
const int echoPin = A2;  // ใช้ A2 เป็น Echo


// ตัวแปรเก็บค่าระยะทาง
long duration;
int distance;

void setup() {
  // เริ่ม Serial Monitor
  Serial.begin(9600);

  // กำหนดโหมดขา
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // ส่งสัญญาณ Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // อ่านค่าระยะเวลา Echo
  duration = pulseIn(echoPin, HIGH);

  // คำนวณระยะทาง (cm)
  distance = duration * 0.034 / 2;

  // แสดงผล
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500); // หน่วงเวลา 0.5 วินาที
}
