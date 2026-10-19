#include "board.h"
#include "display_frames.h"
#include "update_scanout.h"
#include "gfx565.h"
#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <SD_MMC.h>
#include "driver/spi_master.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace board {
// ---- pins (from the Waveshare wiki / demo) ----
static const int PIN_I2C_SDA = 15, PIN_I2C_SCL = 7;
static const int PIN_LCD_SPI_CLK = 2, PIN_LCD_SPI_MOSI = 1;
static const int PIN_BL = 6;
static const int PIN_HSYNC = 38, PIN_VSYNC = 39, PIN_DE = 40, PIN_PCLK = 41;
static const int PIN_DATA[16] = {5, 45, 48, 47, 21, 14, 13, 12, 11, 10, 9, 46, 3, 8, 18, 17};
static const int PIN_TP_INT = 16;
// TCA9554 expander (P0..P7 == EXIO1..EXIO8)
static const uint8_t EXP_ADDR = 0x20, EXP_OUT = 0x01, EXP_CFG = 0x03;
static const uint8_t EX_LCD_RST = 0, EX_TP_RST = 1, EX_LCD_CS = 2, EX_SD_CS = 3, EX_BUZZER = 7;
static const uint8_t TP_ADDR = 0x15, RTC_ADDR = 0x51;

static uint8_t g_expOut = 0xFF;
static esp_lcd_panel_handle_t g_panel = nullptr;
static uint16_t* g_fbs[2] = {nullptr, nullptr};
static DisplayFrames g_frames;
static portMUX_TYPE g_frameMux = portMUX_INITIALIZER_UNLOCKED;
static SemaphoreHandle_t g_vsync = nullptr;
static SemaphoreHandle_t g_frameCopied = nullptr;

static void i2cWrite(uint8_t addr, uint8_t reg, const uint8_t* d, size_t n) {
  Wire.beginTransmission(addr); Wire.write(reg); for (size_t i = 0; i < n; i++) Wire.write(d[i]); Wire.endTransmission(true);
}
static bool i2cRead(uint8_t addr, uint8_t reg, uint8_t* d, size_t n) {
  Wire.beginTransmission(addr); Wire.write(reg);
  if (Wire.endTransmission(true) != 0) return false;
  size_t got = Wire.requestFrom((int)addr, (int)n);
  for (size_t i = 0; i < n; i++) d[i] = Wire.available() ? (uint8_t)Wire.read() : 0;
  return got == n;
}
static void expSet(uint8_t pin, bool level) {
  if (level) g_expOut |= (uint8_t)(1 << pin); else g_expOut &= (uint8_t)~(1 << pin);
  i2cWrite(EXP_ADDR, EXP_OUT, &g_expOut, 1);
}

