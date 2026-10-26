// Host simulator: the whole Porthole shell (profiles, launcher) with its games. With HAVE_SDL: a window (mouse =
// finger). Always: a scripted headless mode that writes BMP snapshots, used to eyeball screens without hardware.
// Also a --serve mode (stdin/stdout protocol) that tools/webemu.py drives to serve the device in a browser.
//   sim                      interactive (SDL)
//   sim --script file.txt    headless; commands: tap X Y | hold X Y | down X Y | move X Y | up | wait MS | skip SEC | snap name
//                            | reset | profile NAME AGE [PIN] | app NAME | newgame KID PET | debug | dbg CMD | ui | screen
//                            | echo WORD | watch MS | monkey N SEED | tilt X Y Z | shake | perf | reboot | sys WHAT [ARG]
//                            | feed WHAT [ARG]
//   sim --serve              headless; stdin commands: down X Y | move X Y | up | tick MS | skip SEC | reset | frame | dbg CMD
//                            | tilt X Y Z | shake | perf
// tilt sets the motion sensor's gravity (milli-g, Input::gx/gy/gz; upright (0 1000 0) until set), shake jolts it for a
// few frames, perf prints the average update and render time per frame since the last perf. reboot is a power cycle
// (saves kept). sys sets the stub wireless updates (SimSystem below): version V (the running firmware; the shell reads
// it at boot) | newest V (the channel's newest) | nowifi | current | download PCT | ready | failed, and for wifi setup
// scan (the running scan ends with canned networks) | scanempty (with none) | joinok | joinfail (the running test join
// ends so) | savefail (the next saveWifi fails) | timeok [UTCepoch] | timefail (complete a clock sync). `debug` adds its sys_ver/sys_chan/sys_state/sys_checks/sys_restarts/
// sys_last and sys_ssid/sys_pass/sys_scans/sys_nets/sys_tests/sys_tried/sys_tested. A reboot keeps lastCheck and the saved network (both in NVS).
// feed sets Plane Radar's stub feed (SimFeed below): live (canned planes, the default) | empty (live, none in range) |
// nospot | nowifi | joining | lost | off. Home comes from the stub system (`sys nohome|found|notfound|homefail`).
// `debug` adds feed_state/feed_on/feed_km/feed_starts and sys_home/sys_lat/sys_lon/sys_lookups/sys_addr.
#include <assert.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <string>
#include <vector>
#include "games/biscuit/game.h"
#include "games/living-kingdom/game.h"
#include "games/marble-kick/game.h"
#include "games/pumpkin-hide-seek/game.h"
#include "games/pets-club/game.h"
#include "games/plane-radar/game.h"
#include "games/starfighter/game.h"
#include "games/tilt-fc/game.h"
#include "games/toy-town-life/game.h"
#include "gfx565.h"
#include "launcher.h"
#include "shell.h"
#include "netclock.h"
#include "timezone.h"
#ifdef HAVE_SDL
#include <SDL.h>
#endif

// Stands in for the board's NVS (namespace, key) blobs as build/host/<ns>-<key>.sav.
struct FileStore : shell::Store {
  static const char* path(const char* ns, const char* key) { static char p[64]; snprintf(p, sizeof p, "build/host/%s-%s.sav", ns, key); return p; }
  size_t load(const char* ns, const char* key, void* buf, size_t max) override {
    FILE* f = fopen(path(ns, key), "rb"); if (!f) return 0;
    size_t got = fread(buf, 1, max, f); bool more = fgetc(f) != EOF; fclose(f);
    return more ? 0 : got;   // like NVS: a blob bigger than the buffer is not loaded at all
  }
  void save(const char* ns, const char* key, const void* data, size_t len) override {
    FILE* f = fopen(path(ns, key), "wb"); if (f) { fwrite(data, 1, len, f); fclose(f); }
  }
  void erase(const char* ns, const char* key) override { remove(path(ns, key)); }
};

