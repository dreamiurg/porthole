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
#include <WiFi.h>
#include <algorithm>
#include <atomic>
#include <math.h>
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
// its day is the once-a-day gate), "home" (Radar's center: shell::Home's 112 bytes as they are). "ran" and "lite" are
// the shell's.
const char* const NS = "sys";
#define RELEASES "https://github.com/dreamiurg/porthole/releases/"
// Each channel's newest version as plain "X.Y.Z". Stable: GitHub's Latest release (pre-releases never are). Beta: the
// rolling porthole-beta pre-release, which the release workflow re-points at every new release. The releases API is no
// help for beta: it does not list releases newest first.
const char* const STABLE_VERSION = RELEASES "latest/download/porthole-version.txt";
const char* const BETA_VERSION = RELEASES "download/porthole-beta/porthole-version.txt";
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
std::atomic<bool> g_dark{false};   // the backlight is off (main loop, every frame)
std::atomic<bool> g_forced{false}; // the running check is a Check now: it writes flash even with the screen lit
std::atomic<bool> g_woke{false};   // the screen came on mid-download: stopped, to resume in the next dark spell
portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;
shell::UpdStatus g_st;             // written by the task, read by the main loop: only under g_mux
// The radio's jobs, one task at a time (g_busy): the update check (on g_checkChan, set before it starts), a scan or a
// test join from the menu's Wifi screens. g_scanCount and g_tested are -1 while theirs runs; g_found belongs to the scan
// task until it sets g_scanCount, and g_try (the network to test) to the test task while it runs. The Home lookup and
// Radar's feed (below) are the other two.
enum Job : uintptr_t { J_CHECK, J_SCAN, J_TEST, J_LOOK, J_RADAR };
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
    // Never the query: a lookup's is the typed address.
    if (code != 200) Serial.printf("[net] GET %s -> %d\n", at.substring(0, (unsigned)at.indexOf('?')).c_str(), code);
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
    if (!g_forced && !g_dark) { g_woke = true; return false; }   // someone woke the screen: no flash writes now
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
  if (ok) return true;
  if (!g_woke) Serial.printf("[net] download failed: %s\n", Update.errorString());   // a wake is not a failure
  if (Update.isRunning()) Update.abort();   // never begun, abort() would drive Update's unset LED pin: GPIO0
  return false;
}

