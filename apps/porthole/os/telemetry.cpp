// Opt-in telemetry's portable half: the owner's settings, the queue and the two wire formats. See telemetry.h.
#include "telemetry.h"
#include <stdio.h>
#include <string.h>
#include "crc32.h"

namespace telemetry {
namespace {
// The words of the SD file or the serial line: lines and spaces both separate them (no value holds a space).
int words(const char* text, char (*w)[128], int max) {
  int n = 0;
  while (*text && n < max) {
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') text++;
    if (!*text) break;
    size_t len = strcspn(text, " \t\r\n");
    if (len >= sizeof w[0]) return -1;   // too long for any field
    memcpy(w[n], text, len); w[n][len] = 0;
    n++; text += len;
  }
  while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') text++;
  return *text ? -1 : n;   // more words than any setting has
}
bool lastIsLevel(const char* w, uint8_t* level) {
  if (!strcmp(w, "anonymous")) { *level = ANONYMOUS; return true; }
  if (!strcmp(w, "personal")) { *level = PERSONAL; return true; }
  return false;
}

// A JSON body written into a fixed buffer: ok goes false (and stays) once something did not fit.
struct Out {
  char* p; size_t cap, n = 0; bool ok = true;
  Out(char* buf, size_t c) : p(buf), cap(c) { if (cap) p[0] = 0; }
  void put(const char* s) {
    const size_t len = strlen(s);
    if (!ok || n + len + 1 > cap) { ok = false; return; }
    memcpy(p + n, s, len + 1); n += len;
  }
  void str(const char* s) {   // a quoted JSON string; control bytes become spaces (no value needs them)
    put("\"");
    for (; *s; s++) {
      const char c = (unsigned char)*s < 0x20 ? ' ' : *s;
      char b[3] = {c, 0, 0};
      if (c == '"' || c == '\\') { b[0] = '\\'; b[1] = c; }
      put(b);
    }
    put("\"");
  }
  void num(long v) { char b[16]; snprintf(b, sizeof b, "%ld", v); put(b); }
};

// UTC seconds as "2026-11-16T09:30:00Z" (days to a civil date: H. Hinnant's algorithm).
void iso(uint32_t utc, char out[32]) {
  const int64_t z = utc / 86400 + 719468;
  const int64_t era = z / 146097, doe = z - era * 146097;
  const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
  const int d = (int)(doy - (153 * mp + 2) / 5 + 1), m = (int)(mp < 10 ? mp + 3 : mp - 9);
  const int y = (int)(yoe + era * 400 + (m <= 2));
  const uint32_t s = utc % 86400;
  snprintf(out, 32, "%04d-%02d-%02dT%02lu:%02lu:%02luZ", y, m, d, (unsigned long)(s / 3600), (unsigned long)(s / 60 % 60),
           (unsigned long)(s % 60));
}
void hex(const uint8_t* b, int n, char* out) {
  static const char D[] = "0123456789abcdef";
  for (int i = 0; i < n; i++) { out[2 * i] = D[b[i] >> 4]; out[2 * i + 1] = D[b[i] & 15]; }
  out[2 * n] = 0;
}
// The anonymous level keeps only what says which games get played, never by whom or how well.
const char* const ANON_EVENTS[] = {"device_started", "game_opened", "game_closed", "crashed"};
const char* const ANON_KEYS[] = {"game", "seconds", "reset", "channel"};
template <size_t N> bool listed(const char* const (&list)[N], const char* s) {
  for (const char* x : list) if (!strcmp(x, s)) return true;
  return false;
}
}  // namespace

// The URL and the optional token: w[1] and, when last is 3, w[2].
static bool takeUrl(const char (*w)[128], int last, Config& c) {
  const size_t url = strlen(w[1]), token = last == 3 ? strlen(w[2]) : 0;
  if (last > 3 || strncmp(w[1], "https://", 8) || url <= 8 || url >= sizeof c.url || token >= sizeof c.token) return false;
  memcpy(c.url, w[1], url + 1);
  if (token) memcpy(c.token, w[2], token + 1);
  return true;
}
bool parseConfig(const char* text, Config& out) {
  char w[4][128];
  const int n = text ? words(text, w, 4) : -1;
  if (n == 1 && !strcmp(w[0], "off")) { out = Config{}; return true; }
  if (n < 2) return false;
  Config c;
  if (!strcmp(w[0], "posthog")) c.format = POSTHOG;
  else if (!strcmp(w[0], "otlp")) c.format = OTLP;
  else return false;
  c.level = PERSONAL;
  const int last = n > 2 && lastIsLevel(w[n - 1], &c.level) ? n - 1 : n;
  if (!takeUrl(w, last, c)) return false;
  if (c.format == POSTHOG && !c.token[0]) return false;   // PostHog takes no event without the project's key
  out = c;
  return true;
}
bool sameConfig(const Config& a, const Config& b) {
  return a.level == b.level && a.format == b.format && !strcmp(a.url, b.url) && !strcmp(a.token, b.token);
}
const char* host(const Config& c, char* out, size_t cap) {
  const char* h = strstr(c.url, "://");
  h = h ? h + 3 : c.url;
  snprintf(out, cap, "%.*s", (int)strcspn(h, "/:"), h);
  return out;
}

void Queue::begin(uint32_t (*utcNow)(), const char* version) { utc_ = utcNow; version_ = version; }
void Queue::configure(const Config& c) {
  if (c.level != cfg_.level) { head_ = n_ = 0; }   // what was kept under one level never goes out under another
  cfg_ = c;
}
void Queue::player(const char* id) { snprintf(player_, sizeof player_, "%s", id ? id : ""); }
bool Queue::keep(const char* name) const {
  return cfg_.level == PERSONAL || (cfg_.level == ANONYMOUS && listed(ANON_EVENTS, name));
}
void Queue::record(const char* name, const Prop* p, int n) {
  if (!keep(name)) return;
  if (n_ == CAP) { head_ = (head_ + 1) % CAP; n_--; }   // full: the oldest goes
  Event& e = q_[(head_ + n_++) % CAP];
  e.name = name; e.utc = utc_ ? utc_() : 0; e.n = 0;
  const bool personal = cfg_.level == PERSONAL;
  snprintf(e.player, sizeof e.player, "%s", personal ? player_ : "");
  for (int i = 0; i < n && e.n < MAX_PROPS; i++) {
    if (!personal && !listed(ANON_KEYS, p[i].key)) continue;
    auto& q = e.p[e.n++];
    q.key = p[i].key; q.text = p[i].str != nullptr; q.num = p[i].num;
    snprintf(q.str, sizeof q.str, "%s", q.text ? p[i].str : "");
  }
}
void Queue::drop(int n) {
  if (n > n_) n = n_;
  head_ = (head_ + n) % CAP; n_ -= n;
}
// Two crc32s over the secret salt, the profile and the name: stable, short, and with the salt unknown to whoever reads
// the data, no way back to the name. shortcut: not a cryptographic hash; fine while only the owner sees the ids.
void Queue::playerId(int profile, const char* name, char out[ID]) const {
  uint8_t mix[8 + 1 + 16] = {0};
  memcpy(mix, cfg_.salt, 8); mix[8] = (uint8_t)profile;
  snprintf((char*)mix + 9, 16, "%s", name ? name : "");
  const uint32_t a = os::crc32(mix, sizeof mix), b = os::crc32(mix + 1, sizeof mix - 1) ^ os::crc32(cfg_.salt, 8);
  uint8_t id[8]; memcpy(id, &a, 4); memcpy(id + 4, &b, 4);
  hex(id, 8, out);
}
// Personal: the device's own id. Anonymous: a hash of it and the month, so months cannot be joined up.
void Queue::distinctId(const Event& e, char out[ID]) const {
  if (cfg_.level == PERSONAL) { hex(cfg_.device, 8, out); return; }
  char month[32]; iso(e.utc, month);
  uint8_t mix[8 + 7]; memcpy(mix, cfg_.device, 8); memcpy(mix + 8, month, 7);   // "2026-11"
  const uint32_t a = os::crc32(mix, sizeof mix), b = os::crc32(mix, sizeof mix - 1) ^ 0x5bd1e995u;
  uint8_t id[8]; memcpy(id, &a, 4); memcpy(id + 4, &b, 4);
  hex(id, 8, out);
}
void Queue::endpoint(char* out, size_t cap) const {
  if (cfg_.format == OTLP) { snprintf(out, cap, "%s", cfg_.url); return; }
  const size_t n = strlen(cfg_.url);
  snprintf(out, cap, "%.*s/batch/", (int)(n && cfg_.url[n - 1] == '/' ? n - 1 : n), cfg_.url);
}
void Queue::auth(char* out, size_t cap) const {
  if (cfg_.format == OTLP && cfg_.token[0]) snprintf(out, cap, "Bearer %s", cfg_.token);
  else if (cap) out[0] = 0;
}
size_t Queue::batch(char* out, size_t cap, int* taken) const {
  *taken = 0;
  if (!n_ || !on()) return 0;
  return cfg_.format == OTLP ? otlp(out, cap, taken) : posthog(out, cap, taken);
}

namespace {
void posthogEvent(Out& o, const Event& e, const char* device, const char* version) {
  char ts[32]; iso(e.utc, ts);
  o.put("{\"event\":"); o.str(e.name);
  o.put(",\"distinct_id\":"); o.str(e.player[0] ? e.player : device);
  o.put(",\"timestamp\":"); o.str(ts);
  o.put(",\"properties\":{\"$lib\":\"porthole\",\"app_version\":"); o.str(version);
  o.put(",\"device\":"); o.str(device);
  if (e.player[0]) { o.put(",\"player\":"); o.str(e.player); }
  else o.put(",\"$process_person_profile\":false");   // a device's own events make no person in PostHog
  for (int i = 0; i < e.n; i++) {
    o.put(","); o.str(e.p[i].key); o.put(":");
    if (e.p[i].text) o.str(e.p[i].str); else o.num(e.p[i].num);
  }
  o.put("}}");
}
void otlpAttr(Out& o, const char* key, const char* text, long num, bool isText) {
  o.put("{\"key\":"); o.str(key);
  if (isText) { o.put(",\"value\":{\"stringValue\":"); o.str(text); }
  else { char b[16]; snprintf(b, sizeof b, "%ld", num); o.put(",\"value\":{\"intValue\":"); o.str(b); }
  o.put("}}");
}
void otlpRecord(Out& o, const Event& e, const char* device) {
  char ns[24]; snprintf(ns, sizeof ns, "%lu000000000", (unsigned long)e.utc);
  o.put("{\"timeUnixNano\":"); o.str(ns);
  o.put(",\"severityNumber\":9,\"severityText\":\"INFO\",\"body\":{\"stringValue\":"); o.str(e.name);
  o.put("},\"attributes\":[");
  otlpAttr(o, "event.name", e.name, 0, true);
  o.put(","); otlpAttr(o, "device.id", device, 0, true);
  if (e.player[0]) { o.put(","); otlpAttr(o, "player.id", e.player, 0, true); }
  for (int i = 0; i < e.n; i++) { o.put(","); otlpAttr(o, e.p[i].key, e.p[i].str, e.p[i].num, e.p[i].text); }
  o.put("]}");
}
}  // namespace

// shortcut: a batch whose reply was lost is sent again whole, so PostHog may count those events twice; add a $insert_id
// if the numbers ever look doubled.
size_t Queue::posthog(char* out, size_t cap, int* taken) const {
  static const char END[] = "]}";
  Out o(out, cap > sizeof END ? cap - sizeof END + 1 : 0);
  o.put("{\"api_key\":"); o.str(cfg_.token); o.put(",\"batch\":[");
  for (int i = 0; i < n_ && o.ok; i++) {
    const size_t mark = o.n;
    char device[ID]; distinctId(q_[(head_ + i) % CAP], device);
    if (i) o.put(",");
    posthogEvent(o, q_[(head_ + i) % CAP], device, version_);
    if (!o.ok) { o.n = mark; out[mark] = 0; break; }
    *taken = i + 1;
  }
  if (!*taken) return 0;
  memcpy(out + o.n, END, sizeof END);
  return o.n + sizeof END - 1;
}
size_t Queue::otlp(char* out, size_t cap, int* taken) const {
  static const char END[] = "]}]}]}";
  Out o(out, cap > sizeof END ? cap - sizeof END + 1 : 0);
  o.put("{\"resourceLogs\":[{\"resource\":{\"attributes\":[");
  otlpAttr(o, "service.name", "porthole", 0, true); o.put(","); otlpAttr(o, "service.version", version_, 0, true);
  o.put("]},\"scopeLogs\":[{\"scope\":{\"name\":\"porthole\"},\"logRecords\":[");
  for (int i = 0; i < n_ && o.ok; i++) {
    const size_t mark = o.n;
    char device[ID]; distinctId(q_[(head_ + i) % CAP], device);
    if (i) o.put(",");
    otlpRecord(o, q_[(head_ + i) % CAP], device);
    if (!o.ok) { o.n = mark; out[mark] = 0; break; }
    *taken = i + 1;
  }
  if (!*taken) return 0;
  memcpy(out + o.n, END, sizeof END);
  return o.n + sizeof END - 1;
}
}  // namespace telemetry
