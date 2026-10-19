// The RGB panel normally scans PSRAM, which is unavailable during flash writes. During an explicit update,
// circular DMA lists reuse black rows around small text strips in internal RAM. No ISR refill is needed.
// This integration targets the pinned IDF 5.4 driver; --wrap captures its LCD DMA channel without private structs.
#include "update_scanout.h"
#include "update_picture.h"
#include <Arduino.h>
#include <new>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_private/gdma.h"
#include "esp_private/gdma_link.h"
#include "esp_rom_sys.h"
#include "hal/lcd_ll.h"

static gdma_channel_handle_t g_lcdDma = nullptr;
static int g_lcdCore = -1;
extern "C" esp_err_t __real_gdma_connect(gdma_channel_handle_t channel, gdma_trigger_t trigger);
extern "C" esp_err_t __wrap_gdma_connect(gdma_channel_handle_t channel, gdma_trigger_t trigger) {
  esp_err_t result = __real_gdma_connect(channel, trigger);
  if (result == ESP_OK && trigger.periph == GDMA_TRIG_PERIPH_LCD && trigger.instance_id == SOC_GDMA_TRIG_PERIPH_LCD0) {
    g_lcdDma = channel;
    g_lcdCore = xPortGetCoreID();
  }
  return result;
}

namespace update_scanout {
static board::UpdatePicture* g_picture = nullptr;
static gdma_link_list_handle_t g_links[2] = {};
static int g_bank = 0;
static uint32_t g_lastDraw = 0;
static char g_label[24] = {};
static bool g_cancellable = true;
// At 16 MHz the 548 x 499 timing is 17.1 ms/frame. Two frames let a tail link switch finish before reuse.
static constexpr uint32_t SCAN_SETTLE_MS = 40;

static bool makeList(int bank) {
  gdma_link_list_config_t config = {};
  config.num_items = board::UpdatePicture::CHUNKS;
  config.item_alignment = 4; config.buffer_alignment = 64;
  config.flags.check_owner = true;
  if (gdma_new_link_list(&config, &g_links[bank]) != ESP_OK) return false;
  for (int i = 0; i < board::UpdatePicture::CHUNKS; i++) {
    gdma_buffer_mount_config_t buffer = {};
    buffer.buffer = (void*)g_picture->chunk(i * board::UpdatePicture::CHUNK_ROWS, bank);
    buffer.length = board::UpdatePicture::CHUNK_BYTES;
    // Neither EOF nor final: all 120 descriptors form a hardware-driven circular frame.
    if (gdma_link_mount_buffers(g_links[bank], i, &buffer, 1, nullptr) != ESP_OK) return false;
  }
  return true;
}

void release() {
  for (auto& list : g_links) if (list) { ESP_ERROR_CHECK(gdma_del_link_list(list)); list = nullptr; }
  if (g_picture) { g_picture->~UpdatePicture(); heap_caps_free(g_picture); g_picture = nullptr; }
}

bool begin() {
  if (!g_lcdDma || xPortGetCoreID() != g_lcdCore) return false;
  void* memory = heap_caps_aligned_calloc(64, 1, sizeof(board::UpdatePicture), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
  if (!memory) return false;
  g_picture = new (memory) board::UpdatePicture{};
  g_picture->drawBack();
  g_picture->drawProgress(0, "Checking..."); g_picture->drawProgress(1, "Checking...");
  if (!makeList(0) || !makeList(1)) { release(); return false; }
  // Same core as the stock ISR: it cannot be in flight while we mask its peripheral source.
  // Its per-vsync DMA restart must stay disabled through flash writes, including delayed interrupts.
  lcd_ll_enable_interrupt(&LCD_CAM, LCD_LL_EVENT_VSYNC_END, false);
  lcd_ll_clear_interrupt_status(&LCD_CAM, UINT32_MAX);
  lcd_ll_stop(&LCD_CAM);
  ESP_ERROR_CHECK(gdma_stop(g_lcdDma)); ESP_ERROR_CHECK(gdma_reset(g_lcdDma));
  lcd_ll_reset(&LCD_CAM); lcd_ll_fifo_reset(&LCD_CAM);
  g_bank = 0; g_cancellable = true; strcpy(g_label, "Checking..."); g_lastDraw = millis();
  ESP_ERROR_CHECK(gdma_start(g_lcdDma, gdma_link_get_head_addr(g_links[0])));
  esp_rom_delay_us(1); lcd_ll_start(&LCD_CAM);
  delay(SCAN_SETTLE_MS);   // show a complete SRAM frame before acknowledging that flash writes may start
  return true;
}

static void setCancelStripe(int bank, bool cancellable) {
  for (int y = board::UpdatePicture::BACK_Y; y < board::UpdatePicture::BACK_Y + board::UpdatePicture::TEXT_ROWS;
       y += board::UpdatePicture::CHUNK_ROWS) {
    gdma_buffer_mount_config_t buffer = {};
    buffer.buffer = cancellable ? g_picture->chunk(y, bank) : g_picture->black;
    buffer.length = board::UpdatePicture::CHUNK_BYTES;
    // TX does not return descriptor ownership. This list is inactive after the two-frame swap wait.
    ESP_ERROR_CHECK(gdma_link_set_owner(g_links[bank], y / board::UpdatePicture::CHUNK_ROWS, GDMA_LLI_OWNER_CPU));
    ESP_ERROR_CHECK(gdma_link_mount_buffers(g_links[bank], y / board::UpdatePicture::CHUNK_ROWS, &buffer, 1, nullptr));
  }
}

void draw(const char* label, bool cancellable) {
  if (!g_picture) return;
  if (g_cancellable == cancellable && (!strcmp(label, g_label) || millis() - g_lastDraw < 250)) return;
  int next = 1 - g_bank;
  g_picture->drawProgress(next, label);
  setCancelStripe(next, cancellable);
  // The inactive list loops on itself; switch only at the end of a whole frame, with no LCD/DMA reset.
  ESP_ERROR_CHECK(gdma_link_concat(g_links[next], -1, g_links[next], 0));
  ESP_ERROR_CHECK(gdma_link_concat(g_links[g_bank], -1, g_links[next], 0));
  delay(SCAN_SETTLE_MS);
  g_bank = next; g_cancellable = cancellable; g_lastDraw = millis(); snprintf(g_label, sizeof g_label, "%s", label);
}

void stop() {
  lcd_ll_stop(&LCD_CAM);
  ESP_ERROR_CHECK(gdma_stop(g_lcdDma)); ESP_ERROR_CHECK(gdma_reset(g_lcdDma));
  lcd_ll_clear_interrupt_status(&LCD_CAM, UINT32_MAX);   // do not replay a stale vsync on restoring the driver
}
bool hitCancel(int x, int y) { return board::UpdatePicture::hitBack(x, y); }
}  // namespace update_scanout
