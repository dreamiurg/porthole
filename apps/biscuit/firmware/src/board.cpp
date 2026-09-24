#include "board.h"
#include <Arduino.h>
#include <Wire.h>
#include <cstring>
#include <ctime>
#include "driver/spi_master.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"

// Pin mapping and ST7701S register sequence adapted from Waveshare's official
// ESP32-S3-Touch-LCD-2.1 Arduino demo, Display_ST7701.{h,cpp}.
// https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1/ESP32-S3-Touch-LCD-2.1-Demo.zip
// Schematic: same directory, ESP32-S3-Touch-LCD-2.1_schematic_diagram.pdf
// The downloaded board example includes no separate license notice.
// RTC register meanings: https://www.nxp.com/docs/en/data-sheet/PCF85063A.pdf
namespace {
constexpr uint8_t kExpander = 0x20, kTouch = 0x15, kRtc = 0x51;
constexpr uint8_t kLcdReset = 1 << 0, kTouchReset = 1 << 1, kLcdCs = 1 << 2;
constexpr int kWidth = 480, kHeight = 480, kBacklight = 6;
constexpr uint64_t kYear2000 = 946684800ULL, kYear2100 = 4102444800ULL;
esp_lcd_panel_handle_t panel = nullptr;
uint16_t* frameBuffer = nullptr;
bool touchReady = false, backlightReady = false;
uint8_t expanderOutput = 0x0F;

bool readRegister(uint8_t address, uint8_t reg, uint8_t* bytes, size_t count) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(address, count) != count) return false;
  for (size_t i = 0; i < count; ++i) bytes[i] = Wire.read();
  return true;
}

bool writeRegister(uint8_t address, uint8_t reg, const uint8_t* bytes, size_t count) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(bytes, count);
  return Wire.endTransmission() == 0;
}

bool writeByte(uint8_t address, uint8_t reg, uint8_t value) {
  return writeRegister(address, reg, &value, 1);
}

bool expanderPin(uint8_t mask, bool high) {
  const uint8_t next = high ? expanderOutput | mask : expanderOutput & ~mask;
  if (!writeByte(kExpander, 0x01, next)) return false;
  expanderOutput = next;
  return true;
}

bool resetPin(uint8_t mask) {
  if (!expanderPin(mask, false)) return false;
  delay(10);
  if (!expanderPin(mask, true)) return false;
  delay(50);
  return true;
}

bool checked(esp_err_t result, const char* operation) {
  if (result == ESP_OK) return true;
  Serial.printf("Board: %s failed: %s\n", operation, esp_err_to_name(result));
  return false;
}