// Canned wireless updates: the state is whatever a `sys` command last set. A local build, like the firmware's: "dev"
// never shows the "something new" card, so every scenario that does not ask for it starts as before.
static uint32_t now();
static uint32_t utcNow();
static uint32_t simulatedMs();
static void setUtcNow(uint32_t epoch);
struct SimSystem : shell::System {
  shell::ClockStatus clock; int syncs = 0;
  timezones::Settings zoneSettings = timezones::settings(timezones::DEFAULT_ZONE, true);
  uint32_t zoneGeneration = 0, zoneJobToken = 0, clockSequence = 0;
  timezones::DetectionRetry zoneRetryPolicy;
  bool zoneBusy = false, zoneRetry = false, unresolvedZoneCache = false, failZoneSave = false; int detections = 0;
  char version[16] = "dev"; shell::UpdStatus st; shell::Channel ch = shell::CH_STABLE;
  bool rebootDue = false; int slot = 0, checks = 0, restarts = 0;
  // Wifi setup: the saved network, the last scan (-1 until `sys scan` or `sys scanempty` ends it) and the last test
  // join (-1 until `sys joinok` or `sys joinfail`). `debug` prints the saved password: host only, never on a board.
  char ssid[33] = "Home-Network-5G-Upstairs", pass[65] = "", tried[33] = "";
  shell::Network nets[shell::MAX_NETWORKS]; int nNets = 0, tested = 0, scans = 0, tests = 0;
  bool failSave = false;   // `sys savefail`: the next saveWifi fails (NVS full or broken)
  // Home (the radar's center): set to Seattle so the radar is live; `sys nohome` clears it. A lookup runs until
  // `sys found` (a canned place) or `sys notfound` ends it; `sys homefail` makes the next saveHome fail.
  shell::Home homeAt = {47.62, -122.35, "Seattle, King County, Washington, United States"};
  bool homeSet = true, failHome = false; int looked = 0, lookups = 0; char addr[65] = "";
  bool hasHome() override { return true; }
  bool home(shell::Home& o) override { o = homeAt; return homeSet; }
  bool saveHome(const shell::Home& h) override {
    if (failHome) { failHome = false; return false; }
    homeAt = h; homeSet = true; return true;
  }
  bool startLookup(const char* a) override {
    if (!radioFree() || looked < 0) return false;
    lookups++; looked = -1; snprintf(addr, sizeof addr, "%s", a); return true;
  }
  int lookupResult(shell::Home& o) override {
    if (looked == 1) o = {47.6205, -122.3493, "Space Needle, 400 Broad Street, Seattle, Washington, United States"};
    return looked;
  }
  bool updating() const { return st.state == shell::UP_CHECKING || st.state == shell::UP_DOWNLOADING; }
  bool radioFree() const { return !updating() && nNets >= 0 && tested >= 0 && looked >= 0 && !zoneBusy && clock.state != shell::TIME_SYNCING; }
  void info(shell::SysInfo& o) override {
    o = {}; o.version = version; snprintf(o.slot, sizeof o.slot, "ota_%d", slot); snprintf(o.reset, sizeof o.reset, "power-on");
    o.heapFree = 187392; o.sd = true; o.wifiSaved = ssid[0] != 0; snprintf(o.ssid, sizeof o.ssid, "%s", ssid);
  }
  shell::UpdStatus status() override { return st; }
  void checkNow() override {
    if (!radioFree()) return;   // a scan or a test holds the radio: the contract says nothing happens
    checks++; st.state = ssid[0] ? shell::UP_CHECKING : shell::UP_NO_WIFI; st.lastCheck = now();
  }
  bool startScan() override { if (!radioFree()) return false; scans++; nNets = -1; return true; }
  int scanResult(shell::Network* out, int max) override {
    for (int i = 0; i < nNets && i < max; i++) out[i] = nets[i];
    return nNets < 0 ? -1 : nNets < max ? nNets : max;
  }
  bool startTest(const char* s, const char*) override {
    if (!radioFree()) return false;
    tests++; tested = -1; snprintf(tried, sizeof tried, "%s", s); return true;
  }
  int testResult() override { return tested; }
  bool saveWifi(const char* s, const char* p) override {
    if (failSave || !s[0] || strlen(s) > 32 || strlen(p) > 64) { failSave = false; return false; }
    if (clock.automaticZone && zoneSettings.reserved[0] && !storeZone(clock.zone, true, false)) return false;
    snprintf(ssid, sizeof ssid, "%s", s); snprintf(pass, sizeof pass, "%s", p);
    ++zoneGeneration;   // a pending canned response belongs to the old network
    if (zoneBusy && clock.automaticZone) zoneRetry = true;
    if (st.state == shell::UP_NO_WIFI || st.state == shell::UP_FAILED) st.state = shell::UP_IDLE;   // as the firmware's
    return true;
  }
  shell::ClockStatus clockStatus() override {
    auto result = clock; result.utcOffsetMin = timezones::offset(utcNow());
    if (result.lastSync) result.lastSync = timezones::local(result.lastSync);
    return result;
  }
  bool syncClock() override {
    if (!radioFree()) return false;
    syncs++; clock.state = ssid[0] ? shell::TIME_SYNCING : shell::TIME_NO_WIFI;
    return true;
  }
  bool setUtcOffset(int16_t) override { return false; }
  bool setTimeZone(const char* id, bool automatic) override { return storeZone(id, automatic, false); }
  bool storeZone(const char* id, bool automatic, bool detected) {
    const auto* zone = timezones::find(id);
    if (!zone || failZoneSave) { failZoneSave = false; return false; }
    const auto wanted = timezones::settings(zone->id, automatic, detected);
    FileStore store; timezones::Settings readback{};
    store.save("sys", "tz", &wanted, sizeof wanted);
    if (store.load("sys", "tz", &readback, sizeof readback) != sizeof readback || memcmp(&wanted, &readback, sizeof wanted)) return false;
    if (!timezones::select(zone->id)) return false;
    zoneSettings = wanted; ++zoneGeneration; unresolvedZoneCache = automatic && !detected;
    zoneRetry = zoneBusy && automatic;
    snprintf(clock.zone, sizeof clock.zone, "%s", zone->id); clock.automaticZone = automatic;
    clock.zoneState = zoneBusy ? shell::ZONE_LOOKING : shell::ZONE_IDLE;
    return true;
  }
  bool detectTimeZone() override {
    if (!clock.automaticZone || !radioFree()) return false;
    if (zoneSettings.reserved[0] && !storeZone(clock.zone, true, false)) return false;
    detections++; zoneJobToken = zoneGeneration;
    zoneBusy = ssid[0] != 0;
    clock.zoneState = zoneBusy ? shell::ZONE_LOOKING : shell::ZONE_FAILED;
    return true;
  }
  bool setZone(const char* what, const char* arg) {
    if (!strcmp(what, "zonesavefail")) { failZoneSave = true; return true; }
    const bool ok = !strcmp(what, "zoneok");
    if (!ok && strcmp(what, "zonefail")) return false;
    if (!zoneBusy) return true;
    zoneBusy = false;
    if (zoneJobToken != zoneGeneration || !clock.automaticZone) {
      clock.zoneState = shell::ZONE_IDLE;
      if (zoneRetry) { zoneRetry = false; detectTimeZone(); }
      return true;
    }
    const bool applied = ok && storeZone(arg[0] ? arg : timezones::DEFAULT_ZONE, true, true);
    zoneRetryPolicy.completed(clockSequence);
    clock.zoneState = applied ? shell::ZONE_OK : shell::ZONE_FAILED;
    return true;
  }
  void tickClockRetry() { (void)zoneRetryPolicy.afterSync(clockSequence, simulatedMs(), false); }
  bool setClock(const char* what, const char* arg) {
    if (!strcmp(what, "utc")) { setUtcNow((uint32_t)strtoul(arg, nullptr, 10)); return true; }
    const bool ok = !strcmp(what, "timeok");
    if (!ok && strcmp(what, "timefail")) return false;
    if (clock.state != shell::TIME_SYNCING) return true;   // only a pending request can complete
    if (!ok) { clock.state = shell::TIME_FAILED; return true; }
    const uint32_t epoch = arg[0] ? (uint32_t)strtoul(arg, nullptr, 10) : utcNow();
    if (epoch < netclock::MIN_UTC || epoch >= netclock::MAX_UTC) { clock.state = shell::TIME_FAILED; return true; }
    setUtcNow(epoch); clock.state = shell::TIME_OK; clock.lastSync = epoch;
    ++clockSequence;
    if (zoneRetryPolicy.afterSync(clockSequence, simulatedMs(), clock.automaticZone &&
                                      (clock.zoneState == shell::ZONE_FAILED || unresolvedZoneCache)))
      detectTimeZone();
    return true;
  }
  // What a scan hears: one name twice (a dual-band router), a mesh's two nodes, a hidden network, open ones.
  void scanned() {
    static const struct { const char* ssid; int rssi; bool secured; } AIR[] = {
      {"Home-Network-5G-Upstairs", -71, true}, {"Cafe Guest", -80, false}, {"", -40, true}, {"Neighbors_2.4GHz", -66, true},
      {"Home-Network-5G-Upstairs", -48, true}, {"Printer-Direct-7F", -77, true}, {"Library Free WiFi", -58, false},
      {"Mesh-Node", -62, true}, {"Mesh-Node", -55, true}};
    nNets = 0;
    for (const auto& a : AIR) nNets = shell::addNetwork(nets, nNets, a.ssid, a.rssi, a.secured);
  }
  bool setWifi(const char* what) {
    if (!strcmp(what, "scan")) scanned();
    else if (!strcmp(what, "scanempty")) nNets = 0;
    else if (!strcmp(what, "joinok")) tested = 1;
    else if (!strcmp(what, "joinfail")) tested = 0;
    else if (!strcmp(what, "savefail")) failSave = true;
    else if (!strcmp(what, "nohome")) homeSet = false;
    else if (!strcmp(what, "found")) looked = 1;
    else if (!strcmp(what, "notfound")) looked = 0;
    else if (!strcmp(what, "homefail")) failHome = true;
    else return false;
    return true;
  }
  void restart() override { restarts++; rebootDue = true; }   // after this frame: not from inside Shell::update
  shell::Channel channel() override { return ch; }
  void setChannel(shell::Channel c) override {   // the contract in shell/system.h
    ch = c;
    if (st.state == shell::UP_CHECKING || st.state == shell::UP_DOWNLOADING || st.state == shell::UP_READY) return;
    st.state = shell::UP_IDLE; st.available[0] = 0;
  }
  void powerOn() {   // a READY image starts in the other slot; the status is RAM: idle again, but lastCheck is in NVS
    if (st.state == shell::UP_READY) { snprintf(version, sizeof version, "%s", st.available); slot ^= 1; }
    const uint32_t last = st.lastCheck;
    st = {}; st.lastCheck = last; nNets = tested = looked = 0;
    FileStore store; timezones::Settings saved{};
    zoneSettings = timezones::settings(timezones::DEFAULT_ZONE, true);
    const bool loaded = store.load("sys", "tz", &saved, sizeof saved) == sizeof saved && timezones::validSettings(saved);
    if (loaded) zoneSettings = saved;
    unresolvedZoneCache = loaded && zoneSettings.automatic && !zoneSettings.reserved[0];
    timezones::select(zoneSettings.zone);
    clock = {}; zoneBusy = zoneRetry = false; ++zoneGeneration;
    zoneRetryPolicy = {}; clockSequence = 0;
    snprintf(clock.zone, sizeof clock.zone, "%s", zoneSettings.zone);
    clock.automaticZone = zoneSettings.automatic != 0;
  }
  void set(const char* what, const char* arg) {
    if (!strcmp(what, "wifi")) { saveWifi(arg, ""); return; }   // host-only saved-network race fixture
    if (setZone(what, arg) || setClock(what, arg) || setWifi(what)) return;
    if (!strcmp(what, "version")) snprintf(version, sizeof version, "%s", arg);
    else if (!strcmp(what, "newest")) snprintf(st.available, sizeof st.available, "%s", arg);
    else if (!strcmp(what, "nowifi")) { ssid[0] = pass[0] = 0; st.state = shell::UP_NO_WIFI; }
    else if (!strcmp(what, "current")) { st.state = shell::UP_CURRENT; snprintf(st.available, sizeof st.available, "%s", version); }
    else if (!strcmp(what, "download")) { st.state = shell::UP_DOWNLOADING; st.pct = (uint8_t)atoi(arg); }
    else if (!strcmp(what, "ready")) { st.state = shell::UP_READY; st.pct = 100; }
    else if (!strcmp(what, "failed")) st.state = shell::UP_FAILED;
    else { fprintf(stderr, "unknown command: sys %s\n", what); return; }
    if (strcmp(what, "version") && strcmp(what, "newest")) st.lastCheck = now();
  }
  void print() const {
    static const char* const TIME[] = {"idle", "syncing", "ok", "nowifi", "failed"};
    printf("[sys sys_time=%s sys_sync=%lu sys_off=%d sys_syncs=%d]\n", TIME[clock.state],
           (unsigned long)(clock.lastSync ? timezones::local(clock.lastSync) : 0), timezones::offset(utcNow()), syncs);
    static const char* const ZONE[] = {"idle", "looking", "ok", "failed"};
    printf("[sys sys_zone=\"%s\" sys_auto=%d sys_zone_state=%s sys_detections=%d sys_zone_cached=%d sys_utc=%lu sys_local=%lu]\n",
           clock.zone, clock.automaticZone, ZONE[clock.zoneState], detections, zoneSettings.reserved[0], (unsigned long)utcNow(), (unsigned long)now());
    static const char* const S[] = {"idle", "nowifi", "checking", "current", "downloading", "ready", "failed"};
    static_assert(sizeof S / sizeof S[0] == shell::UP_COUNT, "one name per UpdState");
    printf("[sys sys_ver=%s sys_chan=%s sys_state=%s sys_checks=%d sys_restarts=%d sys_last=%lu]\n", version,
           ch == shell::CH_BETA ? "beta" : "stable", S[st.state], checks, restarts, (unsigned long)st.lastCheck);
    printf("[sys sys_ssid=\"%s\" sys_pass=\"%s\" sys_scans=%d sys_nets=%d sys_tests=%d sys_tried=\"%s\" sys_tested=%d]\n", ssid,
           pass, scans, nNets, tests, tried, tested);
    printf("[sys sys_home=%d sys_lat=%.4f sys_lon=%.4f sys_lookups=%d sys_addr=\"%s\"]\n", homeSet, homeAt.lat, homeAt.lon,
           lookups, addr);
  }
};

