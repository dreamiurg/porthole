// Telemetry: what the kids play, kept for the family that owns the board. On by default (opt-out, 2026-11-21): a board
// that never had a setting starts with "Usage stats" on (the personal level: random board and player ids, never a name);
// a grown-up turns it off on Admin 2/2, and that Off is kept. From the menu, events go to the Porthole project in
// PostHog, with the public ingest key built in (the same key the website ships); the menu shows the board's id so its owner can find it there. Another backend comes from /porthole/telemetry.txt on the SD
// card or the serial command Y (firmware/main.cpp), kept in NVS "sys"/telem:
//   posthog                      or   otlp
//   https://us.i.posthog.com          https://collector.example/v1/logs
//   phc_...                           <bearer token, or an empty line>
// A fourth line "anonymous" picks the anonymous level; without it the level is personal. One line "off" turns it off.
// Personal: every event with the playing kid's random player id (never a name). Anonymous: only the device and game
// events below, no player, no details, and a device id that changes every month.
// Two wire formats, so a family can use their own backend: PostHog's /batch/ events, or OpenTelemetry logs as OTLP/HTTP
// JSON. Games and the shell only call emit(); the firmware owns the queue (firmware/net.cpp) and sends it on wifi.
// Design: docs/superpowers/specs/2026-11-16-telemetry-design.md.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <initializer_list>

namespace telemetry {
enum Level : uint8_t { OFF, ANONYMOUS, PERSONAL };
enum Format : uint8_t { POSTHOG, OTLP };

// "sys"/telem as these bytes, the one key telemetry keeps (the NVS budget has room for one). Never resized: a new
// field goes in a new key. device and salt are random, made when it is turned on and wiped when it is turned off.
struct Config {
  uint8_t version = 1;
  uint8_t level = OFF;
  uint8_t format = POSTHOG;
  uint8_t reserved = 0;
  char url[96] = {0};
  char token[96] = {0};
  uint8_t device[8] = {0};   // sent: the board's id (personal level only)
  uint8_t salt[8] = {0};     // never sent: what makes a kid's player id
  uint32_t fileCrc = 0;      // the SD file last taken: the menu's Off holds until the file changes
};
static_assert(sizeof(Config) == 216, "the \"sys\"/telem blob's layout");

// The SD file's or the serial command's text into out. False (out untouched) for anything but "off" or a format line,
// an https URL and an optional token and level.
bool parseConfig(const char* text, Config& out);
bool sameConfig(const Config& a, const Config& b);   // the owner's part: level, format, url, token
const char* host(const Config& c, char* out, size_t cap);   // "us.i.posthog.com", for the menu
// The menu's switches: OFF turns it off; on, it keeps the backend set up, or starts the built-in Porthole project.
Config withLevel(const Config& now, uint8_t level);

// One property: a key that is a string literal, and a short text or a number.
struct Prop { const char* key; const char* str; int32_t num; };
inline Prop P(const char* k, const char* s) { return {k, s ? s : "", 0}; }
inline Prop P(const char* k, long n) { return {k, nullptr, (int32_t)n}; }

constexpr int MAX_PROPS = 6, TEXT = 24, ID = 17;   // ID: 16 hex digits and the NUL
struct Event {
  const char* name;    // a literal
  uint32_t utc;
  char player[ID];     // "" = nobody picked
  uint8_t n;
  struct { const char* key; bool text; int32_t num; char str[TEXT]; } p[MAX_PROPS];
};

// The events not yet sent, oldest first; a full queue drops the oldest. Main core only: emit(), the shell and the
// firmware's send all run there. Default-constructible for psram::make; begin() before use.
class Queue {
 public:
  static constexpr int CAP = 1024;
  void begin(uint32_t (*utcNow)(), const char* version);
  void configure(const Config& c);
  const Config& config() const { return cfg_; }
  bool on() const { return cfg_.level != OFF; }
  void player(const char* id);   // the kid playing now ("" or nullptr: nobody); kept on every event after
  void record(const char* name, const Prop* p, int n);
  int size() const { return n_; }
  // The oldest events that fit into out as one request body; *taken = how many. 0 when none (or none fit).
  size_t batch(char* out, size_t cap, int* taken) const;
  void drop(int n);   // after the body with them was accepted
  // Where batch() goes, and the Authorization header's value ("" = none).
  void endpoint(char* out, size_t cap) const;
  void auth(char* out, size_t cap) const;
  // A kid's player id, 16 hex digits: the same for the same profile and name while telemetry stays on; a new kid in a
  // deleted kid's place (another name) gets another. Never stored, and the name cannot be read back from it.
  void playerId(int profile, const char* name, char out[ID]) const;
  void deviceId(char out[ID]) const;   // the board's id as the personal level sends it, for the menu
  uint32_t lastSent = 0;            // utc of the last accepted batch, for the menu

 private:
  bool keep(const char* name) const;
  size_t posthog(char* out, size_t cap, int* taken) const;
  size_t otlp(char* out, size_t cap, int* taken) const;
  void distinctId(const Event& e, char out[ID]) const;
  uint32_t (*utc_)() = nullptr;
  const char* version_ = "dev";
  Config cfg_;
  char player_[ID] = {0};
  Event q_[CAP];
  int head_ = 0, n_ = 0;
};

// The queue the firmware installed; nullptr (the sim, the tests, a board with telemetry off): every call is a no-op.
inline Queue*& slot() { static Queue* q = nullptr; return q; }
inline void install(Queue* q) { slot() = q; }
inline bool active() { return slot() && slot()->on(); }
inline void player(const char* id) { if (slot()) slot()->player(id); }
inline void emit(const char* name, std::initializer_list<Prop> p = {}) {
  if (active()) slot()->record(name, p.begin(), (int)p.size());
}
}  // namespace telemetry