// ---- ST7701 init over 9-bit SPI (D/C bit + 8 data), CS on the expander ----
static spi_device_handle_t g_spi = nullptr;
static void lcdCmd(uint8_t c) { spi_transaction_t t = {}; t.cmd = 0; t.addr = c; spi_device_transmit(g_spi, &t); }
static void lcdDat(uint8_t d) { spi_transaction_t t = {}; t.cmd = 1; t.addr = d; spi_device_transmit(g_spi, &t); }
// Init table: {cmd, nargs, args...}; 0xFF/0x00 marker rows for delays are encoded as cmd=0xFE.
static const uint8_t ST7701_INIT[] = {
  0xFF,5,0x77,0x01,0x00,0x00,0x10, 0xC0,2,0x3B,0x00, 0xC1,2,0x0B,0x02, 0xC2,2,0x07,0x02, 0xCC,1,0x10, 0xCD,1,0x08,
  0xB0,16,0x00,0x11,0x16,0x0e,0x11,0x06,0x05,0x09,0x08,0x21,0x06,0x13,0x10,0x29,0x31,0x18,
  0xB1,16,0x00,0x11,0x16,0x0e,0x11,0x07,0x05,0x09,0x09,0x21,0x05,0x13,0x11,0x2a,0x31,0x18,
  0xFF,5,0x77,0x01,0x00,0x00,0x11, 0xB0,1,0x6d, 0xB1,1,0x37, 0xB2,1,0x81, 0xB3,1,0x80, 0xB5,1,0x43, 0xB7,1,0x85, 0xB8,1,0x20,
  0xC1,1,0x78, 0xC2,1,0x78, 0xD0,1,0x88, 0xE0,3,0x00,0x00,0x02,
  0xE1,11,0x03,0xA0,0x00,0x00,0x04,0xA0,0x00,0x00,0x00,0x20,0x20,
  0xE2,13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xE3,4,0x00,0x00,0x11,0x00, 0xE4,2,0x22,0x00,
  0xE5,16,0x05,0xEC,0xA0,0xA0,0x07,0xEE,0xA0,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xE6,4,0x00,0x00,0x11,0x00, 0xE7,2,0x22,0x00,
  0xE8,16,0x06,0xED,0xA0,0xA0,0x08,0xEF,0xA0,0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xEB,7,0x00,0x00,0x40,0x40,0x00,0x00,0x00,
  0xED,16,0xFF,0xFF,0xFF,0xBA,0x0A,0xBF,0x45,0xFF,0xFF,0x54,0xFB,0xA0,0xAB,0xFF,0xFF,0xFF,
  0xEF,6,0x10,0x0D,0x04,0x08,0x3F,0x1F,
  0xFF,5,0x77,0x01,0x00,0x00,0x13, 0xEF,1,0x08,
  0xFF,5,0x77,0x01,0x00,0x00,0x00, 0x36,1,0x00, 0x3A,1,0x66,
};
static void lcdInitPanel() {
  spi_bus_config_t bus = {}; bus.mosi_io_num = PIN_LCD_SPI_MOSI; bus.miso_io_num = -1; bus.sclk_io_num = PIN_LCD_SPI_CLK;
  bus.quadwp_io_num = -1; bus.quadhd_io_num = -1; bus.max_transfer_sz = 64;
  spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
  spi_device_interface_config_t dev = {}; dev.command_bits = 1; dev.address_bits = 8; dev.mode = 0;
  dev.clock_speed_hz = 40000000; dev.spics_io_num = -1; dev.queue_size = 1;
  spi_bus_add_device(SPI2_HOST, &dev, &g_spi);

  expSet(EX_LCD_RST, false); delay(10); expSet(EX_LCD_RST, true); delay(50);
  expSet(EX_LCD_CS, false); delay(10);
  for (size_t i = 0; i < sizeof ST7701_INIT;) {
    uint8_t cmd = ST7701_INIT[i++], n = ST7701_INIT[i++];
    lcdCmd(cmd); for (uint8_t k = 0; k < n; k++) lcdDat(ST7701_INIT[i++]);
  }
  lcdCmd(0x11); delay(120); lcdCmd(0x20); delay(20); lcdCmd(0x29); delay(20);
  expSet(EX_LCD_CS, true); delay(10);
  spi_bus_remove_device(g_spi); spi_bus_free(SPI2_HOST); g_spi = nullptr;

  esp_lcd_rgb_panel_config_t cfg = {};
  cfg.clk_src = LCD_CLK_SRC_DEFAULT;
  cfg.timings.pclk_hz = 16 * 1000 * 1000;
  cfg.timings.h_res = LCD_W; cfg.timings.v_res = LCD_H;
  cfg.timings.hsync_pulse_width = 8; cfg.timings.hsync_back_porch = 10; cfg.timings.hsync_front_porch = 50;
  cfg.timings.vsync_pulse_width = 3; cfg.timings.vsync_back_porch = 8; cfg.timings.vsync_front_porch = 8;
  cfg.timings.flags.pclk_active_neg = 0;
  cfg.data_width = 16; cfg.bits_per_pixel = 16; cfg.num_fbs = 2;
  // Scan from internal RAM while the CPU copies ahead from PSRAM. The local driver backports the missed-EOF
  // recovery fix and restarts only on underrun/request, rather than disturbing every healthy frame.
  cfg.bounce_buffer_size_px = 10 * LCD_W;
  cfg.dma_burst_size = 64;
  cfg.hsync_gpio_num = PIN_HSYNC; cfg.vsync_gpio_num = PIN_VSYNC; cfg.de_gpio_num = PIN_DE; cfg.pclk_gpio_num = PIN_PCLK; cfg.disp_gpio_num = -1;
  for (int i = 0; i < 16; i++) cfg.data_gpio_nums[i] = PIN_DATA[i];
  cfg.flags.fb_in_psram = 1; cfg.flags.double_fb = 1;
  ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&cfg, &g_panel));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(g_panel));
  ESP_ERROR_CHECK(esp_lcd_panel_init(g_panel));
  void *a = nullptr, *b = nullptr;
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(g_panel, 2, &a, &b));
  g_fbs[0] = (uint16_t*)a; g_fbs[1] = (uint16_t*)b;
  memset(g_fbs[0], 0, LCD_W * LCD_H * 2); memset(g_fbs[1], 0, LCD_W * LCD_H * 2);
  g_vsync = xSemaphoreCreateBinary();
  g_frameCopied = xSemaphoreCreateBinary();
  ESP_ERROR_CHECK(g_vsync && g_frameCopied ? ESP_OK : ESP_ERR_NO_MEM);
  esp_lcd_rgb_panel_event_callbacks_t cbs = {};
  cbs.on_vsync = [](esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void*) -> bool {
    BaseType_t hp = pdFALSE; xSemaphoreGiveFromISR(g_vsync, &hp); return hp == pdTRUE;
  };
  cbs.on_frame_buf_complete = [](esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void*) -> bool {
    portENTER_CRITICAL_ISR(&g_frameMux);
    bool done = g_frames.frameCopied();
    portEXIT_CRITICAL_ISR(&g_frameMux);
    if (!done) return false;
    BaseType_t hp = pdFALSE; xSemaphoreGiveFromISR(g_frameCopied, &hp); return hp == pdTRUE;
  };
  esp_lcd_rgb_panel_register_event_callbacks(g_panel, &cbs, nullptr);
  esp_lcd_panel_draw_bitmap(g_panel, 0, 0, LCD_W, LCD_H, g_fbs[0]);
}

