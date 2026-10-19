#include "rules.h"
#include <math.h>

namespace starfighter {
static float clamp(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static uint32_t random(Run& r) {
  r.seed ^= r.seed << 13;
  r.seed ^= r.seed >> 17;
  r.seed ^= r.seed << 5;
  return r.seed;
}
static bool hit(float x, float y, float xx, float yy, float radius) {
  const float dx = x - xx, dy = y - yy;
  return dx * dx + dy * dy <= radius * radius;
}
bool playing(Stage s) { return s != Stage::Victory && s != Stage::Defeat; }
bool aligned(const Run& r) { return fabsf(r.player.x - r.targetX) <= TARGET_TOLERANCE; }
const char* stageName(Stage s) {
  static const char* const names[] = {"squadron", "waveclear", "trenchentry", "trench", "target", "torpedo", "explosion", "victory", "defeat"};
  return names[static_cast<unsigned>(s)];
}
static void effect(Run& r, float x, float y, bool large) {
  for (auto& e : r.effects) {
    if (e.active) continue;
    e = {x, y, large ? 720U : 360U, large, true};
    return;
  }
}
static void clearHazards(Run& r) {
  for (auto& b : r.bolts) b.active = false;
  for (auto& o : r.obstacles) o.active = false;
}
static void enter(Run& r, Stage stage) {
  r.stage = stage;
  r.stageMs = 0;
  clearHazards(r);
}
void restart(Run& r, uint32_t seed) {
  r = Run{};
  r.seed = seed ? seed : 0xA341316CU;
}
static void hurt(Run& r) {
  if (r.player.invulnerableMs || !r.lives) return;
  --r.lives;
  r.player.invulnerableMs = INVULNERABLE_MS;
  effect(r, r.player.x, r.player.y, false);
  if (!r.lives) enter(r, Stage::Defeat);
}
static void steer(Run& r, float bank) {
  if (!isfinite(bank)) bank = 0;
  bank = clamp(bank, -1, 1);
  if (fabsf(bank) < BANK_DEAD) bank = 0;
  r.player.bank += (bank - r.player.bank) * 0.18f;
  const bool trench = r.stage == Stage::Trench || r.stage == Stage::Target;
  const float lo = trench ? 145 : PLAYER_MIN_X, hi = trench ? 335 : PLAYER_MAX_X;
  r.player.x = clamp(r.player.x + bank * PLAYER_SPEED * DT, lo, hi);
}
static int freeBolts(const Run& r) {
  int count = 0;
  for (const auto& b : r.bolts) if (!b.active) ++count;
  return count;
}
static void bolt(Run& r, const Bolt& shot) {
  for (auto& b : r.bolts) {
    if (b.active) continue;
    b = shot;
    return;
  }
}
static void volley(Run& r) {
  if (freeBolts(r) < 2) return;
  ++r.volleys;
  for (int side = -1; side <= 1; side += 2) {
    bolt(r, {r.player.x + side * CANNON_X, r.player.y - CANNON_Y, -side * LASER_CONVERGE_SPEED,
             -PLAYER_BOLT_SPEED, r.volleys, false, true, r.player.x + side * 12});
  }
}
static void enemyFire(Run& r, float x, float y) {
  const float dx = r.player.x - x, dy = r.player.y - y;
  const float length = sqrtf(dx * dx + dy * dy);
  if (length < 1 || dy < 35) return;
  bolt(r, {x, y + 12, dx / length * ENEMY_BOLT_SPEED, dy / length * ENEMY_BOLT_SPEED, 0, true, true});
}
static uint32_t fireWait(Run& r, bool first) {
  return (first ? 4500 : 10000) + random(r) % (first ? 6500 : 7000) - r.wave * WAVE_FIRE_GAIN_MS;
}
static void spawnGroup(Run& r) {
  const Formation f = static_cast<Formation>((r.wave + r.groupsSpawned) % 3);
  int count = 0;
  for (auto& e : r.enemies) {
    if (e.active) continue;
    const float x = 150 + count * 60.0f;
    const float y = ENEMY_MIN_Y + (fabsf(count - 1.5f) - 0.5f) * 28;
    e = {x, y, x, random(r) % 628 / 100.0f, fireWait(r, true), 0,
         static_cast<uint8_t>(f == Formation::Heavy && count == 1 ? 2 : 1), r.groupsSpawned, f, true, false};
    e.heavy = e.hp > 1;
    if (++count == SHIPS_PER_GROUP) break;
  }
  ++r.groupsSpawned;
}
static bool enemiesAlive(const Run& r) {
  for (const auto& e : r.enemies) if (e.active) return true;
  return false;
}
static void moveEnemies(Run& r) {
  for (auto& e : r.enemies) {
    if (!e.active) continue;
    e.phase += DT * (e.formation == Formation::Swoop ? 1.15f : 0.55f);
    e.x = clamp(e.homeX + sinf(e.phase) * (e.formation == Formation::Swoop ? 52 : 22), 122, 358);
    e.y += DT * (e.formation == Formation::Swoop ? 19 : 13) * (1 + r.wave * WAVE_SPEED_GAIN);
    if (e.y > 405) e.y = ENEMY_MIN_Y;  // A missed fighter loops around; only shooting clears it.
    if (e.fireMs > STEP_MS) e.fireMs -= STEP_MS;
    else {
      enemyFire(r, e.x, e.y);
      e.fireMs = fireWait(r, false);
    }
    if (hit(e.x, e.y, r.player.x, r.player.y, ENEMY_RAM_RADIUS)) hurt(r);
    if (r.stage == Stage::Defeat) return;
  }
}
static void damageEnemy(Run& r, Enemy& e, Bolt& b) {
  b.active = false;
  if (e.lastVolley == b.volley) return;
  e.lastVolley = b.volley;
  --e.hp;
  effect(r, e.x, e.y, !e.hp);
  if (e.hp) return;
  e.active = false;
  r.score += e.heavy ? 150 : 100;
}
static bool enemyHit(const Enemy& e, const Bolt& b) {
  return fabsf(b.x - e.x) <= (e.heavy ? HEAVY_HALF_W : ENEMY_HALF_W) && fabsf(b.y - e.y) <= (e.heavy ? HEAVY_HALF_H : ENEMY_HALF_H);
}
static void playerBolt(Run& r, Bolt& b) {
  for (auto& e : r.enemies) {
    if (!e.active || !enemyHit(e, b)) continue;
    damageEnemy(r, e, b);
    return;
  }
  for (auto& o : r.obstacles) {
    if (!o.active || !(fabsf(b.x - o.x) <= TURRET_HALF_W && fabsf(b.y - o.y) <= TURRET_HALF_H)) continue;
    o.active = b.active = false;
    effect(r, o.x, o.y, true);
    r.score += 50;
    return;
  }
}
static void moveBolt(Bolt& b) {
  b.x += b.vx * DT;
  b.y += b.vy * DT;
  if (b.enemy || !b.vx) return;
  if ((b.vx > 0 && b.x >= b.aimX) || (b.vx < 0 && b.x <= b.aimX)) {
    b.x = b.aimX;
    b.vx = 0;
  }
}
static void moveBolts(Run& r) {
  for (auto& b : r.bolts) {
    if (!b.active) continue;
    moveBolt(b);
    if (b.x < 30 || b.x > 450 || b.y < 35 || b.y > 438) b.active = false;
    if (!b.active) continue;
    if (!b.enemy) playerBolt(r, b);
    else if (hit(b.x, b.y, r.player.x, r.player.y, PLAYER_RADIUS + 4)) {
      b.active = false;
      hurt(r);
    }
  }
}
static void age(Run& r) {
  r.ms += STEP_MS;
  r.stageMs += STEP_MS;
  auto& protection = r.player.invulnerableMs;
  protection = protection > STEP_MS ? protection - STEP_MS : 0;
  for (auto& e : r.effects) {
    if (!e.active) continue;
    if (e.ms <= STEP_MS) e.active = false;
    else e.ms -= STEP_MS;
  }
}
static void reinforce(Run& r) {
  if (r.groupsSpawned >= GROUPS_PER_WAVE) return;
  if (enemiesAlive(r)) { r.groupClearMs = 0; return; }
  r.groupClearMs += STEP_MS;
  if (r.groupsSpawned && r.groupClearMs < GROUP_GAP_MS) return;
  spawnGroup(r);
  r.groupClearMs = 0;
}
static void squadron(Run& r, bool tap) {
  reinforce(r);
  if (tap) volley(r);
  moveEnemies(r);
  moveBolts(r);
  if (r.stage == Stage::Defeat) return;
  if (r.groupsSpawned == GROUPS_PER_WAVE && !enemiesAlive(r)) enter(r, Stage::WaveClear);
}
static void nextWave(Run& r) {
  if (r.stageMs < WAVE_CLEAR_MS) return;
  if (++r.wave == WAVES) enter(r, Stage::TrenchEntry);
  else {
    r.groupsSpawned = 0;
    r.groupClearMs = 0;
    enter(r, Stage::Squadron);
  }
}
static void spawnObstacle(Run& r) {
  const int lanes[] = {0, 2, 0, 1, 2, 0, 2, 1};
  const int index = (r.stageMs / OBSTACLE_MS - 1) % 8;
  const float x = 160 + lanes[index] * 80.0f;
  for (auto& o : r.obstacles) {
    if (o.active) continue;
    o = {x, 95, true};
    enemyFire(r, x, 110);
    return;
  }
}
static void moveObstacles(Run& r) {
  for (auto& o : r.obstacles) {
    if (!o.active) continue;
    o.y += 68 * DT;
    if (o.y > 420) o.active = false;
    else if (hit(o.x, o.y, r.player.x, r.player.y, TURRET_RAM_RADIUS)) {
      o.active = false;
      hurt(r);
    }
  }
}
static void trench(Run& r, bool tap) {
  r.obstacleMs += STEP_MS;
  if (r.obstacleMs >= OBSTACLE_MS) {
    r.obstacleMs = 0;
    spawnObstacle(r);
  }
  if (tap) volley(r);
  moveObstacles(r);
  moveBolts(r);
  if (r.stage != Stage::Defeat && r.stageMs >= TRENCH_MS) enter(r, Stage::Target);
}
static void target(Run& r, bool tap) {
  r.targetX = 240 + sinf(r.stageMs / 1700.0f) * 44;
  if (tap && aligned(r)) {
    r.score += 1000 + r.lives * 250;
    enter(r, Stage::Torpedo);
  } else if (r.stageMs >= TARGET_PASS_MS) {
    ++r.targetPasses;
    r.stageMs = 0;
  }
}
static void cinematic(Run& r) {
  if (r.stage == Stage::TrenchEntry && r.stageMs >= TRENCH_ENTRY_MS) enter(r, Stage::Trench);
  else if (r.stage == Stage::Torpedo && r.stageMs >= TORPEDO_MS) enter(r, Stage::Explosion);
  else if (r.stage == Stage::Explosion && r.stageMs >= EXPLOSION_MS) enter(r, Stage::Victory);
}
void step(Run& r, float bank, bool tap) {
  if (!playing(r.stage)) return;
  age(r);
  if (r.stage != Stage::Torpedo) steer(r, bank);
  switch (r.stage) {
    case Stage::Squadron: squadron(r, tap); break;
    case Stage::WaveClear: nextWave(r); break;
    case Stage::Trench: trench(r, tap); break;
    case Stage::Target: target(r, tap); break;
    default: cinematic(r); break;
  }
}
}  // namespace starfighter
