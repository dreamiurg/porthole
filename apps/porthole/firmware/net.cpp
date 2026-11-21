// Wireless updates: once a local day, a minute after boot, a low-priority task on core 0 joins the saved wifi, asks
// GitHub for the newest release on the channel and, if it is newer, streams it into the spare OTA slot and sets that
// slot to start on the next power-on. The game keeps running on core 1 throughout. Wifi is off again afterwards.
// The system menu's wifi setup (a scan, a test join), its Home lookup (Nominatim) and Radar's plane feed (adsb.fi, routes
// from adsbdb.com) run the same way, one job on the radio at a time.
#include "net.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <SD_MMC.h>
#include <Update.h>
#include "update_gate.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <sys/time.h>
#include "esp_random.h"
#include "netclock.h"
#include "crc32.h"
#include "psram.h"
#include "telemetry.h"
#include "timezone.h"
#include <algorithm>
#include <atomic>
#include <math.h>
#include <memory>
#include "board.h"
#include "psram.h"
#include "esp_heap_caps.h"
#include "esp_ota_ops.h"

// ESP-IDF's built-in root CA bundle (CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_FULL), linked in from libmbedtls.
extern const uint8_t CA_BUNDLE[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t CA_BUNDLE_END[] asm("_binary_x509_crt_bundle_end");

namespace net {
namespace {
// NVS namespace "sys" (new with wireless updates; never reuse a shipped one). Keys: "ssid", "pass" (raw bytes, no
// NUL; no "pass" = open network), "chan" (one byte, shell::Channel), "last" (uint32 local epoch of the last attempt:
// its day is the once-a-day gate), "home" (Radar's center: shell::Home's 112 bytes as they are), "tried" (the version
// the last finished download put in the spare slot, as text: shell::rolledBack). "lite" is the shell's ("ran" was, until Kid news).
const char* const NS = "sys";
#define RELEASES "https://github.com/dreamiurg/porthole/releases/"
// Each channel's newest version as plain "X.Y.Z". Stable: GitHub's Latest release (pre-releases never are). Beta: the
// rolling porthole-beta pre-release, which the release workflow re-points at every new release. The releases API is no
// help for beta: it does not list releases newest first.
const char* const STABLE_VERSION = RELEASES "latest/download/porthole-version.txt";
const char* const BETA_VERSION = RELEASES "download/porthole-beta/porthole-version.txt";
const uint32_t FIRST_MS = 60000, POLL_MS = 60000;   // first check a minute after boot, then a look once a minute
const uint32_t CLOCK_MS = 5000, CLOCK_POLL_MS = 6 * 3600000u, CLOCK_RETRY_MS = 15 * 60000u;
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
std::atomic<bool> g_dark{false};   // idle dimming wants the screen off (daily-check gate)
std::atomic<bool> g_forced{false}; // Check now continues even when touch or motion requests a lit screen
UpdateGate g_screen;
std::atomic<bool> g_woke{false};   // the screen came on mid-download: stopped, to resume in the next dark spell
portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;
shell::ClockStatus g_clock;
bool g_clockPending = false;
uint32_t g_clockUtc = 0, g_clockAt = 0;
std::atomic<uint32_t> g_nextClock{3000};
timezones::Settings g_zoneSettings = timezones::settings(timezones::DEFAULT_ZONE, true);
uint32_t g_zoneGeneration = 0, g_zoneJobToken = 0;
std::atomic<uint32_t> g_clockSequence{0};
timezones::DetectionRetry g_zoneRetry;
bool g_zoneNeeded = true, g_zonePending = false, g_zoneResultOk = false;
char g_zoneResult[64] = {};
uint32_t g_zoneResultToken = 0;
uint32_t g_checkNow = 0;   // main-loop snapshot; workers never read the main-loop clock variables
shell::UpdStatus g_st;             // written by the task, read by the main loop: only under g_mux
// The radio's jobs, one task at a time (g_busy): the update check (on g_checkChan, set before it starts), a scan or a
// test join from the menu's Wifi screens. g_scanCount and g_tested are -1 while theirs runs; g_found belongs to the scan
// task until it sets g_scanCount, and g_try (the network to test) to the test task while it runs. The Home lookup and
// Radar's feed (below) are the other two.
enum Job : uintptr_t { J_CHECK, J_SCAN, J_TEST, J_LOOK, J_RADAR, J_CLOCK, J_ZONE, J_LAN, J_TELEM };
shell::Channel g_checkChan = shell::CH_STABLE;
shell::Network g_found[shell::MAX_NETWORKS];
std::atomic<int> g_scanCount{0}, g_tested{0};
Wifi g_try;

void setState(shell::UpdState s, uint8_t pct = 0) {
  portENTER_CRITICAL(&g_mux); g_st.state = s; g_st.pct = pct; portEXIT_CRITICAL(&g_mux);
}

size_t loadStr(const char* key, char* buf, size_t max) {   // NUL-terminated; 0 if missing
  size_t n = board::loadBlob(NS, key, buf, max - 1);
  buf[n] = 0;
  return n;
}
bool loadWifi(Wifi& w) { loadStr("pass", w.pass, sizeof w.pass); return loadStr("ssid", w.ssid, sizeof w.ssid) > 0; }

// A saved network change also invalidates a successful guess from the previous network, including across reboot.
// Write this first: if storage fails, keep the old credentials rather than retaining a falsely valid cache for new wifi.
bool invalidateDetectedZone() {
  ++g_zoneGeneration;
  if (!g_clock.automaticZone) return true;
  g_zoneNeeded = true;
  if (!g_zoneSettings.reserved[0]) return true;
  const auto pending = timezones::settings(g_zoneSettings.zone, true);
  timezones::Settings checked{};
  if (!board::saveBlob(NS, "tz", &pending, sizeof pending) ||
      board::loadBlob(NS, "tz", &checked, sizeof checked) != sizeof checked || memcmp(&pending, &checked, sizeof pending)) return false;
  g_zoneSettings = pending;
  return true;
}

bool storeWifi(const char* ssid, const char* pass) {
  size_t n = strlen(ssid), p = strlen(pass);
  if (!n || n > 32 || p > 64) return false;
  if (!board::saveBlob(NS, "ssid", ssid, n)) return false;
  snprintf(g_ssid, sizeof g_ssid, "%s", ssid);
  if (p) return board::saveBlob(NS, "pass", pass, p);
  board::eraseBlob(NS, "pass");
  return true;
}

// "sys"/home: shell::Home as raw bytes (two doubles, then the NUL-terminated ASCII name). Never resized.
static_assert(sizeof(shell::Home) == 112, "the \"sys\"/home blob's layout");
shell::Home g_home;     // RAM copy of "sys"/home, for the menu (every frame) and Radar's spot(); main loop only
bool g_homeSet = false;
bool validSpot(double lat, double lon) { return lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180; }   // NaN: false

void wifiOff() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// ArduinoJson's pools in PSRAM: the internal heap is kept for the TLS session.
struct Psram : ArduinoJson::Allocator {
  void* allocate(size_t n) override { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
  void deallocate(void* p) override { heap_caps_free(p); }
  void* reallocate(void* p, size_t n) override { return heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
} g_psram;

bool isRelease() { int v[3]; return shell::parseVersion(PORTHOLE_VERSION, v); }

// One HTTPS client set up the same way for every request: the CA bundle, timeouts, redirects and a User-Agent that
// names the firmware version. Redirects are followed here, not by HTTPClient: GitHub sends a release asset from github.com to
// another host, and on that reconnect Arduino-ESP32 3.2.1's NetworkClientSecure has lost its CA bundle
// (stop_ssl_socket() zeroes the context, bundle callback included), so the handshake fails with "No CA Chain is set".
// setCACertBundle() before every hop attaches it again.
struct Https {
  NetworkClientSecure tls;
  HTTPClient http;
  bool http10 = false;   // an HTTP/1.0 request: never a chunked body, so deserializeJson can read the raw stream
  int get(const char* url) {
    String at = url;
    int code = -1;
    for (int hop = 0; hop < 5; hop++) {
      tls.setCACertBundle(CA_BUNDLE, (size_t)(CA_BUNDLE_END - CA_BUNDLE));
      tls.setHandshakeTimeout(TLS_S);
      if (!http.begin(tls, at)) return -1;
      http.setConnectTimeout(HTTP_MS);
      http.setTimeout(HTTP_MS);
      http.useHTTP10(http10);
      // Nominatim's usage policy asks for a User-Agent naming the application and a way to reach it.
      http.setUserAgent("porthole/" PORTHOLE_VERSION " (+https://github.com/dreamiurg/porthole)");
      code = http.GET();
      if (code < 300 || code > 399 || !http.getLocation().length()) break;
      String next = http.getLocation();
      if (next.startsWith("/")) next = at.substring(0, at.indexOf('/', 8)) + next;   // relative: same scheme and host
      at = next;
      http.end();
      tls.stop();   // now, before the bundle is attached again: begin() on a new host would stop it after
    }
    // The host only: a lookup's query is the typed address, adsb.fi's path is home.
    if (code != 200) Serial.printf("[net] GET %s -> %d\n", at.substring(0, (unsigned)at.indexOf('/', 8)).c_str(), code);
    return code;
  }
  ~Https() { http.end(); }
};

enum Found { F_ERROR, F_NONE, F_VERSION };

// The newest version on the channel into out ("X.Y.Z"). A 404 (no promoted release with the file yet, or no beta pointer
// yet) is nothing to offer, not a failure.
Found fetchAvailable(shell::Channel ch, char out[16]) {
  Https c;
  int code = c.get(ch == shell::CH_BETA ? BETA_VERSION : STABLE_VERSION);
  if (code == 404) return F_NONE;
  if (code != 200) return F_ERROR;
  String body = c.http.getString();
  body.trim();
  int v[3];
  if (body.length() >= 16 || !shell::parseVersion(body.c_str(), v)) { Serial.println("[net] bad version text"); return F_ERROR; }
  strcpy(out, body.c_str());
  return F_VERSION;
}

bool checkStopped() {
  if (!g_screen.cancelled() && (g_forced || g_dark)) return false;
  g_woke = true;
  return true;
}

// Check now keeps its SRAM display through abort, final verification and wifi cleanup.
bool stream(HTTPClient& http, size_t size) {
  std::unique_ptr<uint8_t[]> buf(new (std::nothrow) uint8_t[CHUNK]);
  NetworkClient* s = http.getStreamPtr();
  if (!buf || !s) return false;
  size_t got = 0;
  for (uint32_t last = millis(); got < size && millis() - last < HTTP_MS;) {
    if (checkStopped()) return false;
    size_t n = (size_t)s->available();
    if (!n) { if (!http.connected()) break; vTaskDelay(pdMS_TO_TICKS(5)); continue; }
    int r = s->read(buf.get(), std::min(std::min(n, CHUNK), size - got));
    if (r <= 0) { vTaskDelay(pdMS_TO_TICKS(5)); continue; }
    if (checkStopped()) return false;
    if (Update.write(buf.get(), (size_t)r) != (size_t)r) return false;
    got += (size_t)r; last = millis();
    setState(shell::UP_DOWNLOADING, (uint8_t)(got * 100 / size));
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  return got == size;
}

bool finishDownload() {
  if (checkStopped()) return false;
  // Cancel and the commit race through one atomic state: a tap can never promise cancellation after this point.
  if (!g_screen.beginCommit()) { g_woke = true; return false; }
  return Update.end();
}

// porthole-<ver>-ota.bin into the spare slot. Update.end() then sets that slot to boot, and esp_ota_set_boot_partition
// first verifies the whole image, its appended SHA256 included; a failure leaves the running firmware's slot booting.
bool download(const char* ver) {
  char url[128];
  snprintf(url, sizeof url, "%sdownload/porthole-v%s/porthole-%s-ota.bin", RELEASES, ver, ver);
  Https c;
  int code = c.get(url), size = c.http.getSize();
  if (checkStopped()) return false;
  Serial.printf("[net] downloading %s: %d bytes\n", ver, size);
  setState(shell::UP_DOWNLOADING);
  // A known size, never UPDATE_SIZE_UNKNOWN: that erases the whole 7.9 MB slot first, stalling both cores.
  bool ok = code == 200 && size > 0 && Update.begin((size_t)size) && stream(c.http, (size_t)size) && finishDownload();
  if (ok) return true;
  if (!g_woke) Serial.printf("[net] download failed: %s\n", Update.errorString());   // a wake is not a failure
  if (Update.isRunning()) Update.abort();   // never begun, abort() would drive Update's unset LED pin: GPIO0
  return false;
}

bool joinWifi(const Wifi& w, bool checking = false) {
  WiFi.mode(WIFI_STA);
  // Arduino-ESP32 3.2.1: begin() says WL_CONNECT_FAILED when the station would not start or take the network; after it,
  // the disconnect reason sets WL_NO_SSID_AVAIL (no such network heard) or WL_CONNECT_FAILED (a wrong password, after
  // the one retry of the first connect since boot). Each is final: no need to wait out WIFI_MS. Mode OFF -> STA resets
  // the status (STA_STOP, STA_START), so neither is left over from the last attempt.
  if (WiFi.begin(w.ssid, w.pass[0] ? w.pass : nullptr) == WL_CONNECT_FAILED) {
    Serial.printf("[net] could not start joining %s\n", w.ssid);
    return false;
  }
  for (uint32_t t0 = millis(); WiFi.status() != WL_CONNECTED; vTaskDelay(pdMS_TO_TICKS(250))) {
    if (checking && checkStopped()) return false;
    const wl_status_t s = WiFi.status();
    const bool gaveUp = s == WL_NO_SSID_AVAIL || s == WL_CONNECT_FAILED;
    if (!gaveUp && millis() - t0 <= WIFI_MS) continue;
    Serial.printf("[net] could not join %s%s\n", w.ssid, s == WL_NO_SSID_AVAIL ? ": not found" : s == WL_CONNECT_FAILED ? ": refused" : "");
    return false;
  }
  Serial.printf("[net] joined %s rssi=%d heap_internal=%lu largest_internal=%lu\n", w.ssid, (int)WiFi.RSSI(),
                (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  return true;
}

void clockState(shell::TimeState state) {
  if (state == shell::TIME_FAILED || state == shell::TIME_NO_WIFI) g_nextClock = millis() + CLOCK_RETRY_MS;
  portENTER_CRITICAL(&g_mux); g_clock.state = state; portEXIT_CRITICAL(&g_mux);
}

// A fresh request on every connection. The random token distinguishes this request even when the RTC is wrong.
// DNS and UDP run only on the worker: neither a missing network nor an unavailable time service stalls the UI.
bool fetchClock(uint32_t& utc) {
  IPAddress server;
  if (WiFi.hostByName("pool.ntp.org", server) != 1) return false;
  WiFiUDP udp;
  if (!udp.begin(0)) return false;
  uint8_t packet[48] = {}, token[8];
  packet[0] = 0x23;   // version 4, client
  esp_fill_random(token, sizeof token);
  memcpy(packet + 40, token, sizeof token);
  if (!udp.beginPacket(server, 123) || udp.write(packet, sizeof packet) != sizeof packet || !udp.endPacket()) return false;
  for (uint32_t start = millis(); millis() - start < CLOCK_MS; vTaskDelay(pdMS_TO_TICKS(20))) {
    const int n = udp.parsePacket();
    if (!n) continue;
    const bool source = udp.remoteIP() == server && udp.remotePort() == 123;
    const int got = udp.read(packet, sizeof packet);
    if (source && got >= 0 && netclock::reply(packet, (size_t)got, token, utc)) return true;
  }
  return false;
}

void syncConnected() {
  clockState(shell::TIME_SYNCING);
  uint32_t utc = 0;
  if (!fetchClock(utc)) { clockState(shell::TIME_FAILED); return; }
  const uint32_t ms = millis();
  // POSIX time is UTC for TLS certificate verification. The shell and the physical RTC keep local wall time.
  const timeval tv = {(time_t)utc, 0};
  settimeofday(&tv, nullptr);
  portENTER_CRITICAL(&g_mux);
  g_clockUtc = utc; g_clockAt = ms; g_clockPending = true;
  g_clock.state = shell::TIME_OK;
  g_clock.lastSync = utc;   // raw UTC internally; main-core clockStatus derives local display time
  portEXIT_CRITICAL(&g_mux);
  ++g_clockSequence;   // UTC success handoff only; timezone retry decisions stay on the main core
  g_nextClock = ms + CLOCK_POLL_MS;
  Serial.printf("[net] clock synchronized utc=%lu\n", (unsigned long)utc);
}

void clockJob() {
  Wifi w;
  if (!loadWifi(w) || !joinWifi(w)) clockState(shell::TIME_NO_WIFI);
  else syncConnected();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

// True when this very release was already installed here and the bootloader went back to the running firmware.
bool alreadyRolledBack(const char* avail) {
  char tried[16];
  loadStr("tried", tried, sizeof tried);
  return shell::rolledBack(avail, tried, esp_ota_get_last_invalid_partition() != nullptr);
}

void checkAndDownload(shell::Channel ch) {
  char avail[16] = {0};
  Found f = fetchAvailable(ch, avail);
  if (checkStopped()) return;
  if (f == F_ERROR) { setState(shell::UP_FAILED); return; }
  portENTER_CRITICAL(&g_mux); strcpy(g_st.available, avail); portEXIT_CRITICAL(&g_mux);
  Serial.printf("[net] %s channel: %s, running %s\n", ch == shell::CH_BETA ? "beta" : "stable", f == F_VERSION ? avail : "none",
                PORTHOLE_VERSION);
  if (!shell::newer(avail, PORTHOLE_VERSION)) { setState(shell::UP_CURRENT); return; }
  if (alreadyRolledBack(avail)) {   // not again every day: only another release is tried
    Serial.printf("[net] %s started here and rolled back: waiting for another release\n", avail);
    setState(shell::UP_FAILED);
    return;
  }
  if (checkStopped()) return;
  bool ok = download(avail);
  // After the slot is set to boot, never before: a download that failed was not tried. A write that does not land
  // only loses the protection for this release.
  if (ok) board::saveBlob(NS, "tried", avail, strlen(avail));
  if (ok) setState(shell::UP_READY, 100); else setState(shell::UP_FAILED);   // a ready image is all there
  if (ok) Serial.printf("[net] %s ready: it starts on the next power-on\n", avail);
}

void run(shell::Channel ch) {
  g_woke = false;
  if (checkStopped()) { setState(shell::UP_IDLE); return; }
  Wifi w;
  if (!loadWifi(w)) { setState(shell::UP_NO_WIFI); return; }
  uint32_t now = g_checkNow, prev = 0;
  board::loadBlob(NS, "last", &prev, sizeof prev);
  board::saveBlob(NS, "last", &now, sizeof now);   // before connecting: a crash below can't retry on every boot
  portENTER_CRITICAL(&g_mux); g_st.lastCheck = now; portEXIT_CRITICAL(&g_mux);
  setState(shell::UP_CHECKING);
  if (joinWifi(w, true)) {
    if (!checkStopped()) syncConnected();
    if (!checkStopped()) checkAndDownload(ch);
  } else if (!g_woke) setState(shell::UP_NO_WIFI);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  if (!g_woke) return;
  // Not today's attempt after all: put the last check back so the next dark spell tries again.
  board::saveBlob(NS, "last", &prev, sizeof prev);
  portENTER_CRITICAL(&g_mux); g_st.lastCheck = prev; portEXIT_CRITICAL(&g_mux);
  setState(shell::UP_IDLE);
  Serial.println("[net] check cancelled or screen woke: download stopped");
}

// Wifi setup from the system menu, each a task like the update check. A scan lists what the radio hears; a test joins
// a network the grown-up picked (not saved yet) and reports whether it could. Neither touches g_st.
void scan() {
  int16_t n = WiFi.scanNetworks();   // blocking, here on core 0: every channel, a few seconds; it turns the station on
  int count = 0;
  for (int i = 0; i < n; i++)
    count = shell::addNetwork(g_found, count, WiFi.SSID(i).c_str(), (int)WiFi.RSSI(i),
                              WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  WiFi.scanDelete();
  WiFi.mode(WIFI_OFF);
  Serial.printf("[net] scan: %d heard, %d listed\n", (int)n, count);
  g_scanCount = count;   // last: it hands g_found to the main loop
}
void test() {
  const bool ok = joinWifi(g_try);   // logs the network, never the password
  if (ok) syncConnected();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  memset(g_try.pass, 0, sizeof g_try.pass);
  g_tested = ok ? 1 : 0;
}

// The only request is for the public-IP timezone, with no location/SSID/profile fields in its URL or body.
// ipapi.co/timezone/ returns one plain IANA identifier. No live development probe is required or performed.
bool fetchZone(char (&out)[64]) {
  Https c; c.http10 = true;
  if (c.get("https://ipapi.co/timezone/") != 200) return false;
  const int length = c.http.getSize();
  if (length <= 0 || length >= 64) return false;
  char body[64] = {};
  if (c.http.getStream().readBytes(body, (size_t)length) != (size_t)length) return false;
  return timezones::detectedZone(body, (size_t)length, out);
}
void zoneJob() {
  Wifi w; char zone[64] = {};
  const bool joined = loadWifi(w) && joinWifi(w);
  if (joined) syncConnected();   // establish UTC before the first HTTPS certificate validation after boot
  const bool ok = joined && fetchZone(zone);
  wifiOff();
  portENTER_CRITICAL(&g_mux);
  memcpy(g_zoneResult, zone, sizeof zone); g_zoneResultOk = ok;
  g_zoneResultToken = g_zoneJobToken; g_zonePending = true;
  portEXIT_CRITICAL(&g_mux);
}

void lookup();
void radar();
void lanJob();
void telemJob();
bool waitForProgress() {
  for (uint32_t start = millis(); !g_screen.ready(); vTaskDelay(1)) {
    if (g_screen.cancelled()) { setState(shell::UP_IDLE); return false; }
    if (!g_screen.failed() && millis() - start < 2000) continue;
    Serial.println("[net] progress display unavailable: update deferred");
    setState(shell::UP_FAILED);
    return false;
  }
  return true;
}

void task(void* arg) {
  const Job job = (Job)(uintptr_t)arg;
  if (job == J_SCAN) scan();
  else if (job == J_TEST) test();
  else if (job == J_LOOK) lookup();
  else if (job == J_RADAR) radar();
  else if (job == J_LAN) lanJob();
  else if (job == J_CLOCK) clockJob();
  else if (job == J_ZONE) zoneJob();
  else if (job == J_TELEM) telemJob();
  else if (waitForProgress()) run(g_checkChan);
  g_screen.release();
  g_forced = false;
  g_busy = false;
  vTaskDelete(nullptr);
}

// Starts a job on core 0. The caller holds g_busy (the radio); without memory for the task it lets go and says so.
// Idle priority: it time-slices with IDLE0, so a long stretch without a yield (a TLS handshake, Update.end()'s image
// check) cannot starve IDLE0 into the 5 s task watchdog. Wifi and lwIP run in their own, higher-priority tasks.
bool spawn(Job job) {
  if (xTaskCreatePinnedToCore(task, "porthole-net", STACK, (void*)(uintptr_t)job, tskIDLE_PRIORITY, nullptr, 0) == pdPASS) return true;
  g_screen.release();
  if (job == J_CHECK) setState(shell::UP_FAILED);
  g_forced = false;
  g_busy = false;
  Serial.println("[net] no memory for the wifi task");
  return false;
}

// Home from a street address: one GET to OpenStreetMap's Nominatim (its policy: at most a request a second, a
// User-Agent naming the app; a grown-up's single tap is far under). The address is held only until the lookup ends
// and is never logged; what comes back is the place's coordinates and name.
char g_addr[128];          // the address, the lookup task's while it runs; wiped after
shell::Home g_looked;      // what it found; the main loop's once g_lookState is 1
std::atomic<int> g_lookState{0};

void urlEncode(const char* s, String& out) {   // RFC 3986: only the unreserved characters stay as they are
  static const char DIGITS[] = "0123456789ABCDEF";
  for (; *s; s++) {
    const unsigned char c = (unsigned char)*s;
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') { out += (char)c; continue; }
    out += '%'; out += DIGITS[c >> 4]; out += DIGITS[c & 15];
  }
}

// The geocoder's UTF-8 name as printable ASCII, cut to max-1: Latin-1 letters (U+00C0..U+00FF, lead byte 0xC3) lose
// their accents, any other non-ASCII character becomes '?', control bytes go.
void foldAscii(const char* s, char* out, size_t max) {
  static const char LATIN1[] = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTsaaaaaaaceeeeiiiidnooooo/ouuuuyty";
  size_t n = 0;
  for (const unsigned char* p = (const unsigned char*)s; *p && n + 1 < max; p++) {
    if (*p >= 0x20 && *p < 0x7F) out[n++] = (char)*p;
    else if (*p == 0xC3 && (p[1] & 0xC0) == 0x80) out[n++] = LATIN1[*++p & 0x3F];
    else if (*p >= 0xC0) out[n++] = '?';   // the lead byte of another character; its continuation bytes are dropped
  }
  out[n] = 0;
}

// Only the town, the region and the country: the street address is never kept, not even as the place's name.
void placeName(JsonObjectConst a, char* out, size_t max) {
  const char* town = a["city"] | a["town"] | a["village"] | a["hamlet"] | a["suburb"] | "";
  char utf8[160];
  snprintf(utf8, sizeof utf8, "%s%s%s%s%s", town, *town ? ", " : "", a["state"] | "", a["state"].is<const char*>() ? ", " : "",
           a["country"] | "");
  foldAscii(utf8, out, max);
}

bool geocode(const char* address, shell::Home& out) {
  String url = "https://nominatim.openstreetmap.org/search?format=jsonv2&limit=1&addressdetails=1&accept-language=en&q=";
  urlEncode(address, url);
  Https c;
  c.http10 = true;
  if (c.get(url.c_str()) != 200) return false;
  JsonDocument filter(&g_psram), doc(&g_psram);
  filter[0]["lat"] = filter[0]["lon"] = true;
  for (const char* k : {"city", "town", "village", "hamlet", "suburb", "state", "country"}) filter[0]["address"][k] = true;
  if (deserializeJson(doc, c.http.getStream(), DeserializationOption::Filter(filter))) return false;
  const char* lat = doc[0]["lat"].as<const char*>();   // strings in Nominatim's answer; [] when nothing matched
  const char* lon = doc[0]["lon"].as<const char*>();
  if (!lat || !lon) return false;
  out.lat = strtod(lat, nullptr);
  out.lon = strtod(lon, nullptr);
  placeName(doc[0]["address"], out.name, sizeof out.name);
  return validSpot(out.lat, out.lon);
}
void lookup() {
  Wifi w;
  const bool joined = loadWifi(w) && joinWifi(w);
  if (joined) syncConnected();
  const bool ok = joined && geocode(g_addr, g_looked);
  wifiOff();
  memset(g_addr, 0, sizeof g_addr);
  Serial.printf("[net] home lookup: %s\n", ok ? "found" : "nothing");
  g_lookState = ok ? 1 : 0;   // last: it hands g_looked to the main loop
}

// ---------------------------------------------------------------------------------------------------- Radar's feed
// While Radar is open (g_want) and the screen is lit, a radar task holds the radio: joins the saved wifi once, then
// every RADAR_MS asks adsb.fi for the planes around home, rounded to 2 decimals (~1 km: the only thing about the
// household that leaves the device), and adsbdb.com for one new callsign's route. Joining with the screen lit can
// write flash (the wifi driver's PHY calibration, when it has none yet), like the menu's scan and test join do.
const char* const ADSB = "https://opendata.adsb.fi/api/v3/lat/%.2f/lon/%.2f/dist/%d";   // dist: nautical miles
const char* const ROUTES = "https://api.adsbdb.com/v0/callsign/";
const uint32_t RADAR_MS = 5000;      // a poll every 5 s (adsb.fi allows one a second)
const uint32_t MIN_HEAP = 45000;     // internal bytes a TLS handshake needs; below it a poll is skipped, not failed
const double KM_PER_DEG = 111.0, DEG = M_PI / 180;

std::atomic<bool> g_want{false};     // started and not stopped (main loop writes)
std::atomic<bool> g_airRun{false};   // a radar task exists
std::atomic<uint8_t> g_phase{air::S_JOINING};   // S_JOINING, S_LIVE, or the dead ends S_NO_WIFI / S_NO_SPOT
std::atomic<float> g_km{0};          // the radius start() asked for
std::atomic<uint32_t> g_goodMs{0};   // millis() of the last good fetch
double g_airLat = 0, g_airLon = 0;   // home, exact: set on the main loop before the task starts, then the task's
uint32_t g_airTry = 0;               // main loop: the last try to start the task
// The planes: the task fills g_snap[g_front ^ 1], then flips g_front under g_airMux; read() copies g_snap[g_front]
// under it. Only the task writes g_front.
portMUX_TYPE g_airMux = portMUX_INITIALIZER_UNLOCKED;
air::Snapshot g_snap[2];
int g_front = 0;

// The radar task's: this fetch's nearest planes (nearest first) with their positions, and the routes seen.
// Both tables live in PSRAM (psram.h): 3.6 KB only this task reads, off the static internal RAM an update needs.
struct Seen { air::Plane p; float d2; double lat, lon; };
struct SeenTable { Seen at[air::MAX_PLANES]; };
Seen (&g_seen)[air::MAX_PLANES] = psram::make<SeenTable>().at;
int g_nSeen = 0;
// ponytail: a 32-callsign round-robin cache; grow it if a busy hub's sky keeps evicting routes it still shows.
struct Route { char call[9]; char route[8]; };
struct RouteTable { Route at[32]; };
Route (&g_routes)[32] = psram::make<RouteTable>().at;
int g_routeNext = 0;

const Route* cachedRoute(const char* call) {
  for (const Route& r : g_routes)
    if (r.call[0] && !strcmp(r.call, call)) return &r;
  return nullptr;
}
void remember(const char* call, const char* route) {   // route "" = none known (a 404 or an unlikely one)
  Route& r = g_routes[g_routeNext];
  g_routeNext = (g_routeNext + 1) % (int)(sizeof g_routes / sizeof g_routes[0]);
  snprintf(r.call, sizeof r.call, "%s", call);
  snprintf(r.route, sizeof r.route, "%s", route);
}

double centralAngle(double la1, double lo1, double la2, double lo2) {
  const double c = sin(la1) * sin(la2) + cos(la1) * cos(la2) * cos(lo2 - lo1);
  return acos(c < -1 ? -1 : c > 1 ? 1 : c);
}
double bearing(double la1, double lo1, double la2, double lo2) {
  return atan2(sin(lo2 - lo1) * cos(la2), cos(la1) * sin(la2) - sin(la1) * cos(la2) * cos(lo2 - lo1));
}
// Airlines reuse flight numbers, and adsbdb can name another day's route: trusted only with the plane within 200 km of
// the origin-destination great circle and not past the destination. Airports without coordinates: trusted.
bool onRoute(const Seen& s, JsonObject o, JsonObject d) {
  if (!o["latitude"].is<double>() || !o["longitude"].is<double>() || !d["latitude"].is<double>() ||
      !d["longitude"].is<double>())
    return true;
  const double alat = o["latitude"].as<double>() * DEG, alon = o["longitude"].as<double>() * DEG;
  const double blat = d["latitude"].as<double>() * DEG, blon = d["longitude"].as<double>() * DEG;
  const double plat = s.lat * DEG, plon = s.lon * DEG, max = 200.0 / 6371.0;
  const double ap = centralAngle(alat, alon, plat, plon), ab = centralAngle(alat, alon, blat, blon);
  const double xt = asin(sin(ap) * sin(bearing(alat, alon, plat, plon) - bearing(alat, alon, blat, blon)));
  return fabs(xt) <= max && ap <= ab + max;
}

// One callsign's route from adsbdb: "SEA-SFO", cached; a 404 or any other failure is cached as none.
void lookupRoute(const Seen& s) {
  const char* call = s.p.call;
  for (const char* q = call; *q; q++)
    if (!isalnum((unsigned char)*q)) { remember(call, ""); return; }   // nothing odd goes into a URL
  Https c;
  c.http10 = true;
  const int code = c.get((String(ROUTES) + call).c_str());
  // ponytail: any failure counts as no route until the cache rolls over (a 404 for good): a struggling adsbdb is not
  // asked again every poll, at the cost of a route that stays missing for a while.
  if (code != 200) { remember(call, ""); return; }
  JsonDocument filter(&g_psram), doc(&g_psram);
  JsonObject a = filter["response"]["flightroute"]["origin"].to<JsonObject>();
  for (const char* k : {"iata_code", "latitude", "longitude"}) a[k] = true;
  filter["response"]["flightroute"]["destination"] = a;
  if (deserializeJson(doc, c.http.getStream(), DeserializationOption::Filter(filter))) { remember(call, ""); return; }
  JsonObject o = doc["response"]["flightroute"]["origin"], d = doc["response"]["flightroute"]["destination"];
  const char* oc = o["iata_code"] | "";
  const char* dc = d["iata_code"] | "";
  char route[8] = "";
  if (strlen(oc) == 3 && strlen(dc) == 3 && onRoute(s, o, d)) snprintf(route, sizeof route, "%.3s-%.3s", oc, dc);
  remember(call, route);
}

// The rest of a plane: callsign (space-padded in adsb.fi's answer), altitude and kind.
void describe(JsonObject a, air::Plane& p) {
  const char* fl = a["flight"] | "";
  while (*fl == ' ') fl++;
  size_t n = strnlen(fl, sizeof p.call - 1);
  while (n && fl[n - 1] == ' ') n--;
  memcpy(p.call, fl, n);
  p.call[n] = 0;
  JsonVariant baro = a["alt_baro"], geom = a["alt_geom"];
  auto ft = [](double v) { return v > -2000 && v < 100000 ? (int32_t)lround(v) : air::NO_ALT; };   // NaN/inf: none
  p.altFt = baro.is<double>() ? ft(baro.as<double>()) : geom.is<double>() ? ft(geom.as<double>()) : air::NO_ALT;
  const char* cat = a["category"] | "";
  p.kind = ((a["dbFlags"] | 0) & 1) ? air::K_MILITARY
           : (!strcmp(cat, "A1") || !strcmp(cat, "A2")) ? air::K_SMALL : air::K_AIRLINER;
}
// One adsb.fi aircraft as a plane around home; false if it has no position or is on the ground.
bool toPlane(JsonObject a, Seen& s) {
  if (!a["lat"].is<double>() || !a["lon"].is<double>()) return false;
  JsonVariant baro = a["alt_baro"];
  if (baro.is<const char*>() && !strcmp(baro.as<const char*>(), "ground")) return false;
  s = Seen{};
  s.lat = a["lat"].as<double>();
  s.lon = a["lon"].as<double>();
  if (!(fabs(s.lat) <= 90 && fabs(s.lon) <= 180)) return false;   // network data: also refuses NaN and infinity
  double dlon = s.lon - g_airLon;
  if (dlon > 180) dlon -= 360; else if (dlon < -180) dlon += 360;   // across the antimeridian
  s.p.eastKm = (float)(dlon * KM_PER_DEG * cos(g_airLat * DEG));   // flat earth: fine for a few hundred km
  s.p.northKm = (float)((s.lat - g_airLat) * KM_PER_DEG);
  s.d2 = s.p.eastKm * s.p.eastKm + s.p.northKm * s.p.northKm;
  s.p.trackDeg = a["track"].is<float>() ? a["track"].as<float>() : a["true_heading"] | 0.0f;
  const float gs = a["gs"] | 0.0f;
  s.p.knots = (uint16_t)(gs < 0 ? 0 : gs > 65535 ? 65535 : gs);
  describe(a, s.p);
  return true;
}
void keepNearest(const Seen& s) {   // into g_seen, nearest first; when full the farthest falls off
  int at = g_nSeen;
  while (at > 0 && g_seen[at - 1].d2 > s.d2) at--;
  if (at >= air::MAX_PLANES) return;
  const int n = g_nSeen < air::MAX_PLANES ? g_nSeen : air::MAX_PLANES - 1;
  memmove(g_seen + at + 1, g_seen + at, (size_t)(n - at) * sizeof *g_seen);
  g_seen[at] = s;
  g_nSeen = n + 1;
}

bool fetchPlanes() {
  char url[112];
  const float nm = g_km.load() / 1.852f;
  snprintf(url, sizeof url, ADSB, g_airLat, g_airLon, (int)ceilf(nm < 1 ? 1 : nm > 250 ? 250 : nm));
  Https c;
  c.http10 = true;
  if (c.get(url) != 200) return false;
  JsonDocument filter(&g_psram), doc(&g_psram);
  JsonObject f = filter["ac"].add<JsonObject>();
  for (const char* k : {"lat", "lon", "track", "true_heading", "gs", "alt_baro", "alt_geom", "flight", "category", "dbFlags"})
    f[k] = true;
  const DeserializationError e = deserializeJson(doc, c.http.getStream(), DeserializationOption::Filter(filter));
  if (e || !doc["ac"].is<JsonArray>()) { Serial.printf("[radar] bad answer: %s\n", e.c_str()); return false; }
  g_nSeen = 0;
  Seen s;
  for (JsonObject a : doc["ac"].as<JsonArray>())
    if (toPlane(a, s)) keepNearest(s);
  return true;
}

void publish() {   // g_seen, with the routes known so far, becomes what read() hands out
  static uint32_t gen = 0;
  air::Snapshot& s = g_snap[g_front ^ 1];
  s.n = g_nSeen;
  for (int i = 0; i < g_nSeen; i++) {
    s.planes[i] = g_seen[i].p;
    const Route* r = s.planes[i].call[0] ? cachedRoute(s.planes[i].call) : nullptr;
    memcpy(s.planes[i].route, r ? r->route : "", r ? sizeof r->route : 1);
  }
  s.gen = ++gen;
  portENTER_CRITICAL(&g_airMux); g_front ^= 1; portEXIT_CRITICAL(&g_airMux);
  g_goodMs = millis();
  g_phase = air::S_LIVE;
}

// A TLS handshake needs one big block, not just enough free bytes in all: hours of polling can fragment the heap.
bool heapOk() {
  if (heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) >= MIN_HEAP) return true;
  Serial.println("[radar] heap low: request skipped");
  return false;
}

void poll() {
  if ((int32_t)(millis() - g_nextClock) >= 0) syncConnected();   // the long-running feed already holds the radio
  if (!heapOk() || !fetchPlanes()) return;
  publish();
  if (!g_want || g_dark || !heapOk()) return;   // let the radio go without waiting on a route lookup
  for (int i = 0; i < g_nSeen; i++)   // at most one route lookup a poll, the nearest plane without one; shown next poll
    if (g_seen[i].p.call[0] && !cachedRoute(g_seen[i].p.call)) { lookupRoute(g_seen[i]); break; }
}

void radar() {
  Wifi w;
  if (!loadWifi(w) || !joinWifi(w)) g_phase = air::S_NO_WIFI;   // a dead end until Radar starts the feed again
  else {
    syncConnected();
    // ponytail: stop() can't cut a GET short; the radio comes back after the one in flight (up to its timeouts), and a
    // menu scan started in that window is refused (the menu waits and retries).
    for (uint32_t next = millis(); g_want && !g_dark; vTaskDelay(pdMS_TO_TICKS(100))) {
      if ((int32_t)(millis() - next) < 0) continue;
      next = millis() + RADAR_MS;
      poll();
    }
    if (g_want) g_phase = air::S_JOINING;   // the dark sent it away: it joins again when the screen wakes
  }
  wifiOff();
  Serial.printf("[radar] radio released, stack to spare %u\n", (unsigned)uxTaskGetStackHighWaterMark(nullptr));
  g_airRun = false;
}

// Main loop: starts the radar task when the feed is wanted, the screen lit and the radio free. Covers a radio busy at
// start (an update or a wifi job) and the screen waking after the dark sent the task away. At most a try a second.
// ---------------------------------------------------------------------------------------------------- Rally's network
// The datagrams: what came in (a small ring, the oldest dropped when full) and the one going out (the newest wins:
// each is a whole state). Both under g_lanMux; only the task touches the socket.
const uint16_t LAN_PORT = 47800;
const uint32_t LAN_AWAKE_MS = 2000;   // power save back on this long after the last datagram out
constexpr int LAN_RING = 6;
struct LanPkt { uint32_t addr; uint16_t len; uint8_t data[lan::MAX_PACKET]; };
std::atomic<bool> g_lanWant{false}, g_lanRun{false};
std::atomic<uint8_t> g_lanState{lan::S_OFF};
std::atomic<uint32_t> g_lanSelf{0};
uint32_t g_lanTry = 0;
portMUX_TYPE g_lanMux = portMUX_INITIALIZER_UNLOCKED;
struct LanBufs { LanPkt in[LAN_RING], out; };
LanBufs& g_lanBuf = psram::make<LanBufs>();   // ~1.2 KB kept off static internal RAM
LanPkt* const g_lanIn = g_lanBuf.in;
LanPkt& g_lanOut = g_lanBuf.out;
int g_lanHead = 0, g_lanCount = 0;
bool g_lanOutFull = false;

uint32_t ipWord(const IPAddress& ip) { return (uint32_t)ip[0] | (uint32_t)ip[1] << 8 | (uint32_t)ip[2] << 16 | (uint32_t)ip[3] << 24; }
IPAddress ipOf(uint32_t v) { return IPAddress(v & 255, v >> 8 & 255, v >> 16 & 255, v >> 24); }

void lanReceive(WiFiUDP& udp) {
  for (int n = 0; n < LAN_RING; n++) {
    const int len = udp.parsePacket();
    if (len <= 0) return;
    LanPkt p;
    p.len = (uint16_t)udp.read(p.data, sizeof p.data);
    p.addr = ipWord(udp.remoteIP());
    if (len > (int)sizeof p.data) continue;   // not ours: too big
    portENTER_CRITICAL(&g_lanMux);
    g_lanIn[(g_lanHead + g_lanCount) % LAN_RING] = p;
    if (g_lanCount < LAN_RING) g_lanCount++;
    else g_lanHead = (g_lanHead + 1) % LAN_RING;
    portEXIT_CRITICAL(&g_lanMux);
  }
}
bool lanSend(WiFiUDP& udp) {
  LanPkt p;
  portENTER_CRITICAL(&g_lanMux);
  const bool full = g_lanOutFull;
  if (full) { p = g_lanOut; g_lanOutFull = false; }
  portEXIT_CRITICAL(&g_lanMux);
  if (!full) return false;
  udp.beginPacket(p.addr ? ipOf(p.addr) : WiFi.broadcastIP(), LAN_PORT);
  udp.write(p.data, p.len);
  udp.endPacket();
  return true;
}
void lanPump(WiFiUDP& udp) {
  uint32_t sentMs = 0;
  bool asleep = true;
  for (; g_lanWant && !g_dark; vTaskDelay(pdMS_TO_TICKS(5))) {
    lanReceive(udp);
    if (lanSend(udp)) sentMs = millis();
    const bool idle = millis() - sentMs > LAN_AWAKE_MS;
    // The clock sync can block for seconds (no internet, NTP blocked): only while no race is sending.
    if (idle && (int32_t)(millis() - g_nextClock) >= 0) syncConnected();
    if (idle != asleep) { WiFi.setSleep(idle); asleep = idle; }   // awake while racing: no 100 ms beacon waits
  }
}
void lanJob() {
  Wifi w;
  g_lanState = lan::S_JOINING;
  if (!loadWifi(w) || !joinWifi(w)) g_lanState = lan::S_NO_WIFI;   // a dead end until Rally starts again
  else {
    WiFiUDP udp;
    udp.begin(LAN_PORT);
    g_lanSelf = ipWord(WiFi.localIP());
    g_lanState = lan::S_READY;
    lanPump(udp);
    udp.stop();
    g_lanSelf = 0;
    if (g_lanWant) g_lanState = lan::S_JOINING;   // the dark sent it away: it joins again when the screen wakes
  }
  wifiOff();
  Serial.printf("[lan] radio released, stack to spare %u\n", (unsigned)uxTaskGetStackHighWaterMark(nullptr));
  g_lanRun = false;
}
// Main loop: like pokeRadar, starts the task when Rally wants it, the screen is lit and the radio is free.
void pokeLan() {
  if (!g_lanWant || g_lanRun || g_dark || g_lanState == lan::S_NO_WIFI) return;
  const uint32_t ms = millis();
  if (ms - g_lanTry < 1000) return;
  g_lanTry = ms;
  if (g_busy.exchange(true)) return;
  g_lanRun = true;
  if (!spawn(J_LAN)) g_lanRun = false;
}

void pokeRadar() {
  const uint8_t ph = g_phase;
  if (!g_want || g_airRun || g_dark || ph == air::S_NO_WIFI || ph == air::S_NO_SPOT) return;
  const uint32_t ms = millis();
  if (ms - g_airTry < 1000) return;
  g_airTry = ms;
  if (g_busy.exchange(true)) return;
  g_airLat = g_home.lat;
  g_airLon = g_home.lon;
  g_airRun = true;
  if (!spawn(J_RADAR)) g_airRun = false;
}

bool startCheck(shell::Channel ch, bool forced) {   // false: dropped
  if (g_busy.exchange(true)) return false;   // a check, a scan or a test is running
  // Downloaded already: a second run would rewrite the slot that is set to boot, and a failure would leave it broken.
  portENTER_CRITICAL(&g_mux); bool ready = g_st.state == shell::UP_READY; portEXIT_CRITICAL(&g_mux);
  if (ready) { g_busy = false; return false; }
  g_forced = forced;
  g_checkNow = timezones::local(g_now());
  g_checkChan = ch;
  g_screen.request(); setState(shell::UP_CHECKING);
  return spawn(J_CHECK);
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
  if (invalidateDetectedZone() && storeWifi(ssid.c_str(), pass.c_str())) {
    Serial.printf("[net] wifi from SD card saved: %s\n", ssid.c_str());
  }
}

// Opt-in telemetry (os/telemetry.h). Off until the owner sets it up: "sys"/telem (telemetry::Config) from the SD card's
// /porthole/telemetry.txt or serial Y. Its device id and salt are random, made when it is turned on and wiped when it is
// turned off; its fileCrc is the card file last taken, so the menu's Off holds until the file changes. Events wait in
// PSRAM (lost at power-off) and go out as one radio job while the screen is dark.
const char* resetName(esp_reset_reason_t r);
const uint32_t TELEM_MS = 15 * 60000u, TELEM_MORE_MS = 5000;   // between sends; sooner while a send left some behind
const size_t TELEM_BODY = 32 * 1024;
telemetry::Queue& g_tq = psram::make<telemetry::Queue>();
char* g_tbody = nullptr;            // the request body: the main loop's until spawn, then the job's until g_tresult
size_t g_tlen = 0;
int g_ttaken = 0;
uint32_t g_tgen = 0, g_tsentGen = 0;   // a new setting since the send started: its result drops nothing
char g_turl[160], g_tauth[112];
std::atomic<int> g_tresult{0};      // the job's answer: 1 taken, 2 try again later, 3 refused for good (dropped)
uint32_t g_nextTelem = FIRST_MS;
bool g_toldBoot = false;

void loadTelemetry(telemetry::Config& c) {
  if (board::loadBlob(NS, "telem", &c, sizeof c) != sizeof c || c.version != 1 || c.level > telemetry::PERSONAL) c = telemetry::Config{};
  c.url[sizeof c.url - 1] = c.token[sizeof c.token - 1] = 0;
}
// The owner's settings (level, format, url, token) over what is stored, saved and read back before they take effect:
// a failed flash write must not report a change that did not land. Ids stay while it stays on.
bool storeTelemetry(const telemetry::Config& owner, uint32_t fileCrc) {
  telemetry::Config was, c = owner, back;
  loadTelemetry(was);
  c.fileCrc = fileCrc;
  if (!c.level) memset(c.device, 0, sizeof c.device + sizeof c.salt);
  else if (was.level) { memcpy(c.device, was.device, sizeof c.device); memcpy(c.salt, was.salt, sizeof c.salt); }
  else { esp_fill_random(c.device, sizeof c.device); esp_fill_random(c.salt, sizeof c.salt); }
  if (!board::saveBlob(NS, "telem", &c, sizeof c)) return false;
  loadTelemetry(back);
  if (memcmp(&back, &c, sizeof c)) return false;
  g_tq.configure(c); g_tgen++;
  return true;
}
void telemetryFromSd() {
  File f = SD_MMC.open("/porthole/telemetry.txt");
  if (!f) return;
  char text[512] = {0};
  const size_t n = f.read((uint8_t*)text, sizeof text - 1);
  f.close();
  const uint32_t crc = os::crc32(text, n);
  if (g_tq.config().fileCrc == crc) return;   // taken before: a grown-up's Off in the menu stays off
  telemetry::Config c;
  if (!telemetry::parseConfig(text, c)) { Serial.println("[net] telemetry.txt not understood: telemetry unchanged"); return; }
  if (!storeTelemetry(c, crc)) return;
  char h[40];
  Serial.printf("[net] telemetry from SD card: %s\n", c.level ? telemetry::host(c, h, sizeof h) : "off");
}
// One POST of the queue's body. The token never reaches the log: only the host and the status do.
int telemPost() {
  NetworkClientSecure tls; HTTPClient http;
  tls.setCACertBundle(CA_BUNDLE, (size_t)(CA_BUNDLE_END - CA_BUNDLE));
  tls.setHandshakeTimeout(TLS_S);
  if (!http.begin(tls, g_turl)) return -1;
  http.setConnectTimeout(HTTP_MS); http.setTimeout(HTTP_MS);
  http.setUserAgent("porthole/" PORTHOLE_VERSION);
  http.addHeader("Content-Type", "application/json");
  if (g_tauth[0]) http.addHeader("Authorization", g_tauth);
  const int code = http.POST((uint8_t*)g_tbody, g_tlen);
  http.end();
  return code;
}
void telemJob() {
  Wifi w; int code = -1;
  if (loadWifi(w) && joinWifi(w)) { syncConnected(); code = telemPost(); }
  wifiOff();
  if (code < 200 || code > 299) Serial.printf("[net] telemetry send -> %d\n", code);
  // 400, 413, 422: this body will never be taken; anything else (no wifi, a 5xx, a bad key) is tried again later.
  g_tresult = code >= 200 && code <= 299 ? 1 : code == 400 || code == 413 || code == 422 ? 3 : 2;
}
// The first tick: after the clock came from the RTC, so the boot has a real time.
void tellBoot() {
  g_toldBoot = true;
  const esp_reset_reason_t r = esp_reset_reason();
  telemetry::emit("device_started", {telemetry::P("reset", resetName(r)), telemetry::P("channel", g_chan == shell::CH_BETA ? "beta" : "stable")});
  if (r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT || r == ESP_RST_BROWNOUT)
    telemetry::emit("crashed", {telemetry::P("reset", resetName(r))});
}
// A finished send: drop what was taken (unless the settings changed under it), and when to send next.
void takeTelemetryResult(uint32_t ms) {
  const int r = g_tresult.exchange(0);
  if (!r) return;
  if (r != 2 && g_tsentGen == g_tgen) g_tq.drop(g_ttaken);
  if (r == 1) g_tq.lastSent = g_now();
  g_nextTelem = ms + (r == 1 && g_tq.size() ? TELEM_MORE_MS : TELEM_MS);
}
bool telemetryDue(uint32_t ms) {
  return g_tbody && g_tq.on() && g_tq.size() && g_ssid[0] && g_dark && (int32_t)(ms - g_nextTelem) >= 0;
}
// Main loop: a finished send's result, then the next send when one is due, the radio is free and nobody is looking.
void telemetryTick(uint32_t ms) {
  if (!g_toldBoot) tellBoot();
  takeTelemetryResult(ms);
  if (!telemetryDue(ms) || g_busy.exchange(true)) return;
  g_nextTelem = ms + TELEM_MS;
  g_tlen = g_tq.batch(g_tbody, TELEM_BODY, &g_ttaken);
  if (!g_tlen) { g_tq.drop(1); g_busy = false; return; }   // an event too big for any body: never sendable
  g_tq.endpoint(g_turl, sizeof g_turl); g_tq.auth(g_tauth, sizeof g_tauth);
  g_tsentGen = g_tgen;
  spawn(J_TELEM);
}

const char* resetName(esp_reset_reason_t r) {
  static const char* const NAMES[] = {"unknown", "power-on", "external", "restart", "panic", "int-wdt", "task-wdt",
                                      "wdt", "deep-sleep", "brownout", "sdio", "usb", "jtag", "efuse", "power-glitch",
                                      "cpu-lockup"};
  return (unsigned)r < sizeof NAMES / sizeof NAMES[0] ? NAMES[r] : "unknown";
}
}  // namespace

void Updater::begin(uint32_t (*utcNow)()) {
  g_now = utcNow;
  timezones::Settings saved{};
  if (board::loadBlob(NS, "tz", &saved, sizeof saved) == sizeof saved && timezones::validSettings(saved)) {
    g_zoneSettings = saved; g_zoneNeeded = saved.automatic && !saved.reserved[0];
  }
  timezones::select(g_zoneSettings.zone);
  snprintf(g_clock.zone, sizeof g_clock.zone, "%s", g_zoneSettings.zone);
  g_clock.automaticZone = g_zoneSettings.automatic != 0;
  WiFi.persistent(false);   // the network lives in "sys"; keep the wifi driver from saving its own copy
  uint32_t last = 0;
  if (board::loadBlob(NS, "last", &last, sizeof last) == sizeof last) g_st.lastCheck = last;
  Wifi w;
  if (loadWifi(w)) strcpy(g_ssid, w.ssid);
  uint8_t c = 0;
  board::loadBlob(NS, "chan", &c, 1);
  g_chan = c == shell::CH_BETA ? shell::CH_BETA : shell::CH_STABLE;
  shell::Home h;
  if (board::loadBlob(NS, "home", &h, sizeof h) == sizeof h && validSpot(h.lat, h.lon)) {
    h.name[sizeof h.name - 1] = 0;
    g_home = h;
    g_homeSet = true;
  }
  g_sd = board::mountSd();
  Serial.printf("[net] version %s, SD card %s\n", PORTHOLE_VERSION, g_sd ? "mounted" : "not found");
  g_tq.begin(g_now, PORTHOLE_VERSION);
  g_tbody = (char*)heap_caps_malloc(TELEM_BODY, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  telemetry::Config tc;
  loadTelemetry(tc); g_tq.configure(tc);
  if (g_sd) { wifiFromSd(); telemetryFromSd(); SD_MMC.end(); }   // read once: the FAT and driver state go back to the heap
  telemetry::install(&g_tq);
}

void Updater::tick(uint32_t ms) {
  applyZoneResult();
  telemetryTick(ms);
  if (g_zoneRetry.afterSync(g_clockSequence.load(), ms, g_clock.automaticZone && g_clock.zoneState == shell::ZONE_FAILED))
    g_zoneNeeded = true;
  if ((int32_t)(ms - g_nextClock) >= 0 && !g_busy) {
    g_nextClock = ms + CLOCK_RETRY_MS;
    if (g_ssid[0]) syncClock();
  }
  if (g_zoneNeeded && g_clock.automaticZone && g_ssid[0] && !g_busy) detectTimeZone();
  if ((int32_t)(ms - g_nextPoll) < 0) return;
  g_nextPoll = ms + POLL_MS;
  if (!isRelease() || !g_ssid[0] || status().lastCheck / 86400u == timezones::local(g_now()) / 86400u) return;   // dev, no wifi, done today
  if (!g_dark) return;   // the daily check runs while nobody is looking: it may write flash
  startCheck(channel(), false);
}

// "No wifi" or "Check failed" was about the network before: idle again, as setChannel does. Only those resting states:
// never a run going (CHECKING, DOWNLOADING) or an update waiting (READY).
bool Updater::saveWifi(const char* ssid, const char* pass) {
  if (!invalidateDetectedZone() || !storeWifi(ssid, pass)) return false;
  g_nextClock = millis() + 1000;
  portENTER_CRITICAL(&g_mux);
  if (g_st.state == shell::UP_NO_WIFI || g_st.state == shell::UP_FAILED) { g_st.state = shell::UP_IDLE; g_st.pct = 0; }
  portEXIT_CRITICAL(&g_mux);
  return true;
}

shell::ClockStatus Updater::clockStatus() {
  portENTER_CRITICAL(&g_mux); shell::ClockStatus s = g_clock; portEXIT_CRITICAL(&g_mux);
  s.utcOffsetMin = timezones::offset(g_now());
  if (s.lastSync) s.lastSync = timezones::local(s.lastSync);
  return s;
}

bool Updater::syncClock() {
  if (g_busy.exchange(true)) return false;
  if (!g_ssid[0]) { clockState(shell::TIME_NO_WIFI); g_busy = false; return true; }
  clockState(shell::TIME_SYNCING);
  if (!spawn(J_CLOCK)) clockState(shell::TIME_FAILED);
  return true;
}

bool Updater::takeClock(uint32_t& utcEpoch) {
  portENTER_CRITICAL(&g_mux);
  const bool ready = g_clockPending;
  if (ready) {
    utcEpoch = netclock::advanced(g_clockUtc, g_clockAt, millis());
    g_clockPending = false;
  }
  portEXIT_CRITICAL(&g_mux);
  return ready;
}

bool Updater::setUtcOffset(int16_t) { return false; }   // legacy entry point; zones now own offsets and DST

bool Updater::setTimeZone(const char* iana, bool automatic) { return storeTimeZone(iana, automatic, false); }

bool Updater::storeTimeZone(const char* iana, bool automatic, bool detected) {
  const timezones::Zone* zone = timezones::find(iana);
  if (!zone) return false;
  const timezones::Settings wanted = timezones::settings(zone->id, automatic, detected);
  timezones::Settings checked{};
  if (!timezones::select(zone->id)) return false;
  if (!board::saveBlob(NS, "tz", &wanted, sizeof wanted) ||
      board::loadBlob(NS, "tz", &checked, sizeof checked) != sizeof checked || memcmp(&wanted, &checked, sizeof wanted)) {
    timezones::select(g_zoneSettings.zone); return false;
  }
  g_zoneSettings = wanted; ++g_zoneGeneration; g_zoneNeeded = automatic;
  portENTER_CRITICAL(&g_mux);
  snprintf(g_clock.zone, sizeof g_clock.zone, "%s", zone->id);
  g_clock.automaticZone = automatic;
  if (g_clock.zoneState != shell::ZONE_LOOKING) g_clock.zoneState = shell::ZONE_IDLE;
  portEXIT_CRITICAL(&g_mux);
  return true;
}

bool Updater::detectTimeZone() {
  if (!g_clock.automaticZone || g_busy.exchange(true)) return false;
  if (!invalidateDetectedZone()) {
    g_zoneNeeded = false;   // failed storage must wait for a later NTP event or explicit Detect, never retry each frame
    g_zoneRetry.completed(g_clockSequence.load());
    portENTER_CRITICAL(&g_mux); g_clock.zoneState = shell::ZONE_FAILED; portEXIT_CRITICAL(&g_mux);
    g_busy = false; return true;
  }
  g_zoneNeeded = false;
  if (!g_ssid[0]) {
    g_zoneRetry.completed(g_clockSequence.load());
    portENTER_CRITICAL(&g_mux); g_clock.zoneState = shell::ZONE_FAILED; portEXIT_CRITICAL(&g_mux);
    g_busy = false; return true;
  }
  g_zoneJobToken = g_zoneGeneration;
  portENTER_CRITICAL(&g_mux); g_clock.zoneState = shell::ZONE_LOOKING; portEXIT_CRITICAL(&g_mux);
  if (!spawn(J_ZONE)) {
    portENTER_CRITICAL(&g_mux); g_clock.zoneState = shell::ZONE_FAILED; portEXIT_CRITICAL(&g_mux);
  }
  return true;
}

void Updater::applyZoneResult() {
  char zone[64]; bool ok = false; uint32_t token = 0;
  portENTER_CRITICAL(&g_mux);
  const bool pending = g_zonePending;
  if (pending) { memcpy(zone, g_zoneResult, sizeof zone); ok = g_zoneResultOk; token = g_zoneResultToken; g_zonePending = false; }
  portEXIT_CRITICAL(&g_mux);
  if (!pending) return;
  if (token != g_zoneGeneration || !g_clock.automaticZone) {
    portENTER_CRITICAL(&g_mux);
    if (g_zoneJobToken != g_zoneGeneration || !g_busy) g_clock.zoneState = shell::ZONE_IDLE;
    portEXIT_CRITICAL(&g_mux); return;
  }
  const bool applied = ok && storeTimeZone(zone, true, true);
  g_zoneRetry.completed(g_clockSequence.load());   // a lookup cannot retry itself using its own earlier NTP sample
  g_zoneNeeded = false;   // success is cached; failures retry on later NTP connections with a six-hour cooldown
  portENTER_CRITICAL(&g_mux); g_clock.zoneState = applied ? shell::ZONE_OK : shell::ZONE_FAILED; portEXIT_CRITICAL(&g_mux);
}

shell::TelemetryStatus Updater::telemetryStatus() {
  shell::TelemetryStatus s;
  s.level = g_tq.config().level; s.queued = g_tq.size();
  if (s.level) { telemetry::host(g_tq.config(), s.host, sizeof s.host); g_tq.deviceId(s.device); }
  if (g_tq.lastSent) s.lastSent = timezones::local(g_tq.lastSent);
  return s;
}
bool Updater::setTelemetryLevel(uint8_t level) {
  return storeTelemetry(telemetry::withLevel(g_tq.config(), level), g_tq.config().fileCrc);
}
bool Updater::setTelemetry(const char* text) {
  telemetry::Config c;
  return telemetry::parseConfig(text, c) && storeTelemetry(c, g_tq.config().fileCrc);
}

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

void Updater::checkNow() { startCheck(channel(), true); }
bool Updater::tryCheckNow() { return startCheck(channel(), true); }

// Without memory for the task a scan finds nothing and a test fails: the menu says so instead of waiting on it.
bool Updater::startScan() {
  if (g_busy.exchange(true)) return false;
  g_scanCount = -1;
  if (!spawn(J_SCAN)) g_scanCount = 0;
  return true;
}
int Updater::scanResult(shell::Network* out, int max) {
  const int n = g_scanCount;
  if (n < 0) return -1;
  const int k = n < max ? n : max;
  for (int i = 0; i < k; i++) out[i] = g_found[i];
  return k;
}
bool Updater::startTest(const char* ssid, const char* pass) {
  if (g_busy.exchange(true)) return false;
  snprintf(g_try.ssid, sizeof g_try.ssid, "%s", ssid);
  snprintf(g_try.pass, sizeof g_try.pass, "%s", pass);
  g_tested = -1;
  if (!spawn(J_TEST)) { memset(g_try.pass, 0, sizeof g_try.pass); g_tested = 0; }
  return true;
}
int Updater::testResult() { return g_tested; }
void Updater::setDark(bool dark) {
  g_dark = dark;
  if (!dark && !g_forced) g_screen.cancel();   // daily wake and final commit race through the same atomic gate
}
bool Updater::progressRequested() { return g_screen.requested(); }
bool Updater::manualCheck() { return g_forced; }
void Updater::progressReady(bool ok) { g_screen.acknowledge(ok); }
void Updater::cancelCheck() { g_screen.cancel(); }
bool Updater::cancelling() { return g_screen.cancelled(); }
bool Updater::committing() { return g_screen.committing(); }
void Updater::restart() { ESP.restart(); }

shell::Channel Updater::channel() { return g_chan; }
// The last result was for the old channel: forget it, unless a run is going or an update is already waiting.
void Updater::setChannel(shell::Channel c) {
  g_chan = c;
  uint8_t b = c;
  board::saveBlob(NS, "chan", &b, 1);
  portENTER_CRITICAL(&g_mux);
  shell::UpdState s = g_st.state;
  if (!g_busy && s != shell::UP_CHECKING && s != shell::UP_DOWNLOADING && s != shell::UP_READY) {
    g_st.state = shell::UP_IDLE; g_st.pct = 0; g_st.available[0] = 0;
  }
  portEXIT_CRITICAL(&g_mux);
}

bool Updater::home(shell::Home& out) {
  out = g_home;
  return g_homeSet;
}
// A small NVS write: fine with the screen lit.
bool Updater::saveHome(const shell::Home& h) {
  if (!validSpot(h.lat, h.lon)) return false;
  shell::Home c = h;
  c.name[sizeof c.name - 1] = 0;
  if (!board::saveBlob(NS, "home", &c, sizeof c)) return false;
  g_home = c;
  g_homeSet = true;
  return true;
}
bool Updater::startLookup(const char* address) {
  if (g_busy.exchange(true)) return false;
  snprintf(g_addr, sizeof g_addr, "%s", address);
  g_lookState = -1;
  if (!spawn(J_LOOK)) { memset(g_addr, 0, sizeof g_addr); g_lookState = 0; }
  return true;
}
int Updater::lookupResult(shell::Home& out) {
  const int r = g_lookState;
  if (r == 1) out = g_looked;
  return r;
}

// A second start() only changes the radius. With no home it never touches the radio.
void Radar::start(float radiusKm) {
  g_km = radiusKm;
  if (g_want) {   // a range tap: also a retry after "No wifi" (the task already ended)
    if (g_phase == air::S_NO_WIFI) g_phase = air::S_JOINING;
    return;
  }
  g_phase = g_homeSet ? air::S_JOINING : air::S_NO_SPOT;
  g_want = true;
  pokeRadar();
}
void Radar::stop() { g_want = false; }   // the task sees it within 100 ms (or after a GET in flight) and lets go
void Radar::read(air::Snapshot& out) {
  pokeRadar();
  portENTER_CRITICAL(&g_airMux);
  out = g_snap[g_front];
  portEXIT_CRITICAL(&g_airMux);
  air::State s = g_want ? (air::State)g_phase.load() : air::S_OFF;
  if (s == air::S_LIVE && millis() - g_goodMs > air::STALE_MS) s = air::S_LOST;
  out.state = s;
  if (s != air::S_LIVE) out.n = 0;
}
void Lan::start() {
  if (!g_lanRun) g_lanState = lan::S_JOINING;   // a retry after "no wifi"; a task still running keeps its state
  portENTER_CRITICAL(&g_lanMux); g_lanCount = 0; g_lanOutFull = false; portEXIT_CRITICAL(&g_lanMux);
  g_lanWant = true;
  pokeLan();
}
void Lan::stop() { g_lanWant = false; }   // the task sees it within 5 ms and lets the radio go
lan::State Lan::state() {
  pokeLan();
  return g_lanWant ? (lan::State)g_lanState.load() : lan::S_OFF;
}
uint32_t Lan::self() { return g_lanSelf; }
void Lan::send(const void* data, size_t len, uint32_t to) {
  if (len > lan::MAX_PACKET || g_lanState != lan::S_READY) return;
  portENTER_CRITICAL(&g_lanMux);
  g_lanOut.addr = to; g_lanOut.len = (uint16_t)len;
  memcpy(g_lanOut.data, data, len);
  g_lanOutFull = true;
  portEXIT_CRITICAL(&g_lanMux);
}
size_t Lan::recv(void* buf, size_t cap, uint32_t* from) {
  LanPkt p;
  portENTER_CRITICAL(&g_lanMux);
  const bool any = g_lanCount > 0;
  if (any) { p = g_lanIn[g_lanHead]; g_lanHead = (g_lanHead + 1) % LAN_RING; g_lanCount--; }
  portEXIT_CRITICAL(&g_lanMux);
  if (!any || p.len > cap) return 0;   // a datagram bigger than the caller's is not one of its own: dropped
  memcpy(buf, p.data, p.len);
  *from = p.addr;
  return p.len;
}

bool Radar::spot(double* lat, double* lon) {
  *lat = g_home.lat;
  *lon = g_home.lon;
  return g_homeSet;
}

}  // namespace net