// What each buffer holds of an indexed frame, so expand() rewrites only the rows that changed. Anything else draws into
// a buffer through backBuffer() (an RGB565 app's render), which forgets what that buffer held.
static gfx565::Expanded g_expanded[2] = {};
void expand(const uint8_t* fb160, const uint16_t* pal565) {
  gfx565::target(g_fbs[g_frames.back()]);
  gfx565::expandIndexed3(fb160, pal565, &g_expanded[g_frames.back()]);
}
uint16_t* backBuffer() { g_expanded[g_frames.back()].palette = nullptr; return g_fbs[g_frames.back()]; }
bool beginUpdateDisplay() { return update_scanout::begin(); }
void drawUpdateProgress(const char* label, bool cancellable) { update_scanout::draw(label, cancellable); }
bool hitUpdateCancel(int x, int y) { return update_scanout::hitCancel(x, y); }
void endUpdateDisplay() {
  update_scanout::stop();
  ESP_ERROR_CHECK(esp_lcd_panel_init(g_panel));   // restore normal timings, vsync ISR and original framebuffer DMA
  update_scanout::release();   // normal DMA no longer references the sparse frame
  xSemaphoreTake(g_vsync, 0);
  g_expanded[0].palette = g_expanded[1].palette = nullptr;   // repaint both whole: nothing assumed across an update
}
const uint16_t* frontBuffer() { return g_fbs[g_frames.front()]; }
void presentHires() {
  // Submission and arming are atomic with the same-core DMA ISR. An earlier frame completion cannot acknowledge it.
  portENTER_CRITICAL(&g_frameMux);
  xSemaphoreTake(g_frameCopied, 0);
  ESP_ERROR_CHECK(g_frames.submit() ? ESP_OK : ESP_ERR_INVALID_STATE);
  ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(g_panel, 0, 0, LCD_W, LCD_H, g_fbs[g_frames.back()]));
  portEXIT_CRITICAL(&g_frameMux);
  // A missing ISR must never hand out a buffer that the driver may still read. Fail closed instead of tearing it.
  ESP_ERROR_CHECK(xSemaphoreTake(g_frameCopied, pdMS_TO_TICKS(250)) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT);
  portENTER_CRITICAL(&g_frameMux);
  ESP_ERROR_CHECK(g_frames.finish() ? ESP_OK : ESP_ERR_INVALID_STATE);
  portEXIT_CRITICAL(&g_frameMux);
}