struct PanelCommand { uint8_t command, length, data[16]; uint16_t pauseMs; };
constexpr PanelCommand kPanelCommands[] = {
  {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x10}, 0},
  {0xC0, 2, {0x3B, 0x00}, 0},
  {0xC1, 2, {0x0B, 0x02}, 0},
  {0xC2, 2, {0x07, 0x02}, 0},
  {0xCC, 1, {0x10}, 0},
  {0xCD, 1, {0x08}, 0},
  {0xB0, 16, {0x00, 0x11, 0x16, 0x0E, 0x11, 0x06, 0x05, 0x09, 0x08, 0x21, 0x06, 0x13, 0x10, 0x29, 0x31, 0x18}, 0},
  {0xB1, 16, {0x00, 0x11, 0x16, 0x0E, 0x11, 0x07, 0x05, 0x09, 0x09, 0x21, 0x05, 0x13, 0x11, 0x2A, 0x31, 0x18}, 0},
  {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x11}, 0},
  {0xB0, 1, {0x6D}, 0}, {0xB1, 1, {0x37}, 0},
  {0xB2, 1, {0x81}, 0}, {0xB3, 1, {0x80}, 0},
  {0xB5, 1, {0x43}, 0}, {0xB7, 1, {0x85}, 0},
  {0xB8, 1, {0x20}, 0}, {0xC1, 1, {0x78}, 0},
  {0xC2, 1, {0x78}, 0}, {0xD0, 1, {0x88}, 0},
  {0xE0, 3, {0x00, 0x00, 0x02}, 0},
  {0xE1, 11, {0x03, 0xA0, 0x00, 0x00, 0x04, 0xA0, 0x00, 0x00, 0x00, 0x20, 0x20}, 0},
  {0xE2, 13, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
  {0xE3, 4, {0x00, 0x00, 0x11, 0x00}, 0},
  {0xE4, 2, {0x22, 0x00}, 0},
  {0xE5, 16, {0x05, 0xEC, 0xA0, 0xA0, 0x07, 0xEE, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
  {0xE6, 4, {0x00, 0x00, 0x11, 0x00}, 0},
  {0xE7, 2, {0x22, 0x00}, 0},
  {0xE8, 16, {0x06, 0xED, 0xA0, 0xA0, 0x08, 0xEF, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
  {0xEB, 7, {0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00}, 0},
  {0xED, 16, {0xFF, 0xFF, 0xFF, 0xBA, 0x0A, 0xBF, 0x45, 0xFF, 0xFF, 0x54, 0xFB, 0xA0, 0xAB, 0xFF, 0xFF, 0xFF}, 0},
  {0xEF, 6, {0x10, 0x0D, 0x04, 0x08, 0x3F, 0x1F}, 0},
  {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x13}, 0},
  {0xEF, 1, {0x08}, 0},
  {0xFF, 5, {0x77, 0x01, 0x00, 0x00, 0x00}, 0},
  {0x36, 1, {0x00}, 0},
  {0x3A, 1, {0x66}, 0}, // Vendor panel mode; host wiring remains RGB565.
  {0x11, 0, {}, 480}, {0x20, 0, {}, 120}, {0x29, 0, {}, 0},
};

bool panelCommands() {
  spi_bus_config_t bus = {};
  bus.mosi_io_num = 1;
  bus.miso_io_num = -1;
  bus.sclk_io_num = 2;
  bus.quadwp_io_num = -1;
  bus.quadhd_io_num = -1;
  bus.max_transfer_sz = 64;
  if (!checked(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO), "SPI init")) return false;
  spi_device_interface_config_t config = {};
  config.command_bits = 1; // Ninth bit distinguishes a command from its data.
  config.address_bits = 8;
  config.mode = 0;
  config.clock_speed_hz = 40000000;
  config.spics_io_num = -1; // CS belongs to the I2C expander.
  config.queue_size = 1;
  spi_device_handle_t spi = nullptr;
  bool success = checked(spi_bus_add_device(SPI2_HOST, &config, &spi), "SPI device");
  if (success) success = expanderPin(kLcdCs, false);
  delay(10);
  for (const auto& entry : kPanelCommands) {
    if (!success) break;
    spi_transaction_t transaction = {};
    transaction.addr = entry.command;
    success = checked(spi_device_transmit(spi, &transaction), "LCD command");
    transaction.cmd = 1;
    for (uint8_t i = 0; success && i < entry.length; ++i) {
      transaction.addr = entry.data[i];
      success = checked(spi_device_transmit(spi, &transaction), "LCD data");
    }
    if (entry.pauseMs) delay(entry.pauseMs);
  }
  if (!expanderPin(kLcdCs, true)) success = false;
  delay(10);
  if (spi) spi_bus_remove_device(spi);
  spi_bus_free(SPI2_HOST); // GPIO1/2 are also the SD bus; no task owns them afterward.
  return success;
}

bool rgbBegin() {
  esp_lcd_rgb_panel_config_t config = {};
  config.clk_src = LCD_CLK_SRC_DEFAULT;
  config.timings.pclk_hz = 16000000;
  config.timings.h_res = kWidth;
  config.timings.v_res = kHeight;
  config.timings.hsync_pulse_width = 8;
  config.timings.hsync_back_porch = 10;
  config.timings.hsync_front_porch = 50;
  config.timings.vsync_pulse_width = 3;
  config.timings.vsync_back_porch = 8;
  config.timings.vsync_front_porch = 8;
  config.timings.flags.pclk_active_neg = false;
  config.data_width = 16;
  config.bits_per_pixel = 16;
  config.num_fbs = 1;
  config.bounce_buffer_size_px = kWidth * 10;
  config.dma_burst_size = 64;
  config.hsync_gpio_num = 38;
  config.vsync_gpio_num = 39;
  config.de_gpio_num = 40;
  config.pclk_gpio_num = 41;
  config.disp_gpio_num = -1;
  const int pins[] = {5, 45, 48, 47, 21, 14, 13, 12, 11, 10, 9, 46, 3, 8, 18, 17};
  memcpy(config.data_gpio_nums, pins, sizeof(pins));
  config.flags.fb_in_psram = true;
  if (!checked(esp_lcd_new_rgb_panel(&config, &panel), "RGB allocation")) return false;
  void* pixels = nullptr;
  const bool success = checked(esp_lcd_panel_reset(panel), "RGB reset") &&
    checked(esp_lcd_panel_init(panel), "RGB init") &&
    checked(esp_lcd_rgb_panel_get_frame_buffer(panel, 1, &pixels), "RGB framebuffer");
  if (!success) {
    esp_lcd_panel_del(panel);
    panel = nullptr;
    return false;
  }
  frameBuffer = static_cast<uint16_t*>(pixels);
  memset(frameBuffer, 0, kWidth * kHeight * sizeof(uint16_t));
  return true;
}

int fromBcd(uint8_t value) {
  if ((value & 15) > 9 || (value >> 4) > 9) return -1;
  return (value >> 4) * 10 + (value & 15);
}
uint8_t toBcd(int value) { return (value / 10) * 16 + value % 10; }
bool leapYear(int year) { return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); }
int monthDays(int year, int month) {
  constexpr int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return days[month - 1] + (month == 2 && leapYear(year));
}
} // namespace