// Canned planes for Plane Radar: they fly on, one step per 5 s of sim time (a fetch on the board), and wrap around.
// Between them they cover every kind, a plane with no callsign, one past the 20 km ring (a rim dot) and one near the
// glass's edge (its tag goes on the far side).
struct SimFeed : air::Feed {
  air::State state = air::S_LIVE; bool on = false, empty = false; float km = 0; int starts = 0;
  void start(float radiusKm) override { if (!on) starts++; on = true; km = radiusKm; }
  void stop() override { on = false; }
  void read(air::Snapshot& out) override;
  bool spot(double* la, double* lo) override;   // home, from the system menu (SimSystem)
  void set(const char* what, const char*) {
    static const char* const S[] = {"off", "nospot", "nowifi", "joining", "live", "lost"};
    empty = !strcmp(what, "empty");
    if (empty) { state = air::S_LIVE; return; }
    for (int i = 0; i < air::S_COUNT; i++)
      if (!strcmp(what, S[i])) { state = (air::State)i; return; }
    fprintf(stderr, "unknown command: feed %s\n", what);
  }
  void print() const {
    printf("[feed feed_state=%d feed_on=%d feed_km=%.1f feed_starts=%d]\n", (int)state, on, km, starts);
  }
};
static SimFeed g_feed;