// ---- touch (CST820) ----
Touch readTouch() {
  uint8_t b[6];
  Touch t = {false, 0, 0};
  if (!i2cRead(TP_ADDR, 0x01, b, 6)) return t;
  uint8_t points = b[1] & 0x0F;
  if (points) {
    int x = ((b[2] & 0x0F) << 8) | b[3], y = ((b[4] & 0x0F) << 8) | b[5];
    if (x < LCD_W && y < LCD_H) { t.down = true; t.x = x; t.y = y; }
  }
  return t;
}

// ---- RTC (PCF85063) ----
static uint8_t bcd2(uint8_t v) { return (uint8_t)((v >> 4) * 10 + (v & 15)); }
static uint8_t dec2(uint8_t v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
static uint32_t daysFromCivil(int y, int m, int d) {  // Howard Hinnant's algorithm
  y -= m <= 2; int era = (y >= 0 ? y : y - 399) / 400; unsigned yoe = (unsigned)(y - era * 400);
  unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return (uint32_t)(era * 146097 + (int)doe - 719468);
}
static void civilFromDays(uint32_t z0, int& y, int& m, int& d) {
  int z = (int)z0 + 719468; int era = (z >= 0 ? z : z - 146096) / 146097; unsigned doe = (unsigned)(z - era * 146097);
  unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; y = (int)yoe + era * 400;
  unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100); unsigned mp = (5 * doy + 2) / 153;
  d = (int)(doy - (153 * mp + 2) / 5 + 1); m = (int)(mp < 10 ? mp + 3 : mp - 9); y += (m <= 2);
}
bool rtcValid() {
  uint8_t s; if (!i2cRead(RTC_ADDR, 0x04, &s, 1)) return false;
  if (s & 0x80) return false;  // oscillator-stop flag: time is garbage
  uint8_t yr; if (!i2cRead(RTC_ADDR, 0x0A, &yr, 1)) return false;
  return bcd2(yr) >= 25;  // year 2025+ (register counts from 2000)
}
uint32_t rtcNow() {
  uint8_t b[7]; if (!i2cRead(RTC_ADDR, 0x04, b, 7)) return 0;
  int sec = bcd2(b[0] & 0x7F), min = bcd2(b[1] & 0x7F), hr = bcd2(b[2] & 0x3F);
  int day = bcd2(b[3] & 0x3F), mon = bcd2(b[5] & 0x1F), yr = 2000 + bcd2(b[6]);
  return daysFromCivil(yr, mon, day) * 86400u + (uint32_t)(hr * 3600 + min * 60 + sec);
}
void rtcSet(uint32_t e) {
  int y, m, d; civilFromDays(e / 86400u, y, m, d);
  uint32_t s = e % 86400u;
  uint8_t ctrl = 0x01;  // 12.5pF, 24h, run
  i2cWrite(RTC_ADDR, 0x00, &ctrl, 1);
  uint8_t b[7] = {dec2((uint8_t)(s % 60)), dec2((uint8_t)((s / 60) % 60)), dec2((uint8_t)(s / 3600)),
                  dec2((uint8_t)d), (uint8_t)((e / 86400u + 4) % 7), dec2((uint8_t)m), dec2((uint8_t)(y - 2000))};
  i2cWrite(RTC_ADDR, 0x04, b, 7);
}