bool joinWifi(const Wifi& w) {
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

void checkAndDownload(shell::Channel ch) {
  char avail[16] = {0};
  Found f = fetchAvailable(ch, avail);
  if (f == F_ERROR) { setState(shell::UP_FAILED); return; }
  portENTER_CRITICAL(&g_mux); strcpy(g_st.available, avail); portEXIT_CRITICAL(&g_mux);
  Serial.printf("[net] %s channel: %s, running %s\n", ch == shell::CH_BETA ? "beta" : "stable", f == F_VERSION ? avail : "none",
                PORTHOLE_VERSION);
  if (!shell::newer(avail, PORTHOLE_VERSION)) { setState(shell::UP_CURRENT); return; }
  if (!g_forced && !g_dark) { g_woke = true; return; }   // woken during the check: download in the next dark spell
  bool ok = download(avail);
  if (ok) setState(shell::UP_READY, 100); else setState(shell::UP_FAILED);   // a ready image is all there
  if (ok) Serial.printf("[net] %s ready: it starts on the next power-on\n", avail);
}

// TODO(porthole): sync the clock by NTP while connected. The RTC and the whole firmware keep local wall-clock seconds
// with no timezone anywhere on the device, so UTC from NTP cannot be turned into local time without a timezone setting.
void run(shell::Channel ch) {
  Wifi w;
  if (!loadWifi(w)) { setState(shell::UP_NO_WIFI); return; }
  uint32_t now = g_now(), prev = 0;
  board::loadBlob(NS, "last", &prev, sizeof prev);
  board::saveBlob(NS, "last", &now, sizeof now);   // before connecting: a crash below can't retry on every boot
  portENTER_CRITICAL(&g_mux); g_st.lastCheck = now; portEXIT_CRITICAL(&g_mux);
  setState(shell::UP_CHECKING);
  g_woke = false;
  if (joinWifi(w)) checkAndDownload(ch);
  else setState(shell::UP_NO_WIFI);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  if (!g_woke) return;
  // Not today's attempt after all: put the last check back so the next dark spell tries again.
  board::saveBlob(NS, "last", &prev, sizeof prev);
  portENTER_CRITICAL(&g_mux); g_st.lastCheck = prev; portEXIT_CRITICAL(&g_mux);
  setState(shell::UP_IDLE);
  Serial.println("[net] screen woke: download stopped, it resumes the next time the screen is dark");
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
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  memset(g_try.pass, 0, sizeof g_try.pass);
  g_tested = ok ? 1 : 0;
}

void lookup();
void radar();
void task(void* arg) {
  const Job job = (Job)(uintptr_t)arg;
  if (job == J_SCAN) scan();
  else if (job == J_TEST) test();
  else if (job == J_LOOK) lookup();
  else if (job == J_RADAR) radar();
  else run(g_checkChan);
  g_forced = false;
  g_busy = false;
  vTaskDelete(nullptr);
}

// Starts a job on core 0. The caller holds g_busy (the radio); without memory for the task it lets go and says so.
// Idle priority: it time-slices with IDLE0, so a long stretch without a yield (a TLS handshake, Update.end()'s image
// check) cannot starve IDLE0 into the 5 s task watchdog. Wifi and lwIP run in their own, higher-priority tasks.
bool spawn(Job job) {
  if (xTaskCreatePinnedToCore(task, "porthole-net", STACK, (void*)(uintptr_t)job, tskIDLE_PRIORITY, nullptr, 0) == pdPASS) return true;
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

bool geocode(const char* address, shell::Home& out) {
  String url = "https://nominatim.openstreetmap.org/search?format=jsonv2&limit=1&accept-language=en&q=";
  urlEncode(address, url);
  Https c;
  c.http10 = true;
  if (c.get(url.c_str()) != 200) return false;
  JsonDocument filter(&g_psram), doc(&g_psram);
  for (const char* k : {"lat", "lon", "display_name"}) filter[0][k] = true;
  if (deserializeJson(doc, c.http.getStream(), DeserializationOption::Filter(filter))) return false;
  const char* lat = doc[0]["lat"].as<const char*>();   // strings in Nominatim's answer; [] when nothing matched
  const char* lon = doc[0]["lon"].as<const char*>();
  if (!lat || !lon) return false;
  out.lat = strtod(lat, nullptr);
  out.lon = strtod(lon, nullptr);
  foldAscii(doc[0]["display_name"] | "", out.name, sizeof out.name);
  return validSpot(out.lat, out.lon);
}
void lookup() {
  Wifi w;
  const bool ok = loadWifi(w) && joinWifi(w) && geocode(g_addr, g_looked);
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
struct Seen { air::Plane p; float d2; double lat, lon; };
Seen g_seen[air::MAX_PLANES];
int g_nSeen = 0;
// ponytail: a 32-callsign round-robin cache; grow it if a busy hub's sky keeps evicting routes it still shows.
struct Route { char call[9]; char route[8]; };
Route g_routes[32];
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

// One callsign's route from adsbdb: "SEA-SFO", cached; a 404 is cached as none, any other failure tries again later.
void lookupRoute(const Seen& s) {
  const char* call = s.p.call;
  for (const char* q = call; *q; q++)
    if (!isalnum((unsigned char)*q)) { remember(call, ""); return; }   // nothing odd goes into a URL
  Https c;
  c.http10 = true;
  const int code = c.get((String(ROUTES) + call).c_str());
  if (code == 404) remember(call, "");
  if (code != 200) return;
  JsonDocument filter(&g_psram), doc(&g_psram);
  JsonObject a = filter["response"]["flightroute"]["origin"].to<JsonObject>();
  for (const char* k : {"iata_code", "latitude", "longitude"}) a[k] = true;
  filter["response"]["flightroute"]["destination"] = a;
  if (deserializeJson(doc, c.http.getStream(), DeserializationOption::Filter(filter))) return;
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
  p.altFt = baro.is<double>() ? (int32_t)lround(baro.as<double>())
            : geom.is<double>() ? (int32_t)lround(geom.as<double>()) : air::NO_ALT;
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

void poll() {
  if (heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < MIN_HEAP) { Serial.println("[radar] heap low: poll skipped"); return; }
  if (!fetchPlanes()) return;
  publish();
  for (int i = 0; i < g_nSeen; i++)   // at most one route lookup a poll, the nearest plane without one; shown next poll
    if (g_seen[i].p.call[0] && !cachedRoute(g_seen[i].p.call)) { lookupRoute(g_seen[i]); break; }
}

void radar() {
  Wifi w;
  if (!loadWifi(w) || !joinWifi(w)) g_phase = air::S_NO_WIFI;   // a dead end until Radar starts the feed again
  else {
    // ponytail: stop() can't cut a GET short; the radio comes back after it (up to HTTP_MS), and a menu scan started
    // in that window is refused (the menu waits and retries).
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
  g_checkChan = ch;
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
  shell::Home h;
  if (board::loadBlob(NS, "home", &h, sizeof h) == sizeof h && validSpot(h.lat, h.lon)) {
    h.name[sizeof h.name - 1] = 0;
    g_home = h;
    g_homeSet = true;
  }
  g_sd = board::mountSd();
  Serial.printf("[net] version %s, SD card %s\n", PORTHOLE_VERSION, g_sd ? "mounted" : "not found");
  if (g_sd) wifiFromSd();
}

void Updater::tick(uint32_t ms) {
  if ((int32_t)(ms - g_nextPoll) < 0) return;
  g_nextPoll = ms + POLL_MS;
  if (!isRelease() || !g_ssid[0] || status().lastCheck / 86400u == g_now() / 86400u) return;   // dev, no wifi, done today
  if (!g_dark) return;   // the daily check runs while nobody is looking: it may write flash
  startCheck(channel(), false);
}

// "No wifi" or "Check failed" was about the network before: idle again, as setChannel does. Only those resting states:
// never a run going (CHECKING, DOWNLOADING) or an update waiting (READY).
bool Updater::saveWifi(const char* ssid, const char* pass) {
  if (!storeWifi(ssid, pass)) return false;
  portENTER_CRITICAL(&g_mux);
  if (g_st.state == shell::UP_NO_WIFI || g_st.state == shell::UP_FAILED) { g_st.state = shell::UP_IDLE; g_st.pct = 0; }
  portEXIT_CRITICAL(&g_mux);
  return true;
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
void Updater::setDark(bool dark) { g_dark = dark; }
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
  if (g_want) return;
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
bool Radar::spot(double* lat, double* lon) {
  *lat = g_home.lat;
  *lon = g_home.lon;
  return g_homeSet;
}

}  // namespace net