bool boardBegin() {
  if (panel && touchReady) return true;
  Serial.printf("Board: PSRAM=%u bytes\n", ESP.getPsramSize());
  if (!psramFound()) { Serial.println("Board: OPI PSRAM is required"); return false; }
  pinMode(kBacklight, OUTPUT);
  digitalWrite(kBacklight, LOW);
  if (!Wire.begin(15, 7, 100000)) { Serial.println("Board: I2C init failed"); return false; }
  Wire.setTimeOut(50);
  // P0..3: LCD reset, touch reset, LCD CS, SD CS. P7: buzzer (off).
  // P4..6 are interrupt INPUTS: IMU INT2, IMU INT1, RTC INT (schematic).
  expanderOutput = 0x0F;
  uint8_t configuration = 0;
  if (!writeByte(kExpander, 0x01, expanderOutput) || !writeByte(kExpander, 0x03, 0x70) ||
      !readRegister(kExpander, 0x03, &configuration, 1) || configuration != 0x70) {
    Serial.println("Board: TCA9554 unavailable");
    return false;
  }
  pinMode(16, INPUT_PULLUP);
  uint8_t identity[3] = {};
  touchReady = resetPin(kTouchReset) && writeByte(kTouch, 0xFE, 0xFF) &&
    readRegister(kTouch, 0xA7, identity, sizeof(identity));
  Serial.printf("Board: TCA9554=ready CST820=%s id=%02X project=%02X firmware=%02X\n",
    touchReady ? "responding" : "unavailable", identity[0], identity[1], identity[2]);
  if (!touchReady) return false;
  uint8_t rtcControl = 0;
  const bool rtcPresent = readRegister(kRtc, 0x00, &rtcControl, 1);
  Serial.printf("Board: PCF85063=%s UTC=%llu\n", rtcPresent ? "responding" : "unavailable", boardClockRead());
  if (!resetPin(kLcdReset) || !panelCommands() || !rgbBegin()) return false;
  backlightReady = ledcAttach(kBacklight, 20000, 10);
  if (!backlightReady) { Serial.println("Board: backlight PWM init failed"); return false; }
  boardBrightness(50);
  Serial.println("Board: RGB driver ready, 480x480 RGB565, 16MHz, PSRAM framebuffer, 10-line bounce");
  return true;
}