// ---- motion (QMI8658, accelerometer only) ----
// Registers: QST's QMI8658 datasheet as transcribed by SensorLib (github.com/lewisxhe/SensorLib,
// src/sensor/imu/qmi8658/SensorQMI8658_Reg.hpp); addresses 0x6B/0x6A (SA0) from Waveshare's demo driver for this board
// (Gyro_QMI8658.h). WHO_AM_I 0x00 = 0x05; soft reset: 0xB0 to 0x60, done when 0x4D bit 7 is set.
static const uint8_t IMU_WHO_AM_I = 0x00, IMU_CTRL1 = 0x02, IMU_CTRL2 = 0x03, IMU_CTRL5 = 0x06, IMU_CTRL7 = 0x08;
static const uint8_t IMU_AX_L = 0x35, IMU_RESET = 0x60, IMU_RST_RESULT = 0x4D;
static const int IMU_LSB_PER_G = 8192;   // +-4 g full scale
// Chip axis -> screen axis: screen x, y, z = sign * chip[axis]. Flip a sign or swap two axes here if a physical tilt test
// disagrees. The screen frame (x right, y down, z toward the viewer) is left-handed and the chip's is right-handed, so a
// valid table has an odd number of -1 signs without an x/y swap, or an even number with one.
// Measured on the board (2026-09-27, serial "A"): held upright with the home button on top, the chip read +1 g on its x;
// turned a quarter clockwise (right edge down), -1 g on its y; lying face up, -1 g on its z. So chip +x points to the
// screen's top, chip +y to its right, chip +z into the glass.
static const struct { uint8_t axis; int8_t sign; } SCREEN_FROM_CHIP[3] = {{1, +1}, {0, -1}, {2, -1}};
static uint8_t g_imu = 0;   // the chip's I2C address, 0 = not found

static void imuInit() {
  for (uint8_t a : {(uint8_t)0x6B, (uint8_t)0x6A}) {
    uint8_t id = 0;
    if (i2cRead(a, IMU_WHO_AM_I, &id, 1) && id == 0x05) { g_imu = a; break; }
  }
  if (!g_imu) { Serial.println("[board] no QMI8658 motion sensor"); return; }
  Serial.printf("[board] QMI8658 at 0x%02X\n", g_imu);
  uint8_t v = 0xB0; i2cWrite(g_imu, IMU_RESET, &v, 1);
  for (int i = 0; i < 50 && !(i2cRead(g_imu, IMU_RST_RESULT, &v, 1) && (v & 0x80)); i++) delay(2);
  v = 0x40; i2cWrite(g_imu, IMU_CTRL1, &v, 1);   // register auto-increment, little endian, oscillator on
  v = 0x15; i2cWrite(g_imu, IMU_CTRL2, &v, 1);   // +-4 g, 224 Hz
  v = 0x07; i2cWrite(g_imu, IMU_CTRL5, &v, 1);   // accel low-pass on, mode 3 (13.37% of ODR, ~30 Hz)
  v = 0x01; i2cWrite(g_imu, IMU_CTRL7, &v, 1);   // accelerometer only
}