static Game g_pets;
static biscuit::Game g_biscuit;
static marble::Game g_marble;
static fc::Game g_tiltfc;
static pumpkin::Game g_pumpkin;
static radar::Game g_radar;
static starfighter::Game g_starfighter;
static kingdom::Game g_kingdom;
static toytown::Game g_toytown;
static App* const APPS[] = {&g_pets, &g_biscuit, &g_marble, &g_tiltfc, &g_pumpkin, &g_radar, &g_starfighter, &g_kingdom, &g_toytown};
static FileStore g_store;
static SimSystem g_sys;
static Shell g_shell;
static InputTracker g_tracker;
bool SimFeed::spot(double* la, double* lo) {
  shell::Home h;
  const bool set = g_sys.home(h);
  *la = h.lat; *lo = h.lon;
  return set;
}
static uint32_t g_ms = 0, g_epoch = 0;
static uint32_t simulatedMs() { return g_ms; }
static uint32_t g_pal[TINT_COUNT][C_COUNT];
static uint16_t g_pal565[TINT_COUNT][C_COUNT];   // the same, as the firmware hands it to board::present
// The panel's two buffers, as on the device: every present flips them, an indexed frame's too (upscaled into the one it
// presents), so an RGB565 render's target holds the frame from two presents ago. Magenta once at start, never cleared:
// a pixel a render forgets shows up.
static uint16_t g_fb565[2][gfx565::W * gfx565::H];
static int g_shown = 0;                          // the buffer presented last: what the glass shows
static bool g_hires = false;                     // that frame was an RGB565 app's
static int16_t g_gravity[3] = {0, 1000, 0};      // the motion sensor: held upright
static int g_shakeFrames = 0;                    // frames of shaking left
static double g_updateMs = 0, g_renderMs = 0; static int g_perfFrames = 0;

static uint32_t utcNow() { return g_epoch + g_ms / 1000; }
static uint32_t now() { return timezones::local(utcNow()); }