bool boardTouch(uint16_t& x, uint16_t& y) {
  uint8_t bytes[6] = {};
  if (!touchReady || !readRegister(kTouch, 0x01, bytes, sizeof(bytes)) || bytes[1] != 1) return false;
  const uint16_t nextX = ((bytes[2] & 15) << 8) | bytes[3];
  const uint16_t nextY = ((bytes[4] & 15) << 8) | bytes[5];
  if (nextX >= kWidth || nextY >= kHeight) return false;
  x = nextX;
  y = nextY;
  return true;
}

void boardFlush(int x1, int y1, int x2, int y2, const uint16_t* pixels) {
  if (!panel || !pixels || x1 < 0 || y1 < 0 || x2 >= kWidth || y2 >= kHeight || x2 < x1 || y2 < y1) return;
  // With an external LVGL draw buffer, draw_bitmap copies into the one RGB framebuffer.
  checked(esp_lcd_panel_draw_bitmap(panel, x1, y1, x2 + 1, y2 + 1, pixels), "RGB flush");
}

void boardBrightness(uint8_t percent) {
  if (!backlightReady) return;
  const uint32_t level = percent > 100 ? 100 : percent;
  ledcWrite(kBacklight, (level * 1023 + 50) / 100);
}

const uint16_t* boardFrameBuffer() { return frameBuffer; }

uint64_t boardClockRead() {
  uint8_t control = 0, bytes[7] = {};
  if (!readRegister(kRtc, 0x00, &control, 1) || (control & 0xA0) ||
      !readRegister(kRtc, 0x04, bytes, sizeof(bytes)) || (bytes[0] & 0x80)) return 0;
  const int second = fromBcd(bytes[0] & 0x7F), minute = fromBcd(bytes[1] & 0x7F);
  int hour = fromBcd(bytes[2] & ((control & 0x02) ? 0x1F : 0x3F));
  if (control & 0x02) {
    if (hour < 1 || hour > 12) return 0;
    hour = hour % 12 + ((bytes[2] & 0x20) ? 12 : 0);
  }
  const int day = fromBcd(bytes[3] & 0x3F), month = fromBcd(bytes[5] & 0x1F);
  const int shortYear = fromBcd(bytes[6]), year = 2000 + shortYear;
  if (second < 0 || second > 59 || minute < 0 || minute > 59 || hour < 0 || hour > 23 ||
      shortYear < 0 || month < 1 || month > 12 || day < 1 || day > monthDays(year, month) || (bytes[4] & 7) > 6) return 0;
  uint64_t days = day - 1;
  for (int y = 2000; y < year; ++y) days += leapYear(y) ? 366 : 365;
  for (int m = 1; m < month; ++m) days += monthDays(year, m);
  return kYear2000 + days * 86400 + hour * 3600 + minute * 60 + second;
}

bool boardClockSet(uint64_t epochSeconds) {
  if (epochSeconds < kYear2000 || epochSeconds >= kYear2100) return false;
  const time_t epoch = static_cast<time_t>(epochSeconds);
  tm utc = {};
  if (!gmtime_r(&epoch, &utc)) return false;
  uint8_t control = 0;
  if (!readRegister(kRtc, 0x00, &control, 1)) return false;
  const uint8_t running = control & ~0xB2; // Normal operation, running, 24-hour clock.
  if (!writeByte(kRtc, 0x00, running | 0x20)) return false;
  const uint8_t bytes[] = {toBcd(utc.tm_sec), toBcd(utc.tm_min), toBcd(utc.tm_hour),
    toBcd(utc.tm_mday), toBcd(utc.tm_wday), toBcd(utc.tm_mon + 1), toBcd(utc.tm_year - 100)};
  // Leave STOP set after a failed transfer: a partial date must not look valid.
  if (!writeRegister(kRtc, 0x04, bytes, sizeof(bytes))) return false;
  return writeByte(kRtc, 0x00, running); // Date write also cleared the oscillator-stop flag.
}