bool readAccel(int16_t& gx, int16_t& gy, int16_t& gz, int16_t* raw) {
  uint8_t b[6];
  if (!g_imu || !i2cRead(g_imu, IMU_AX_L, b, 6)) return false;
  int16_t chip[3];
  for (int i = 0; i < 3; i++) chip[i] = (int16_t)(b[2 * i] | b[2 * i + 1] << 8);
  if (!chip[0] && !chip[1] && !chip[2]) return false;   // no sample yet (a real 0 g on all three is free fall)
  if (raw) for (int i = 0; i < 3; i++) raw[i] = chip[i];
  int16_t* out[3] = {&gx, &gy, &gz};
  // The chip reports specific force (+1 g up at rest); gravity points the other way.
  for (int i = 0; i < 3; i++) *out[i] = (int16_t)(-SCREEN_FROM_CHIP[i].sign * chip[SCREEN_FROM_CHIP[i].axis] * 1000 / IMU_LSB_PER_G);
  return true;
}

void setBacklight(uint8_t pct) { if (pct > 100) pct = 100; ledcWrite(PIN_BL, pct == 100 ? 1023 : pct * 10); }
bool settlePanel() {
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_restart(g_panel));
  xSemaphoreTake(g_vsync, 0);   // discard a notification left over from the download
  // The first ISR restarts DMA; by the second vsync an entire clean frame has been scanned out.
  for (int i = 0; i < 2; i++) if (xSemaphoreTake(g_vsync, pdMS_TO_TICKS(100)) != pdTRUE) return false;
  return true;
}
void buzzer(bool on) { static int last = -1; if ((int)on == last) return; last = on; expSet(EX_BUZZER, on); }

// Each call opens its namespace read-write (creating it, as the old boot-time begin() did) and the
// Preferences destructor closes it again; saves come at most every few seconds, so the open is cheap.
bool saveBlob(const char* ns, const char* key, const void* d, size_t n) { Preferences p; return p.begin(ns, false) && p.putBytes(key, d, n) == n; }
size_t loadBlob(const char* ns, const char* key, void* d, size_t n) {
  Preferences p; if (!p.begin(ns, false)) return 0;
  size_t have = p.getBytesLength(key); return have && have <= n ? p.getBytes(key, d, have) : 0;
}
void eraseBlob(const char* ns, const char* key) { Preferences p; if (p.begin(ns, false)) p.remove(key); }
void eraseNamespace(const char* ns) { Preferences p; if (p.begin(ns, false)) p.clear(); }
uint32_t freeHeap() { return (uint32_t)ESP.getFreeHeap(); }

// ---- microSD (1-bit SDMMC; pins from the Waveshare wiki's table, not their demo code) ----
// CLK and CMD are GPIO2/GPIO1, the ST7701's init SPI: lcdInitPanel() frees that bus for good, and with the panel's CS
// (expander P2) held high it ignores the card's traffic. D3 is expander P3, held high since init(), so the card starts
// in SD mode rather than SPI.
static const int PIN_SD_CLK = PIN_LCD_SPI_CLK, PIN_SD_CMD = PIN_LCD_SPI_MOSI, PIN_SD_D0 = 42;
bool mountSd() { return SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0) && SD_MMC.begin("/sdcard", true); }

void init() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
  uint8_t cfg = 0x00; i2cWrite(EXP_ADDR, EXP_CFG, &cfg, 1);   // all expander pins output
  g_expOut = (uint8_t)((1 << EX_LCD_RST) | (1 << EX_TP_RST) | (1 << EX_LCD_CS) | (1 << EX_SD_CS));  // buzzer off
  i2cWrite(EXP_ADDR, EXP_OUT, &g_expOut, 1);
  pinMode(PIN_TP_INT, INPUT_PULLUP);
  ledcAttach(PIN_BL, 20000, 10); setBacklight(0);
  lcdInitPanel();
  // touch reset + disable auto-sleep
  expSet(EX_TP_RST, false); delay(10); expSet(EX_TP_RST, true); delay(60);
  uint8_t noSleep = 0xFF; i2cWrite(TP_ADDR, 0xFE, &noSleep, 1);
  imuInit();
  setBacklight(100);
}
}  // namespace board
