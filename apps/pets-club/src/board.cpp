#include "board.h"
#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
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
static int g_front = 0;
static SemaphoreHandle_t g_vsync = nullptr;
static Preferences g_prefs;

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
  cfg.bounce_buffer_size_px = 10 * LCD_W; cfg.psram_trans_align = 64;
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
  esp_lcd_rgb_panel_event_callbacks_t cbs = {};
  cbs.on_vsync = [](esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void*) -> bool {
    BaseType_t hp = pdFALSE; xSemaphoreGiveFromISR(g_vsync, &hp); return hp == pdTRUE;
  };
  esp_lcd_rgb_panel_register_event_callbacks(g_panel, &cbs, nullptr);
  esp_lcd_panel_draw_bitmap(g_panel, 0, 0, LCD_W, LCD_H, g_fbs[0]);
}

void present(const uint8_t* fb160, const uint16_t* pal565) {
  uint16_t* dst = g_fbs[1 - g_front];
  static uint16_t line[LCD_W];
  for (int y = 0; y < 160; y++) {
    const uint8_t* src = fb160 + y * 160;
    uint16_t* l = line;
    for (int x = 0; x < 160; x++) { uint16_t c = pal565[src[x]]; *l++ = c; *l++ = c; *l++ = c; }
    uint16_t* row = dst + (size_t)y * 3 * LCD_W;
    memcpy(row, line, sizeof line); memcpy(row + LCD_W, line, sizeof line); memcpy(row + 2 * LCD_W, line, sizeof line);
  }
  esp_lcd_panel_draw_bitmap(g_panel, 0, 0, LCD_W, LCD_H, dst);  // pointer is a panel fb: swaps at next vsync
  g_front = 1 - g_front;
  xSemaphoreTake(g_vsync, pdMS_TO_TICKS(40));
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
  uint8_t yr; i2cRead(RTC_ADDR, 0x0A, &yr, 1);
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

void setBacklight(uint8_t pct) { if (pct > 100) pct = 100; ledcWrite(PIN_BL, pct == 100 ? 1023 : pct * 10); }
void buzzer(bool on) { static int last = -1; if ((int)on == last) return; last = on; expSet(EX_BUZZER, on); }

static const char* slotKey(int slot) { static char k[4]; snprintf(k, sizeof k, "s%d", slot); return k; }
bool saveBlob(int slot, const void* d, size_t n) { return g_prefs.putBytes(slotKey(slot), d, n) == n; }
size_t loadBlob(int slot, void* d, size_t n) { size_t have = g_prefs.getBytesLength(slotKey(slot)); return have && have <= n ? g_prefs.getBytes(slotKey(slot), d, have) : 0; }
size_t loadLegacyBlob(void* d, size_t n) { size_t have = g_prefs.getBytesLength("save"); return have && have <= n ? g_prefs.getBytes("save", d, have) : 0; }
void eraseBlob(int slot) { g_prefs.remove(slotKey(slot)); }
void eraseAll() { g_prefs.clear(); }
uint32_t freeHeap() { return (uint32_t)ESP.getFreeHeap(); }

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
  g_prefs.begin("crago", false);  // legacy NVS namespace: renaming it orphans every save already on a device
  setBacklight(100);
}
}  // namespace board