void SimFeed::read(air::Snapshot& out) {
  static const air::Plane PLANES[] = {
    {-9.0f, 6.0f, 95, 420, 12300, "ASA1234", "SEA-SFO", air::K_AIRLINER},
    {4.0f, -11.0f, 340, 310, 8650, "DAL88", "SEA-ATL", air::K_AIRLINER},
    {12.0f, 9.0f, 220, 120, 2500, "N512SP", "", air::K_SMALL},
    {-3.0f, -4.0f, 45, 160, 4100, "", "", air::K_SMALL},
    {6.5f, 14.5f, 180, 380, 21000, "RCH401", "", air::K_MILITARY},
    {-18.0f, 1.5f, 70, 450, 35000, "UAL905", "SFO-NRT", air::K_AIRLINER},
    {22.0f, -6.0f, 270, 400, 30000, "SWA77", "OAK-SEA", air::K_AIRLINER},
  };
  out = air::Snapshot{};
  out.state = state;
  if (state != air::S_LIVE) return;
  const uint32_t step = g_ms / 5000;
  out.gen = step + 1;
  if (empty) return;
  for (const air::Plane& p : PLANES) {
    air::Plane q = p;
    const float km = q.knots * 1.852f / 3600 * 5 * (float)(step % 24);   // 5 s a step, wrapping every 2 minutes
    q.eastKm += sinf(q.trackDeg * 3.14159265f / 180) * km;
    q.northKm += cosf(q.trackDeg * 3.14159265f / 180) * km;
    out.planes[out.n++] = q;
  }
}
static void setUtcNow(uint32_t epoch) { g_epoch = epoch - g_ms / 1000; }
static void wipe() {   // every namespace, every key: all the .sav files
  DIR* d = opendir("build/host"); if (!d) return;
  while (dirent* e = readdir(d)) {
    size_t n = strlen(e->d_name);
    if (n > 4 && !strcmp(e->d_name + n - 4, ".sav")) remove((std::string("build/host/") + e->d_name).c_str());
  }
  closedir(d);
}
static void boot(bool seedClock = true) {
  const int n = (int)(sizeof APPS / sizeof APPS[0]);
  for (int k = 0; k < n; k++) {   // every game's name, at the launcher slot it has with these n, whole and off the bezel
    const int p = launcher::pageOf(n, k), first = launcher::first(n, p);
    assert(launcher::name(APPS[k]->name(), k - first, launcher::count(n, p), n).fits);
  }
  g_sys.powerOn();
  air::setFeed(&g_feed);
  g_shell.begin(g_store, APPS, n, &g_sys);
  if (seedClock) g_shell.update(now(), g_ms, Input{});   // fixture creation must see the current local play window
}
static void reset() { wipe(); boot(); }

// Physical pixel (x, y) of the panel as RGB888: the indexed frame upscaled 3x through the tint's palette, or the RGB565
// surface as is, with the round glass masked, under the active backlight (Shell::backlight(); the firmware's idle
// dimming is not modelled). Snapshots, the web emulator and the window all show this.
static uint32_t dimmed(uint32_t c) {
  static int pct = 100, k = 256;
  if (g_shell.backlight() >= 100) return c;
  // The backlight scales light, not sRGB values: each channel by pct^(1/2.2), so Soft's 40% looks like the glass does.
  if (g_shell.backlight() != pct) { pct = g_shell.backlight(); k = (int)(256 * powf(pct / 100.0f, 1 / 2.2f)); }
  return (uint32_t)(((c >> 16 & 255) * k >> 8) << 16 | ((c >> 8 & 255) * k >> 8) << 8 | ((c & 255) * k >> 8));
}
static uint32_t framePixel(int x, int y) {
  int dx = x - 240, dy = y - 240;
  if (dx * dx + dy * dy > 240 * 240) return 0x202020;   // outside the round glass: dark
  if (g_hires) return dimmed(gfx565::rgb888(g_fb565[g_shown][y * gfx565::W + x]));
  return dimmed(g_pal[g_shell.tint()][gfx::fb[(y / 3) * gfx::W + x / 3]]);   // at full RGB888, as snapshots always were
}

static void writeBMP(const char* path) {
  const int W = gfx565::W, H = gfx565::H;
  FILE* f = fopen(path, "wb"); if (!f) { perror(path); return; }
  uint32_t rowBytes = (uint32_t)W * 3, fileSize = 54 + rowBytes * (uint32_t)H;
  uint8_t hdr[54] = {'B', 'M'}; memcpy(hdr + 2, &fileSize, 4); uint32_t off = 54; memcpy(hdr + 10, &off, 4);
  uint32_t dib = 40; memcpy(hdr + 14, &dib, 4); int32_t w = W, h = H; memcpy(hdr + 18, &w, 4); memcpy(hdr + 22, &h, 4);
  uint16_t planes = 1, bpp = 24; memcpy(hdr + 26, &planes, 2); memcpy(hdr + 28, &bpp, 2);
  fwrite(hdr, 1, 54, f);
  std::vector<uint8_t> row(rowBytes);
  for (int y = H - 1; y >= 0; y--) {
    for (int x = 0; x < W; x++) {
      uint32_t c = framePixel(x, y);
      row[x * 3 + 0] = c & 255; row[x * 3 + 1] = (c >> 8) & 255; row[x * 3 + 2] = (c >> 16) & 255;
    }
    fwrite(row.data(), 1, rowBytes, f);
  }
  fclose(f);
}

