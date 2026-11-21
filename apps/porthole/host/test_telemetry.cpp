// Opt-in telemetry (os/telemetry.h): the owner's settings text, what each level keeps, and the two request bodies.
// The anonymous level is the promise to other families: no player, no details, no id that lasts past a month.
#include "host/check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "telemetry.h"

static int checks = 0;
static uint32_t g_utc = 1794817800u;   // 2026-11-16T08:30:00Z
static uint32_t nowUtc() { return g_utc; }
static bool has(const char* body, const char* s) { return strstr(body, s) != nullptr; }

static void settings() {
  using namespace telemetry;
  Config c;
  CHECK(parseConfig("posthog\nhttps://us.i.posthog.com\nphc_abc\n", c));
  CHECK(c.level == PERSONAL && c.format == POSTHOG && !strcmp(c.url, "https://us.i.posthog.com") && !strcmp(c.token, "phc_abc"));
  CHECK(parseConfig("posthog https://us.i.posthog.com phc_abc anonymous\r\n", c) && c.level == ANONYMOUS);   // serial: one line
  CHECK(parseConfig("otlp\nhttps://otel.example/v1/logs\n", c) && c.format == OTLP && !c.token[0]);       // no token: no header
  CHECK(parseConfig("otlp https://otel.example/v1/logs personal", c) && c.level == PERSONAL && !c.token[0]);
  CHECK(parseConfig("off\n", c) && c.level == OFF && !c.url[0]);
  Config kept; parseConfig("posthog https://h.example phc_x", kept); c = kept;
  CHECK(!parseConfig("posthog https://us.i.posthog.com", c));        // PostHog needs the project key
  CHECK(!parseConfig("posthog http://us.i.posthog.com phc_x", c));    // never in the clear
  CHECK(!parseConfig("statsd https://x.example t", c));
  CHECK(!parseConfig("posthog https://h.example a b c", c));          // a fifth word
  CHECK(!parseConfig("", c) && !parseConfig(nullptr, c));
  CHECK(sameConfig(c, kept));                                           // a refusal leaves the old settings
  char h[40];
  CHECK(!strcmp(host(kept, h, sizeof h), "h.example"));
}

static void menuSwitches() {
  using namespace telemetry;
  // The menu's two switches: on from off starts the built-in Porthole project; a level change keeps what is set up.
  const Config on = withLevel(Config{}, ANONYMOUS);
  CHECK(on.level == ANONYMOUS && on.format == POSTHOG && !strcmp(on.url, "https://us.i.posthog.com"));
  CHECK(!strncmp(on.token, "phc_", 4) && strlen(on.token) > 20);
  CHECK(withLevel(on, PERSONAL).level == PERSONAL && sameConfig(withLevel(withLevel(on, PERSONAL), ANONYMOUS), on));
  Config otlpOn; CHECK(parseConfig("otlp https://otel.example/v1/logs anonymous", otlpOn));
  const Config kids = withLevel(otlpOn, PERSONAL);
  CHECK(kids.format == OTLP && !strcmp(kids.url, "https://otel.example/v1/logs") && kids.level == PERSONAL);
  CHECK(withLevel(kids, OFF).level == OFF && !withLevel(kids, OFF).url[0] && withLevel(on, 9).level == PERSONAL);
}

static telemetry::Queue& fresh(const char* text) {
  static telemetry::Queue* q = nullptr;
  delete q; q = new telemetry::Queue();   // 250 KB: never on the stack
  q->begin(nowUtc, "1.2.3");
  telemetry::Config c; CHECK(telemetry::parseConfig(text, c));
  for (int i = 0; i < 8; i++) { c.device[i] = (uint8_t)(i + 1); c.salt[i] = (uint8_t)(0x40 + i); }
  q->configure(c);
  return *q;
}

static void personal() {
  using telemetry::P;
  telemetry::Queue& q = fresh("posthog https://us.i.posthog.com/ phc_key");
  telemetry::install(&q);
  telemetry::emit("device_started", {P("reset", "power-on")});
  telemetry::slot()->player("00aa11bb22cc33dd");
  telemetry::emit("times_cards_answer", {P("fact", "7x8"), P("correct", 1L), P("ms", 2400L), P("note", "say \"hi\"\\")});
  CHECK(q.size() == 2);
  static char body[8192]; int taken = 0;
  const size_t n = q.batch(body, sizeof body, &taken);
  CHECK(n == strlen(body) && taken == 2);
  CHECK(has(body, "{\"api_key\":\"phc_key\",\"batch\":[{\"event\":\"device_started\",\"distinct_id\":\"0102030405060708\""));
  CHECK(has(body, "\"timestamp\":\"2026-11-16T08:30:00Z\""));
  CHECK(has(body, "\"$process_person_profile\":false,\"reset\":\"power-on\"}}"));   // the device's own event: no person
  CHECK(has(body, "\"distinct_id\":\"00aa11bb22cc33dd\""));                         // the kid's: their random id
  CHECK(has(body, "\"player\":\"00aa11bb22cc33dd\",\"$set\":{\"device\":\"0102030405060708\"},\"fact\":\"7x8\",\"correct\":1,\"ms\":2400,\"note\":\"say \\\"hi\\\"\\\\\"}}]}"));
  CHECK(has(body, "\"app_version\":\"1.2.3\""));
  char url[160], auth[120];
  q.endpoint(url, sizeof url); q.auth(auth, sizeof auth);
  CHECK(!strcmp(url, "https://us.i.posthog.com/batch/") && !auth[0]);
  char dev[telemetry::ID]; q.deviceId(dev);
  CHECK(!strcmp(dev, "0102030405060708"));   // what the menu shows is what the events carry
  // A small buffer takes the oldest that fit; drop() lets the next batch start after them.
  const size_t one = q.batch(body, 420, &taken);
  CHECK(taken == 1 && one < 420 && has(body, "device_started") && !has(body, "times_cards") && !strcmp(body + one - 2, "]}"));
  CHECK(q.batch(body, 60, &taken) == 0 && taken == 0);   // not even one fits
  q.drop(1);
  CHECK(q.batch(body, sizeof body, &taken) && taken == 1 && has(body, "times_cards_answer"));
  q.drop(5);
  CHECK(q.size() == 0 && q.batch(body, sizeof body, &taken) == 0);
  telemetry::install(nullptr);
  telemetry::emit("game_opened");   // nothing installed: a no-op
  CHECK(q.size() == 0);
}

