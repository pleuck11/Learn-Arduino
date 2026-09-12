// firebase.h
#pragma once

// ====== Firebase Project Config (ใส่ค่าจริงของโปรเจคคุณ) ======
#define FIREBASE_API_KEY       "AIzaSyCTJZ3rpJrXL5iZjZ9DIvKz8gPBrA88hqk"
#define FIREBASE_DATABASE_URL  "https://room-status-86153-default-rtdb.asia-southeast1.firebasedatabase.app"

// เลือกวิธี auth อย่างใดอย่างหนึ่ง
// 1) Anonymous sign-in (เปิดใน Console > Authentication > Sign-in method > Anonymous: Enable)
//    ปล่อยสองบรรทัดนี้ว่าง
// #define USER_EMAIL             ""
// #define USER_PASSWORD          ""

// 2) หรือใช้ Email/Password (ให้เปิด Email/Password และใส่ค่าจริง)
#define USER_EMAIL          "roomstatus@dev.com"
#define USER_PASSWORD       "PlEuCk0803"

// ตั้งรหัสอุปกรณ์ (ใช้เป็น path/โหนดใน RTDB)
#define DEVICE_ID              "esp32-01"