// board::present: the 160x160 indexed frame, 3x, into the buffer about to be shown.
static void upscale(uint16_t* dst, const uint16_t* pal) {
  for (int y = 0; y < gfx565::H; y++) for (int x = 0; x < gfx565::W; x++) dst[y * gfx565::W + x] = pal[gfx::fb[(y / 3) * gfx::W + x / 3]];
}
static double since(std::chrono::steady_clock::time_point t) {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t).count();
}
static void frame(bool down, int x, int y) {   // the shell writes saves itself, through g_store
  Input in = g_tracker.step(down, x, y, g_ms);
  in.gx = g_gravity[0]; in.gy = g_gravity[1]; in.gz = g_gravity[2];
  if (g_shakeFrames > 0) { in.gx = (int16_t)(g_shakeFrames-- & 1 ? 1800 : -1800); in.gz = 900; }   // hard jolts side to side
  auto t = std::chrono::steady_clock::now();
  g_sys.tickClockRetry();
  g_shell.update(now(), g_ms, in);
  g_updateMs += since(t);
  t = std::chrono::steady_clock::now();
  g_shown ^= 1;
  g_hires = g_shell.surface() == SURFACE_RGB565;
  if (g_hires) gfx565::target(g_fb565[g_shown]);
  g_shell.render();
  g_renderMs += since(t); g_perfFrames++;
  if (!g_hires) upscale(g_fb565[g_shown], g_pal565[g_shell.tint()]);
  if (g_sys.rebootDue) { g_sys.rebootDue = false; boot(); }   // restart(): the board reboots between frames
}
// Clamped to the sensor's +-4 g full scale: nothing past it ever reaches Input on the device.
static int16_t mg(const char* s) { int v = atoi(s); return (int16_t)(v < -4000 ? -4000 : v > 4000 ? 4000 : v); }
static void setTilt(const char* x, const char* y, const char* z) { g_gravity[0] = mg(x); g_gravity[1] = mg(y); g_gravity[2] = mg(z); }
static void perf() {
  const int n = g_perfFrames ? g_perfFrames : 1;
  printf("perf frames=%d update=%.3fms render=%.3fms\n", g_perfFrames, g_updateMs / n, g_renderMs / n);
  g_updateMs = g_renderMs = 0; g_perfFrames = 0;
}

// The same frame as writeBMP, as raw RGB triples (PPM order) to stdout.
static void writePPM() {
  const int W = gfx565::W, H = gfx565::H;
  std::vector<uint8_t> buf((size_t)W * H * 3);
  size_t i = 0;
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      uint32_t c = framePixel(x, y);
      buf[i++] = (c >> 16) & 255; buf[i++] = (c >> 8) & 255; buf[i++] = c & 255;
    }
  }
  printf("PPM %zu\n", buf.size());
  fwrite(buf.data(), 1, buf.size(), stdout);
  fflush(stdout);
}

static bool serveExtra(const char* cmd, const char* a, const char* b) {
  if (!strcmp(cmd, "perf")) { perf(); return false; }
  if (!strcmp(cmd, "sys")) { g_sys.set(a, b); puts("ok"); return true; }
  puts("ok"); return false;
}

// --serve mode: newline-delimited commands on stdin, one response on stdout per command (flushed).
// down/move/up/tick/skip/reset all print "ok"; frame prints a binary PPM (P6) preceded by "PPM <bytes>".
static void runServe() {
  bool down = false; int cx = 0, cy = 0;
  char line[256];
  while (fgets(line, sizeof line, stdin)) {
    char cmd[32] = {0}; char a[64] = {0}, b[64] = {0};
    if (sscanf(line, "%31s %63s %63s", cmd, a, b) < 1) continue;
    if (!strcmp(cmd, "down")) { down = true; cx = atoi(a); cy = atoi(b); frame(true, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "move")) { cx = atoi(a); cy = atoi(b); frame(down, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "up")) { down = false; frame(false, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "tick")) { int ms = atoi(a); for (int t = 0; t < ms; t += 33) { frame(down, cx, cy); g_ms += 33; } puts("ok"); }
    else if (!strcmp(cmd, "skip")) { g_epoch += (uint32_t)atol(a); frame(down, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "reset")) { reset(); down = false; puts("ok"); }
    else if (!strcmp(cmd, "frame")) writePPM();
    else if (!strcmp(cmd, "dbg")) { g_shell.debugCmd(a); puts("ok"); }
    else if (!strcmp(cmd, "tilt")) { char c[64] = {0}; sscanf(line, "%*s %*s %*s %63s", c); setTilt(a, b, c); puts("ok"); }
    else if (!strcmp(cmd, "shake")) { g_shakeFrames = 6; puts("ok"); }
    else if (serveExtra(cmd, a, b)) { frame(down, cx, cy); g_ms += 33; }
    fflush(stdout);
  }
}

