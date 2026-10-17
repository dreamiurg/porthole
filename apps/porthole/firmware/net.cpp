// Wireless updates: once a local day, a minute after boot, a low-priority task on core 0 joins the saved wifi, asks
// GitHub for the newest release on the channel and, if it is newer, streams it into the spare OTA slot and sets that
// slot to start on the next power-on. The game keeps running on core 1 throughout. Wifi is off again afterwards.
#include "net.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <SD_MMC.h>
#include <Update.h>
#include <WiFi.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include "board.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"

// ESP-IDF's built-in root CA bundle (CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL), linked in from libmbedtls.
extern const uint8_t CA_BUNDLE[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t CA_BUNDLE_END[] asm("_binary_x509_crt_bundle_end");

namespace net {
namespace {
// NVS namespace "sys" (new with wireless updates; never reuse a shipped one). Keys: "ssid", "pass" (raw bytes, no
// NUL; no "pass" = open network), "chan" (one byte, shell::Channel), "last" (uint32 local epoch of the last attempt:
// its day is the once-a-day gate). "ran" is reserved for the shell's "something new" card.
const char* const NS = "sys";
const char* const RELEASES = "https://github.com/dreamiurg/porthole/releases/";
const char* const BETA_API = "https://api.github.com/repos/dreamiurg/porthole/releases?per_page=1";
const char* const TAG = "\"tag_name\":\"porthole-v";
const uint32_t FIRST_MS = 60000, POLL_MS = 60000;   // first check a minute after boot, then a look once a minute
const uint32_t WIFI_MS = 20000;                    // to join the network
const uint16_t HTTP_MS = 15000;                    // TCP connect, and the longest wait for the next bytes
const unsigned long TLS_S = 15;                    // TLS handshake, seconds
const size_t CHUNK = 4096;                         // one flash sector per write
const uint32_t STACK = 10240;                      // bytes; the Arduino loop task does HTTPS in 8192

struct Wifi { char ssid[33]; char pass[65]; };

uint32_t (*g_now)() = nullptr;
bool g_sd = false;
uint32_t g_nextPoll = FIRST_MS;
// RAM copies of "sys"/ssid and "sys"/chan, so the menu can read them every frame without opening NVS. Main loop only.
char g_ssid[33] = {0};
shell::Channel g_chan = shell::CH_STABLE;
std::atomic<bool> g_busy{false};   // a check task exists
portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;
shell::UpdStatus g_st;             // written by the task, read by the main loop: only under g_mux

void setState(shell::UpdState s, uint8_t pct = 0) {
  portENTER_CRITICAL(&g_mux); g_st.state = s; g_st.pct = pct; portEXIT_CRITICAL(&g_mux);
}

size_t loadStr(const char* key, char* buf, size_t max) {   // NUL-terminated; 0 if missing
  size_t n = board::loadBlob(NS, key, buf, max - 1);
  buf[n] = 0;
  return n;
}
bool loadWifi(Wifi& w) { loadStr("pass", w.pass, sizeof w.pass); return loadStr("ssid", w.ssid, sizeof w.ssid) > 0; }

bool storeWifi(const char* ssid, const char* pass) {
  size_t n = strlen(ssid), p = strlen(pass);
  if (!n || n > 32 || p > 64) return false;
  if (!board::saveBlob(NS, "ssid", ssid, n)) return false;
  snprintf(g_ssid, sizeof g_ssid, "%s", ssid);
  if (p) return board::saveBlob(NS, "pass", pass, p);
  board::eraseBlob(NS, "pass");
  return true;
}

bool isRelease() { int v[3]; return shell::parseVersion(PORTHOLE_VERSION, v); }

// One HTTPS client set up the same way for every request: the CA bundle, timeouts, redirects (GitHub sends a release
// asset from github.com to another host; HTTPClient reconnects when the host changes) and a User-Agent (the API
// requires one).
struct Https {
  NetworkClientSecure tls;
  HTTPClient http;
  int get(const char* url) {
    tls.setCACertBundle(CA_BUNDLE, (size_t)(CA_BUNDLE_END - CA_BUNDLE));
    tls.setHandshakeTimeout(TLS_S);
    if (!http.begin(tls, url)) return -1;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setConnectTimeout(HTTP_MS);
    http.setTimeout(HTTP_MS);
    http.setUserAgent("porthole/" PORTHOLE_VERSION);
    int code = http.GET();
    if (code != 200) Serial.printf("[net] GET %s -> %d\n", url, code);
    return code;
  }
  ~Https() { http.end(); }
};

enum Found { F_ERROR, F_NONE, F_VERSION };

// The newest version on the channel into out ("X.Y.Z"). Stable: the newest non-pre-release's porthole-version.txt
// (404 until a release that has one is promoted: nothing to offer, not a failure). Beta: the newest release of all.
Found fetchAvailable(shell::Channel ch, char out[16]) {
  Https c;
  int code = c.get(ch == shell::CH_BETA ? BETA_API : "https://github.com/dreamiurg/porthole/releases/latest/download/porthole-version.txt");
  if (code == 404 && ch == shell::CH_STABLE) return F_NONE;
  if (code != 200) return F_ERROR;
  String body = c.http.getString();
  if (ch == shell::CH_BETA) {
    int at = body.indexOf(TAG);
    if (at < 0) { Serial.println("[net] newest release is not a porthole one"); return F_NONE; }
    at += (int)strlen(TAG);
    body = body.substring(at, body.indexOf('"', at));
  }
  body.trim();
  int v[3];
  if (body.length() >= 16 || !shell::parseVersion(body.c_str(), v)) { Serial.println("[net] bad version text"); return F_ERROR; }
  strcpy(out, body.c_str());
  return F_VERSION;
}

// Body bytes into Update, a sector at a time, pausing between writes so core 0's idle task and the wifi stack run.
bool stream(HTTPClient& http, size_t size) {
  std::unique_ptr<uint8_t[]> buf(new (std::nothrow) uint8_t[CHUNK]);
  NetworkClient* s = http.getStreamPtr();
  if (!buf || !s) return false;
  size_t got = 0;
  for (uint32_t last = millis(); got < size && millis() - last < HTTP_MS;) {
    size_t n = (size_t)s->available();
    if (!n) { if (!http.connected()) break; vTaskDelay(pdMS_TO_TICKS(5)); continue; }
    int r = s->read(buf.get(), std::min(std::min(n, CHUNK), size - got));
    if (r <= 0) { vTaskDelay(pdMS_TO_TICKS(5)); continue; }
    if (Update.write(buf.get(), (size_t)r) != (size_t)r) return false;
    got += (size_t)r; last = millis();
    setState(shell::UP_DOWNLOADING, (uint8_t)(got * 100 / size));
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  return got == size;
}

// porthole-<ver>-ota.bin into the spare slot. Update.end() then sets that slot to boot, and esp_ota_set_boot_partition
// first verifies the whole image, its appended SHA256 included; a failure leaves the running firmware's slot booting.
bool download(const char* ver) {
  char url[128];
  snprintf(url, sizeof url, "%sdownload/porthole-v%s/porthole-%s-ota.bin", RELEASES, ver, ver);
  Https c;
  int code = c.get(url), size = c.http.getSize();
  Serial.printf("[net] downloading %s: %d bytes\n", ver, size);
  setState(shell::UP_DOWNLOADING);
  // A known size, never UPDATE_SIZE_UNKNOWN: that erases the whole 7.9 MB slot first, stalling both cores.
  bool ok = code == 200 && size > 0 && Update.begin((size_t)size) && stream(c.http, (size_t)size) && Update.end();
  if (!ok) { Serial.printf("[net] download failed: %s\n", Update.errorString()); Update.abort(); }
  return ok;
}

bool joinWifi(const Wifi& w) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(w.ssid, w.pass[0] ? w.pass : nullptr);
  for (uint32_t t0 = millis(); WiFi.status() != WL_CONNECTED; vTaskDelay(pdMS_TO_TICKS(250)))
    if (millis() - t0 > WIFI_MS) { Serial.printf("[net] could not join %s\n", w.ssid); return false; }
  Serial.printf("[net] joined %s rssi=%d heap_internal=%lu largest_internal=%lu\n", w.ssid, (int)WiFi.RSSI(),
                (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  return true;
}

void checkAndDownload(shell::Channel ch) {
  char avail[16] = {0};
  Found f = fetchAvailable(ch, avail);
  if (f == F_ERROR) { setState(shell::UP_FAILED); return; }
  portENTER_CRITICAL(&g_mux); strcpy(g_st.available, avail); portEXIT_CRITICAL(&g_mux);
  Serial.printf("[net] %s channel: %s, running %s\n", ch == shell::CH_BETA ? "beta" : "stable", f == F_VERSION ? avail : "none",
                PORTHOLE_VERSION);
  if (!shell::newer(avail, PORTHOLE_VERSION)) { setState(shell::UP_CURRENT); return; }
  bool ok = download(avail);
  setState(ok ? shell::UP_READY : shell::UP_FAILED);
  if (ok) Serial.printf("[net] %s ready: it starts on the next power-on\n", avail);
}

// TODO(porthole): sync the clock by NTP while connected. The RTC and the whole firmware keep local wall-clock seconds
// with no timezone anywhere on the device, so UTC from NTP cannot be turned into local time without a timezone setting.
void run(shell::Channel ch) {
  Wifi w;
  if (!loadWifi(w)) { setState(shell::UP_NO_WIFI); return; }
  uint32_t now = g_now();
  board::saveBlob(NS, "last", &now, sizeof now);   // before connecting: a crash below can't retry on every boot
  portENTER_CRITICAL(&g_mux); g_st.lastCheck = now; portEXIT_CRITICAL(&g_mux);
  setState(shell::UP_CHECKING);
  if (joinWifi(w)) checkAndDownload(ch);
  else setState(shell::UP_NO_WIFI);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void task(void* arg) {
  run((shell::Channel)(uintptr_t)arg);
  g_busy = false;
  vTaskDelete(nullptr);
}

void spawn(shell::Channel ch) {
  if (g_busy.exchange(true)) return;
  // Downloaded already: a second run would rewrite the slot that is set to boot, and a failure would leave it broken.
  portENTER_CRITICAL(&g_mux); bool ready = g_st.state == shell::UP_READY; portEXIT_CRITICAL(&g_mux);
  if (ready) { g_busy = false; return; }
  if (xTaskCreatePinnedToCore(task, "porthole-net", STACK, (void*)(uintptr_t)ch, 1, nullptr, 0) != pdPASS) {
    g_busy = false;
    Serial.println("[net] no memory for the update task");
  }
}

// /porthole/wifi.txt on the card: line 1 the network, line 2 its password. Saved when it differs from "sys".
void wifiFromSd() {
  File f = SD_MMC.open("/porthole/wifi.txt");
  if (!f) return;
  String ssid = f.readStringUntil('\n'), pass = f.readStringUntil('\n');
  f.close();
  for (String* s : {&ssid, &pass}) while (s->endsWith("\r") || s->endsWith("\n")) s->remove(s->length() - 1);
  Wifi w;
  if (loadWifi(w) && ssid == w.ssid && pass == w.pass) return;
  if (storeWifi(ssid.c_str(), pass.c_str())) Serial.printf("[net] wifi from SD card saved: %s\n", ssid.c_str());
}

const char* resetName(esp_reset_reason_t r) {
  static const char* const NAMES[] = {"unknown", "power-on", "external", "restart", "panic", "int-wdt", "task-wdt",
                                      "wdt", "deep-sleep", "brownout", "sdio", "usb", "jtag", "efuse", "power-glitch",
                                      "cpu-lockup"};
  return (unsigned)r < sizeof NAMES / sizeof NAMES[0] ? NAMES[r] : "unknown";
}
}  // namespace

void Updater::begin(uint32_t (*localNow)()) {
  g_now = localNow;
  WiFi.persistent(false);   // the network lives in "sys"; keep the wifi driver from saving its own copy
  uint32_t last = 0;
  if (board::loadBlob(NS, "last", &last, sizeof last) == sizeof last) g_st.lastCheck = last;
  Wifi w;
  if (loadWifi(w)) strcpy(g_ssid, w.ssid);
  uint8_t c = 0;
  board::loadBlob(NS, "chan", &c, 1);
  g_chan = c == shell::CH_BETA ? shell::CH_BETA : shell::CH_STABLE;
  g_sd = board::mountSd();
  Serial.printf("[net] version %s, SD card %s\n", PORTHOLE_VERSION, g_sd ? "mounted" : "not found");
  if (g_sd) wifiFromSd();
}

void Updater::tick(uint32_t ms) {
  if ((int32_t)(ms - g_nextPoll) < 0) return;
  g_nextPoll = ms + POLL_MS;
  if (!isRelease() || !g_ssid[0] || status().lastCheck / 86400u == g_now() / 86400u) return;   // dev, no wifi, done today
  spawn(channel());
}

bool Updater::saveWifi(const char* ssid, const char* pass) { return storeWifi(ssid, pass); }

void Updater::info(shell::SysInfo& out) {
  out = shell::SysInfo{};
  out.version = PORTHOLE_VERSION;
  const esp_partition_t* run = esp_ota_get_running_partition();
  if (run) snprintf(out.slot, sizeof out.slot, "%.7s", run->label);   // "ota_0" / "ota_1"
  snprintf(out.reset, sizeof out.reset, "%s", resetName(esp_reset_reason()));
  out.heapFree = (uint32_t)heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  out.sd = g_sd;
  out.wifiSaved = g_ssid[0] != 0;
  strcpy(out.ssid, g_ssid);
}

shell::UpdStatus Updater::status() {
  portENTER_CRITICAL(&g_mux);
  shell::UpdStatus s = g_st;
  portEXIT_CRITICAL(&g_mux);
  return s;
}

void Updater::checkNow() { spawn(channel()); }
void Updater::restart() { ESP.restart(); }

shell::Channel Updater::channel() { return g_chan; }
void Updater::setChannel(shell::Channel c) { g_chan = c; uint8_t b = c; board::saveBlob(NS, "chan", &b, 1); }

}  // namespace net
