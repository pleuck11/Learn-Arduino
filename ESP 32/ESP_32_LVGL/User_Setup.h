// ====================== User_Setup.h (local) ======================
// ทำให้ TFT_eSPI ใช้การตั้งค่านี้ แทนการแก้ไฟล์ในไลบรารี
#define USER_SETUP_LOADED 1
#define USER_SETUP_ID 2432

// จอของบอร์ดนี้
#define ILI9341_DRIVER

// ใช้บัส HSPI สำหรับจอ (ไม่ชนกับ microSD ที่ VSPI)
#define TFT_SPI_PORT HSPI

// พินจอ ILI9341 ตามรูป RandomNerd
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1     // ไม่มีขา RST แยก

// Backlight
#define TFT_BL           21
#define TFT_BACKLIGHT_ON HIGH

// ความถี่ SPI
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  16000000

// =================================================================