// ---- script mode. step() advances one 40 ms frame exactly like `wait`, so playtest runs replay identically.
struct Finger { bool down = false; int x = 0, y = 0; };
static void step(bool down, int x, int y) { frame(down, x, y); g_ms += 40; }
static uint32_t g_rng = 1;  // own LCG: a monkey seed replays the same gestures on every platform
static int mrand(int n) { g_rng = g_rng * 1664525u + 1013904223u; return (int)((g_rng >> 8) % (uint32_t)n); }
static void monkeyPoint(int& x, int& y) { do { x = 2 + mrand(157); y = 2 + mrand(157); } while ((x - 80) * (x - 80) + (y - 80) * (y - 80) > 78 * 78); }
static void press(int x, int y, int ms) { for (int t = 0; t < ms; t += 40) step(true, x, y); step(false, x, y); }  // 80 ms = the script `tap`
static void drag(int x, int y) {
  int ex, ey, k = 4 + mrand(5); monkeyPoint(ex, ey);
  step(true, x, y); for (int j = 1; j <= k; j++) step(true, x + (ex - x) * j / k, y + (ey - y) * j / k); step(false, ex, ey);
}
// Chaos monkey: a kid mashing the glass. 65% taps, 10% double taps, 10% drags, 10% slow taps (30-200 ms), 5% long presses.
static void monkey(int n, uint32_t seed) {
  g_rng = seed;
  for (int i = 1; i <= n; i++) {
    int x, y, r = mrand(100); monkeyPoint(x, y);
    if (r < 65) press(x, y, 80);
    else if (r < 75) { press(x, y, 80); press(x, y, 80); }  // double tap: the second press starts 120 ms after the first
    else if (r < 85) drag(x, y);
    else if (r < 95) press(x, y, 30 + mrand(171));
    else press(x, y, 700);
    for (int t = 0, w = 40 + mrand(461); t < w; t += 40) step(false, x, y);
    if (i % 100 == 0) printf("monkey %d screen=%s\n", i, g_shell.screenName());
  }
  puts("monkey done");
}
// Steps frames like `wait` and reports each trick-lesson glow: the orange (C_ORANGE) ring at the lit zone. Rows above
// y=26 are skipped (the orange home button); a dozen pixels minimum so stray confetti does not count.
static void watch(int ms, const Finger& f) {
  bool was = false;
  for (int t = 0; t < ms; t += 40) {
    step(f.down, f.x, f.y);
    long sx = 0, sy = 0, n = 0;
    for (int py = 26; py < gfx::H; py++) for (int px = 0; px < gfx::W; px++) if (gfx::fb[py * gfx::W + px] == C_ORANGE) { sx += px; sy += py; n++; }
    if (n >= 12 && !was) printf("glow %ld %ld\n", (sx + n / 2) / n, (sy + n / 2) / n);
    was = n >= 12;
  }
}
static void audit(const Finger& f) {  // one audited frame: every hit region and text box the screen touched
  UiAudit::enabled = true; gfx::textLogEnabled = true; UiAudit::reset(); gfx::textLogReset();
  step(f.down, f.x, f.y);
  printf("ui screen=%s regions=%d texts=%d\n", g_shell.screenName(), UiAudit::count, gfx::textLogCount);
  for (int i = 0; i < UiAudit::count; i++) printf("region %d %d %d %d\n", UiAudit::regions[i].x, UiAudit::regions[i].y, UiAudit::regions[i].w, UiAudit::regions[i].h);
  for (int i = 0; i < gfx::textLogCount; i++) {
    const gfx::TextBox& t = gfx::textLog[i];
    if (t.rgb) printf("text %d %d %d %d rgb=%06X bg=%06X\n", t.x, t.y, t.w, t.h, (unsigned)t.color, (unsigned)t.bg);
    else printf("text %d %d %d %d color=%u bg=%u\n", t.x, t.y, t.w, t.h, (unsigned)t.color, (unsigned)t.bg);
  }
  UiAudit::enabled = false; gfx::textLogEnabled = false;
}
// newgame KID PET: a fresh device with one profile (age 8) whose pup is already adopted, opened in Pets Club (splash).
static void newgame(const char* kid, const char* pup) {
  reset();
  int id = g_shell.createProfile(kid, 8, "");
  Save s; pet::adopt(s, now(), kid, pup); s.kidAge = 8; pet::seal(s);
  g_store.save(g_pets.store(), shell::key('s', id), &s, sizeof s);
  g_shell.openApp(g_pets.name());
}

// Each script command gets the finger (it persists between lines for down/move/up/wait) and up to three words.
using Cmd = void (*)(Finger&, const char*, const char*, const char*);
struct Named { const char* name; Cmd fn; };
static const Named COMMANDS[] = {
  {"tap", [](Finger&, const char* a, const char* b, const char*) { press(atoi(a), atoi(b), 80); }},
  {"hold", [](Finger&, const char* a, const char* b, const char*) { press(atoi(a), atoi(b), 1000); }},   // a long press fires at 600 ms
  {"down", [](Finger& f, const char* a, const char* b, const char*) { f = {true, atoi(a), atoi(b)}; step(true, f.x, f.y); }},
  {"move", [](Finger& f, const char* a, const char* b, const char*) { f.x = atoi(a); f.y = atoi(b); step(f.down, f.x, f.y); }},
  {"up", [](Finger& f, const char*, const char*, const char*) { f.down = false; step(false, f.x, f.y); }},
  {"wait", [](Finger& f, const char* a, const char*, const char*) { for (int t = 0; t < atoi(a); t += 40) step(f.down, f.x, f.y); }},
  {"skip", [](Finger& f, const char* a, const char*, const char*) { g_epoch += (uint32_t)atol(a); step(f.down, f.x, f.y); }},
  {"snap", [](Finger& f, const char* a, const char*, const char*) {
     char p[256]; snprintf(p, sizeof p, "build/host/%s.bmp", a[0] ? a : "snap"); frame(f.down, f.x, f.y); writeBMP(p); printf("wrote %s\n", p); }},
  {"reset", [](Finger& f, const char*, const char*, const char*) { reset(); f.down = false; }},
  {"profile", [](Finger&, const char* a, const char* b, const char* c) { g_shell.createProfile(a, (uint8_t)atoi(b), c); }},
  {"app", [](Finger&, const char* a, const char*, const char*) { if (!g_shell.openApp(a)) printf("app %s: not opened\n", a); }},
  {"newgame", [](Finger& f, const char* a, const char* b, const char*) { newgame(a, b); f.down = false; }},
  {"debug", [](Finger&, const char*, const char*, const char*) { g_shell.debugPrint(); g_sys.print(); g_feed.print(); }},
  {"ui", [](Finger& f, const char*, const char*, const char*) { audit(f); }},
  {"dbg", [](Finger&, const char* a, const char*, const char*) { g_shell.debugCmd(a); }},
  {"screen", [](Finger&, const char*, const char*, const char*) { printf("screen=%s\n", g_shell.screenName()); }},
  {"echo", [](Finger&, const char* a, const char*, const char*) { puts(a); }},
  {"watch", [](Finger& f, const char* a, const char*, const char*) { watch(atoi(a), f); }},
  {"monkey", [](Finger& f, const char* a, const char* b, const char*) { monkey(atoi(a), (uint32_t)strtoul(b, nullptr, 10)); f.down = false; }},
  {"tilt", [](Finger&, const char* a, const char* b, const char* c) { setTilt(a, b, c); }},
  {"shake", [](Finger& f, const char*, const char*, const char*) { g_shakeFrames = 6; while (g_shakeFrames) step(f.down, f.x, f.y); }},
  {"perf", [](Finger&, const char*, const char*, const char*) { perf(); }},
  {"reboot", [](Finger& f, const char*, const char*, const char*) { boot(); f.down = false; }},
  {"sys", [](Finger&, const char* a, const char* b, const char*) { g_sys.set(a, b); }},
  {"feed", [](Finger&, const char* a, const char* b, const char*) { g_feed.set(a, b); }},
};
static void runScript(const char* path) {
  FILE* f = fopen(path, "r"); if (!f) { perror(path); exit(1); }
  setvbuf(stdout, nullptr, _IOLBF, 0);  // keep every line written before a sanitizer abort
  char line[256]; Finger finger;
  while (fgets(line, sizeof line, f)) {
    char cmd[32] = {0}, a[64] = {0}, b[64] = {0}, c[64] = {0};
    if (sscanf(line, "%31s %63s %63s %63s", cmd, a, b, c) < 1 || cmd[0] == '#') continue;
    const Named* hit = nullptr;
    for (const Named& n : COMMANDS) if (!strcmp(cmd, n.name)) hit = &n;
    if (hit) hit->fn(finger, a, b, c); else fprintf(stderr, "unknown command: %s\n", cmd);
  }
  fclose(f);
}

