#pragma once

// ====== Firebase Project Config (เติมของจริง) ======
#define FIREBASE_API_KEY"AIzaSyBWeTT-iggIr_OqKGT65A8dafpOEYu-7yA"
#define FIREBASE_DATABASE_URL"https://test-404f0-default-rtdb.asia-southeast1.firebasedatabase.app"

// เลือกแบบใดแบบหนึ่ง:
// 1) ใช้ Anonymous sign-in -> เปิดใน Console (Authentication > Sign-in method > Anonymous: Enable)
//    แล้วปล่อยสองบรรทัดนี้ว่าง
#define USER_EMAIL              ""
#define USER_PASSWORD           ""

// 2) หรือใช้ Email/Password (เปิด Email/Password แล้วใส่ค่าจริง)
// #define USER_EMAIL           "myuser@example.com"
// #define USER_PASSWORD        "mypassword"

// ตั้งรหัสอุปกรณ์ (โหนดใน RTDB)
#define DEVICE_ID               "esp32-lab4"