static void anonymous() {
  using telemetry::P;
  telemetry::Queue& q = fresh("posthog https://us.i.posthog.com phc_key anonymous");
  q.player("00aa11bb22cc33dd");
  const telemetry::Prop answer[] = {P("fact", "7x8")};
  q.record("times_cards_answer", answer, 1);   // details: never at this level
  q.record("player_selected", answer, 1);
  const telemetry::Prop closed[] = {P("game", "racing"), P("seconds", 300L), P("stars", 4L)};
  q.record("game_closed", closed, 3);
  g_utc += 40u * 86400u;   // December
  q.record("game_opened", closed, 1);
  CHECK(q.size() == 2);
  static char body[8192]; int taken = 0;
  q.batch(body, sizeof body, &taken);
  CHECK(taken == 2 && !has(body, "00aa11bb22cc33dd") && !has(body, "player") && !has(body, "stars") && has(body, "\"seconds\":300"));
  CHECK(!has(body, "0102030405060708"));   // never the device's own id
  const char* a = strstr(body, "\"distinct_id\":\"");
  const char* b = strstr(a + 1, "\"distinct_id\":\"");
  CHECK(a && b && strncmp(a, b, 32));      // a new month, a new id
  g_utc -= 40u * 86400u;
}

static void otlp() {
  using telemetry::P;
  telemetry::Queue& q = fresh("otlp https://otel.example/v1/logs tok");
  q.player("00aa11bb22cc33dd");
  const telemetry::Prop p[] = {P("game", "times-cards"), P("ms", 1500L)};
  q.record("game_opened", p, 2);
  static char body[8192]; int taken = 0;
  const size_t n = q.batch(body, sizeof body, &taken);
  CHECK(taken == 1 && n == strlen(body));
  CHECK(has(body, "{\"resourceLogs\":[{\"resource\":{\"attributes\":[{\"key\":\"service.name\",\"value\":{\"stringValue\":\"porthole\"}}"));
  CHECK(has(body, "\"timeUnixNano\":\"1794817800000000000\""));
  CHECK(has(body, "{\"key\":\"player.id\",\"value\":{\"stringValue\":\"00aa11bb22cc33dd\"}}"));
  CHECK(has(body, "{\"key\":\"ms\",\"value\":{\"intValue\":\"1500\"}}]}]}]}]}"));
  char url[160], auth[120];
  q.endpoint(url, sizeof url); q.auth(auth, sizeof auth);
  CHECK(!strcmp(url, "https://otel.example/v1/logs") && !strcmp(auth, "Bearer tok"));
}

static void queue() {
  using telemetry::P;
  telemetry::Queue& q = fresh("posthog https://h.example phc_k");
  const telemetry::Prop p[] = {P("i", 0L)};
  for (int i = 0; i < telemetry::Queue::CAP + 5; i++) { telemetry::Prop k = p[0]; k.num = i; q.record("e", &k, 1); }
  CHECK(q.size() == telemetry::Queue::CAP);   // full: the oldest went
  static char body[1 << 16]; int taken = 0;
  q.batch(body, sizeof body, &taken);
  CHECK(has(body, "\"i\":5}") && !has(body, "\"i\":4}"));
  telemetry::Config off; telemetry::parseConfig("off", off);
  q.configure(off);   // another level: nothing kept goes out under it
  CHECK(q.size() == 0 && !q.on() && q.batch(body, sizeof body, &taken) == 0);
  q.record("e", p, 1);
  CHECK(q.size() == 0);
  g_utc = 951782400u;   // 2000-02-29: the date math over a leap day
  telemetry::Queue& r = fresh("posthog https://h.example phc_k");
  r.record("e", p, 1);
  r.batch(body, sizeof body, &taken);
  CHECK(has(body, "\"timestamp\":\"2000-02-29T00:00:00Z\""));
  char id[telemetry::ID], other[telemetry::ID];
  r.playerId(0, "Sam", id); r.playerId(1, "Sam", other);
  CHECK(strlen(id) == 16 && strcmp(id, other) && !strstr(id, "Sam"));   // another profile slot: another kid
}

int main() {
  settings();
  menuSwitches();
  personal();
  anonymous();
  otlp();
  queue();
  printf("test_telemetry: %d checks passed\n", checks);
  return 0;
}