#ifdef HAVE_SDL
static void onKey(SDL_Keycode k, bool& run) {
  if (k == SDLK_q || k == SDLK_ESCAPE) run = false;
  if (k == SDLK_h) g_epoch += 3600;            // +1 hour
  if (k == SDLK_d) g_epoch += 86400;           // +1 day
  if (k == SDLK_n) g_epoch += 8 * 3600;        // +8 hours (a night)
  if (k == SDLK_s) { writeBMP("build/host/shot.bmp"); printf("wrote build/host/shot.bmp\n"); }
  if (k == SDLK_r) reset();
  if (k == SDLK_p) g_shell.debugPrint();
  if (k == SDLK_b) g_shell.debugCmd("light");   // the next brightness, as the launcher's sun picks it
  if (k == SDLK_SPACE) g_shakeFrames = 6;
}
// The motion sensor: left/right turn gravity, up lays the board flatter, down stands it up.
static void onArrow(SDL_Keycode k) {
  if (k != SDLK_LEFT && k != SDLK_RIGHT && k != SDLK_UP && k != SDLK_DOWN) return;
  float a = atan2f(g_gravity[0], g_gravity[1]), m = fminf(1, hypotf(g_gravity[0], g_gravity[1]) / 1000);
  a += k == SDLK_LEFT ? -0.15f : k == SDLK_RIGHT ? 0.15f : 0;
  m = fminf(1, fmaxf(0, m + (k == SDLK_DOWN ? 0.1f : k == SDLK_UP ? -0.1f : 0)));
  g_gravity[0] = (int16_t)(sinf(a) * m * 1000); g_gravity[1] = (int16_t)(cosf(a) * m * 1000); g_gravity[2] = (int16_t)(-sqrtf(1 - m * m) * 1000);
}
static void runWindow() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* win = SDL_CreateWindow("Porthole", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 480, SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, gfx565::W, gfx565::H);
  bool run = true, down = false; int mx = 0, my = 0; uint32_t start = SDL_GetTicks();
  static uint32_t pix[gfx565::W * gfx565::H];
  while (run) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) run = false;
      if (e.type == SDL_MOUSEBUTTONDOWN) { down = true; mx = e.button.x / 3; my = e.button.y / 3; }
      if (e.type == SDL_MOUSEBUTTONUP) down = false;
      if (e.type == SDL_MOUSEMOTION) { mx = e.motion.x / 3; my = e.motion.y / 3; }
      if (e.type == SDL_KEYDOWN) { onKey(e.key.keysym.sym, run); onArrow(e.key.keysym.sym); }
    }
    g_ms = SDL_GetTicks() - start;
    frame(down, mx, my);
    for (int i = 0; i < gfx565::W * gfx565::H; i++) pix[i] = 0xFF000000u | framePixel(i % gfx565::W, i / gfx565::W);   // the panel's own pixels
    SDL_UpdateTexture(tex, nullptr, pix, gfx565::W * 4);
    SDL_RenderClear(ren); SDL_RenderCopy(ren, tex, nullptr, nullptr); SDL_RenderPresent(ren);
  }
  SDL_Quit();
}
#else
static void runWindow() { fprintf(stderr, "built without SDL; use --script\n"); }
#endif

int main(int argc, char** argv) {
  for (int t = 0; t < TINT_COUNT; t++) {
    palette_build((Tint)t, g_pal[t]);
    for (int i = 0; i < C_COUNT; i++) g_pal565[t][i] = rgb888_to_565(g_pal[t][i]);
  }
  for (auto& b : g_fb565) for (uint16_t& p : b) p = 0xF81F;
  gfx565::target(g_fb565[0]);
  // Default clock: a fixed Tuesday 16:00 local so snapshots are deterministic; --now overrides.
  g_epoch = 1790000000u - (1790000000u % 86400u) + 16 * 3600;
  const char* script = nullptr; bool serve = false;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--script") && i + 1 < argc) script = argv[++i];
    else if (!strcmp(argv[i], "--serve")) serve = true;
    else if (!strcmp(argv[i], "--now") && i + 1 < argc) g_epoch = (uint32_t)atol(argv[++i]);
    else if (!strcmp(argv[i], "--fresh")) wipe();
  }
  boot(false);
  g_epoch = timezones::toUtc(g_epoch);   // --now and the deterministic default remain local-wall inputs
  g_shell.update(now(), g_ms, Input{});   // initial programmatic profile/newgame hooks run before the first rendered frame
  if (script) runScript(script);
  else if (serve) runServe();
  else runWindow();
  return 0;
}
