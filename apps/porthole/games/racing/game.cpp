#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace racing {
namespace {
using paint::Leave;
constexpr uint32_t GRIP_LOST_MS = 300, FINISH_MS = 1500;
bool inside(const ui::Box& b, int x, int y) { return x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h; }
// A fresh press that began on the button: holding, sliding in from elsewhere or a carried touch never fires.
bool press(const Input& in, const ui::Box& b) { return in.hit(b.x, b.y, b.w, b.h) && in.pressed && inside(b, in.downX, in.downY); }
bool holding(const Input& in, const ui::Box& b) {
  return in.hit(b.x, b.y, b.w, b.h) && in.down && inside(b, in.downX, in.downY);
}
}  // namespace

void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; rawMs_ = e.ms; ms_ = runs_ = 0;
  seed_ = (e.ms ^ e.nowSec * 2654435761u) | 1;   // the clock too: after a power cycle the boot's ms repeats
  daylight_ = e.daylight; home_ = seenHowTo_ = recenter_ = gripLost_ = false; in_ = {};
  neutral_ = {0, 1000, 0};
  track_ = 0;
  loadKids(e);
  start(race_, TRACK[track_], seed_, raceLevel());
  duo_ = host_ = raced_ = gone_ = ready_ = false; wire_ = {}; peer_ = {}; notice_[0] = 0; wallet_ = e.stars; book_ = e.book;
  visit_ = {}; item_ = {}; rankUp_ = -1; starsGot_ = 0;
  avatars_ = e.avatars; nAvatars_ = e.avatarCount;
  if (e.who) { snprintf(wire_.name, sizeof wire_.name, "%s", e.who->name); wire_.avatar = e.who->avatar; }
  link_.begin(lan::radio());   // on the track cards, says this kid is here (Together)
  if (lan::radio()) lan::radio()->start();
  go(Screen::Tracks);
}

void Game::leave() {
  endTogether();
  if (lan::radio()) lan::radio()->stop();
}

void Game::loadKids(const AppEnter& e) {
  save_ = {}; dirty_ = urgent_ = false; newBest_ = 0; nKids_ = 0; me_ = -1;
  for (int i = 0; i < e.n && nKids_ < MAX_PROFILES; i++) {
    Kid& k = kids_[nKids_];
    k = {};
    snprintf(k.name, sizeof k.name, "%s", e.all[i].name);
    Save s{};
    if (e.saves && e.saves[i].len && loadBlob(e.saves[i].data, e.saves[i].len, s)) {
      memcpy(k.best, s.best, sizeof k.best);
      memcpy(k.stock, s.stock, sizeof k.stock);   // tenths
      k.power = s.power;
    }
    k.me = e.who && e.all[i].id == e.who->id;
    if (k.me) {   // a level saved under an older top races, reads and is saved again at today's top
      me_ = nKids_; save_ = s;
      for (int t = 0; t < TRACKS; t++) {
        save_.level[t] = (int8_t)clampLevel(s.level[t], t);
        dirty_ = dirty_ || save_.level[t] != s.level[t];
      }
    }
    nKids_++;
  }
  seedLevels();
}

// A track the kid has never finished starts one level below their best level elsewhere, inside 0..LEVEL_SEED_MAX: a
// kid good on one track is not handed easy rivals on the next, a new track is still learned first, and one bad day
// elsewhere never makes it a walk. Derived again after every finish until it is raced.
void Game::seedLevels() {
  int top = LEVEL_MIN - 1;   // none raced yet
  for (int t = 0; t < TRACKS; t++) if (save_.best[t].raceMs && save_.level[t] > top) top = save_.level[t];
  const int seed = top - 1 < 0 ? 0 : top - 1 < LEVEL_SEED_MAX ? top - 1 : LEVEL_SEED_MAX;   // a level, not a rung
  for (int t = 0; t < TRACKS; t++) if (!save_.best[t].raceMs) save_.level[t] = (int8_t)clampLevel(seed, t);
}

int Game::raceLevel() const { return levelOn(track_); }
int Game::levelOn(int track) const {
  static_assert(sizeof HARD_LEVEL == TRACKS, "one per track");
  switch (save_.mode) {
    case EASY: return EASY_LEVEL;
    case NORMAL: return 0;
    case HARD: return HARD_LEVEL[track];
    default: return save_.level[track];
  }
}

// Every finish, any place, any mode, together too, counts toward the kid's rank (garage.h); the Results banner says when
// it reaches the next one.
void Game::countFinish() {
  const int before = garage::rank(save_.finishes);
  if (save_.finishes < 255) { save_.finishes++; dirty_ = true; }
  rankUp_ = (int8_t)(garage::rank(save_.finishes) > before ? garage::rank(save_.finishes) : -1);
}

// Once per finish: the kid's own record for this track, kept only when it improves, and the rivals' next level. A
// race with parts counts on the Tuned board (every time, with its Power); one with Power 0 on both boards. "New best"
// is said of the board of the class just raced.
// TODO: leaving a race (Restart/Tracks/Home) never counts, so quitting one being lost never lowers the level;
// deliberate for now, revisit if kids learn to quit instead of finishing.
void Game::recordFinish() {
  countFinish();
  starsGot_ = (uint8_t)purse::payFinish(book_, wallet_, pay_, now_, false);
  int power = 0;
  for (uint8_t t : race_.tier) power += t;
  const uint32_t ms = race_.car[PLAYER].finishMs, lap = bestLap(race_);
  uint16_t& stock = save_.stock[track_];
  prevBestMs_ = power ? save_.best[track_].raceMs : stock * 100u;
  const uint8_t all = record(save_.best[track_], ms, lap);
  if (all & BEST_RACE) save_.power = (uint16_t)((save_.power & ~(15u << 4 * track_)) | (unsigned)power << 4 * track_);
  const bool stockBest = !power && (!stock || tenths(ms) < stock);   // the Stock board keeps races, not laps
  if (stockBest) stock = tenths(ms);
  newBest_ = power ? all : stockBest ? BEST_RACE : 0;
  dirty_ = dirty_ || all || newBest_;
  if (save_.mode == AUTO) {   // a fixed setting races at its own level and leaves the adaptive ones alone
    const int8_t level = (int8_t)clampLevel(nextLevel(race_.level, track_, race_.place, runaway(race_)), track_);
    dirty_ = dirty_ || level != save_.level[track_];
    save_.level[track_] = level;
  }
  seedLevels();
  if (me_ >= 0) { Kid& k = kids_[me_]; k.best[track_] = save_.best[track_]; k.stock[track_] = save_.stock[track_]; k.power = save_.power; }
}

// This track's leaderboard: every kid with a time, fastest race first (ties: the faster lap, then profile order).
// stock: the Stock board (races run with Power 0); otherwise Tuned, every time with the Power it was set with.
int Game::board(paint::Row out[MAX_PROFILES], bool stock) const {
  int n = 0;
  for (int i = 0; i < nKids_; i++) {
    const Best b = stock ? Best{kids_[i].stock[track_] * 100u, 0} : kids_[i].best[track_];
    if (!b.raceMs) continue;
    const uint8_t power = stock ? paint::NO_POWER : (uint8_t)(kids_[i].power >> 4 * track_ & 15);
    paint::Row row = {kids_[i].name, b.raceMs, b.lapMs, kids_[i].me, power};
    int j = n++;
    for (; j > 0 && (out[j - 1].raceMs > row.raceMs || (out[j - 1].raceMs == row.raceMs && out[j - 1].lapMs > row.lapMs)); j--) out[j] = out[j - 1];
    out[j] = row;
  }
  return n;
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  seal(save_);
  *data = &save_; *len = sizeof save_;   // the shell copies it out before the next update
  dirty_ = urgent_ = false;
  return true;
}

void Game::go(Screen s) {
  screen_ = s;
  if (s == Screen::Calibrate) { calibration_ = {}; gripSet_ = false; }
  else if (s != Screen::Race) recenter_ = false;   // a recenter only ever returns to the race it paused
  gate_.shown(ms_); hold_ = {};
  boost_ = rocket_ = gripDone_ = false;
  if (s != Screen::Tracks) ready_ = false;   // Race! is only ever asked on the cards
  if (s == Screen::Results) resultsMs_ = ms_;
  hintMs_ = 0;
  badGripMs_ = 0;
}

void Game::startRace() {
  uint8_t kit[PARTS];
  garage::kit(save_, kit);
  start(race_, TRACK[track_], seed_ + ++runs_, raceLevel(), kit);   // every run is fresh: inventory, timers and rivals reset
  fresh();
}
void Game::fresh() {
  steer_ = 0; resumeMs_ = stepMs_ = finishMs_ = 0; gripLost_ = recenter_ = recorded_ = false; newBest_ = 0;
  go(Screen::Race);
}

void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  const int32_t elapsed = (int32_t)(ms - rawMs_);
  const uint32_t dt = elapsed <= 0 ? 0 : (uint32_t)elapsed > MAX_FRAME_MS ? MAX_FRAME_MS : (uint32_t)elapsed;
  rawMs_ = ms; now_ = nowSec; ms_ += dt; in_ = in;
  gate_.filter(in_, ms_);
  togetherTick(dt);
  switch (screen_) {
    case Screen::Tracks: updateTracks(); break;
    case Screen::HowTo: updateHowTo(); break;
    case Screen::Calibrate: updateCalibrate(); break;
    case Screen::Race: updateRace(dt); break;
    case Screen::Pause: updatePause(); break;
    case Screen::Confirm: updateConfirm(); break;
    case Screen::Results: updateResults(); break;
    case Screen::Board: updateBoard(); break;
    case Screen::Lobby: updateLobby(); break;
    case Screen::Garage: updateGarage(); break;
    case Screen::Shop: updateShop(); break;
    case Screen::Buy: updateBuy(); break;
  }
}

void Game::updateTracks() {
  if (tracksTogether()) return;
  if (ui::clicked(in_, paint::MODE_CHIP)) {   // saved at once: a power cut right after still keeps the kid's pick
    save_.mode = (uint8_t)((save_.mode + 1) % MODES);
    dirty_ = urgent_ = true;
    return;
  }
  for (int i = 0; i < TRACKS; i++)
    if (ui::clicked(in_, paint::CARD[i])) { track_ = i; go(seenHowTo_ ? Screen::Calibrate : Screen::HowTo); return; }
  if (ui::clicked(in_, paint::TIMES)) go(Screen::Board);
  else if (ui::clicked(in_, paint::CAR_BTN)) { visit_ = garage::begin(save_); go(Screen::Garage); }
  else if (ui::clicked(in_, paint::HOME)) home_ = true;
}

// The garage: the kid's car, its Power and the Stock switch; Looks and Power open the item pages. Leaving it keeps
// this visit's purchases for good (no more Undo).
void Game::updateGarage() {
  using garage::Item;
  if (ui::clicked(in_, paint::STOCK_SW)) { save_.stockOn = !save_.stockOn; dirtyNow(); }
  else if (ui::clicked(in_, paint::LOOKS_BTN)) { item_ = Item{garage::PAINT, (uint8_t)save_.paint}; go(Screen::Shop); }
  else if (ui::clicked(in_, paint::POWER_BTN)) { item_ = Item{garage::PART, 0}; go(Screen::Shop); }
  else if (ui::clicked(in_, paint::GARAGE_BACK)) { visit_ = {}; go(Screen::Tracks); }
}
void Game::shopStep(int by) {
  const int n = garage::count(item_.group);
  item_.index = (uint8_t)((item_.index + n + by) % n);
  hintMs_ = 0;
}
// One item at a time: < > browse its group, the chip turns Paint and Stripes over; Buy asks on its own page, Undo
// takes back what this visit bought, Equip puts an owned look on. Too few stars: a gold hint for a moment, no sound.
void Game::updateShop() {
  using garage::State;
  const State st = garage::state(save_, purse::balance(wallet_), item_);
  const bool undo = garage::canUndo(save_, item_, visit_);
  if (ui::clicked(in_, paint::SHOP_BACK)) { go(Screen::Garage); return; }
  if (item_.group != garage::PART && ui::clicked(in_, paint::GROUP_CHIP)) {
    item_.group = item_.group == garage::PAINT ? garage::STRIPE : garage::PAINT;
    item_.index = item_.group == garage::PAINT ? save_.paint : save_.stripe;
    hintMs_ = 0;
    return;
  }
  if (ui::clicked(in_, paint::PREV_ITEM)) { shopStep(-1); return; }
  if (ui::clicked(in_, paint::NEXT_ITEM)) { shopStep(1); return; }
  if (undo && ui::clicked(in_, paint::SPLIT_UNDO)) { garage::undo(save_, wallet_, item_, visit_); dirtyNow(); return; }
  if (!ui::clicked(in_, undo ? paint::SPLIT_MAIN : paint::MAIN)) return;
  if (st == State::Buy) go(Screen::Buy);
  else if (st == State::Short) hintMs_ = ms_ + paint::HINT_MS;
  else if (st == State::Equip) { garage::wear(save_, item_); dirtyNow(); }
}
void Game::updateBuy() {
  if (ui::clicked(in_, paint::BUY_YES)) {
    if (garage::buy(save_, wallet_, item_, visit_)) dirtyNow();
    go(Screen::Shop);
  } else if (ui::clicked(in_, paint::BUY_NO)) go(Screen::Shop);
}

// Every kid's best times, one track at a time from the last one picked; the arrows go round all four.
void Game::updateBoard() {
  if (ui::clicked(in_, paint::TUNED_CHIP)) boardStock_ = false;
  else if (ui::clicked(in_, paint::STOCK_CHIP)) boardStock_ = true;
  else if (ui::clicked(in_, paint::PREV_TRACK)) track_ = (track_ + TRACKS - 1) % TRACKS;
  else if (ui::clicked(in_, paint::NEXT_TRACK)) track_ = (track_ + 1) % TRACKS;
  else if (ui::clicked(in_, paint::BOARD_BACK)) go(Screen::Tracks);
}

void Game::updateHowTo() {
  if (ui::clicked(in_, paint::BACK)) go(Screen::Tracks);
  else if (ui::clicked(in_, paint::NEXT)) { seenHowTo_ = true; go(Screen::Calibrate); }
}

// The first grip held steady here becomes straight ahead at once: the preview car centers on it, turning from it moves
// the car (and stays turned), and Go keeps it. Laying the puck down lets the next steady grip replace it. Until a
// steady grip is seen, the preview follows the previous one.
void Game::updateCalibrate() {
  calibration_.add(gravity(), rawMs_);
  tilt::Grav n{};
  if (!controls::upright(gravity())) gripSet_ = false;
  else if (!gripSet_ && calibration_.get(&n)) { grip_ = n; gripSet_ = true; }
  steer_ = controls::steer(gravity(), gripSet_ ? grip_ : neutral_);
  if (ui::clicked(in_, paint::BACK)) {   // clears recenter_
    if (recenter_) { go(Screen::Pause); return; }
    endTogether();
    go(Screen::Tracks);
    return;
  }
  if (gripDone_) { awaitStart(); return; }
  if (!ui::clicked(in_, paint::NEXT) || !gripReady()) return;
  neutral_ = grip_;
  if (!recenter_ && duo_) { gripDone_ = true; awaitStart(); return; }   // set: now the other kid's grip
  if (!recenter_) { startRace(); return; }
  recenter_ = false; gripLost_ = false; resumeMs_ = RESUME_MS; steer_ = 0;   // the race waits a beat, then goes on
  go(Screen::Race);
}

// A flat or implausible pose for GRIP_LOST_MS stops the race: no unstable steering, no driving blind. Leaning back or
// turning hard is still racing (controls::held); the grip page alone asks for a comfortable upright hold.
bool Game::gripOk(uint32_t dt) {
  if (controls::held(gravity())) { badGripMs_ = 0; return true; }
  badGripMs_ += dt;
  if (badGripMs_ < GRIP_LOST_MS) return true;
  gripLost_ = true;
  go(Screen::Pause);
  return false;
}

void Game::finishLine(uint32_t dt) {   // a moment on the line to see "Finish!" before the results
  if (!recorded_) { recorded_ = true; if (duo_) jointFinish(); else recordFinish(); }
  finishMs_ += dt;
  if (finishMs_ >= FINISH_MS) go(Screen::Results);
}

void Game::updateRace(uint32_t dt) {
  if (race_.phase == Phase::Finished) { finishLine(dt); return; }
  if (!gripOk(dt)) return;
  // Together: a few pauses a race, and always one while the other kid is paused (a way to the menu).
  hold_.step(holding(in_, paint::LAP) && (!duo_ || pausesLeft_ || peerPaused_), ms_);
  if (hold_.progress() >= 1) { gripLost_ = false; if (duo_ && pausesLeft_) pausesLeft_--; go(Screen::Pause); return; }
  boost_ = boost_ || press(in_, paint::BOOST_BTN);
  rocket_ = rocket_ || press(in_, paint::ROCKET_BTN);
  if (peerHold()) return;
  if (resumeMs_) { resumeMs_ = resumeMs_ > dt ? resumeMs_ - dt : 0; boost_ = rocket_ = false; return; }
  simulate(dt);
}

void Game::simulate(uint32_t dt) {
  const float target = controls::steer(gravity(), neutral_);
  stepMs_ += dt;
  while (stepMs_ >= STEP_MS && race_.phase != Phase::Finished) {
    stepMs_ -= STEP_MS;
    steer_ += (target - steer_) * (STEP_MS / (100.0f + STEP_MS));   // ~100 ms smoothing
    const bool racing = race_.phase == Phase::Racing;
    Controls c;
    c.steer = steer_;
    c.boost = racing && boost_;
    c.rocket = racing && rocket_;
    boost_ = rocket_ = false;   // spent or not, a press is used once; one before GO is simply dropped
    race_.car[PLAYER].top = windTop();
    step(race_, c);
  }
}

void Game::updatePause() {
  if (ui::clicked(in_, paint::RESUME) && controls::held(gravity())) {
    gripLost_ = false; resumeMs_ = RESUME_MS;
    go(Screen::Race);   // the exact paused state, down to the partial step
    return;
  }
  if (ui::clicked(in_, paint::RECENTER)) { recenter_ = true; go(Screen::Calibrate); return; }
  const ui::Box* boxes[] = {&paint::RESTART, &paint::TRACKS_BTN, &paint::HOME_BTN};
  for (int i = duo_ ? 1 : 0; i < 3; i++)   // together, one kid can't restart both
    if (ui::clicked(in_, *boxes[i])) { leave_ = (Leave)i; go(Screen::Confirm); return; }
}

void Game::updateConfirm() {
  if (ui::clicked(in_, paint::NO)) { go(Screen::Pause); return; }
  if (!ui::clicked(in_, paint::YES)) return;
  if (leave_ == Leave::Restart) { startRace(); return; }
  endTogether();
  if (leave_ == Leave::Tracks) go(Screen::Tracks);
  else home_ = true;
}

void Game::updateResults() {
  if (ui::clicked(in_, paint::REPLAY)) {
    if (duo_ && link_.paired()) { raced_ = false; go(host_ ? Screen::Calibrate : Screen::Lobby); return; }   // Again
    endTogether();
    startRace();
  } else if (ui::clicked(in_, paint::RESULT_TRACKS)) { endTogether(); go(Screen::Tracks); }
  else if (ui::clicked(in_, paint::RESULT_HOME)) { endTogether(); home_ = true; }
}

void Game::render() {
  paint::View v;
  v.page = screen_;
  v.race = &race_; v.track = track_; v.ms = ms_; v.steer = steer_;
  v.calibration = calibration_.progress(); v.hold = (uint16_t)(hold_.progress() * 1000); v.resumeMs = resumeMs_;
  v.gripReady = gripReady(); v.gripUpright = controls::upright(gravity()); v.gripHeld = controls::held(gravity());
  v.gripLost = gripLost_; v.leave = leave_;
  paint::Row rows[MAX_PROFILES];
  int power = 0;
  for (uint8_t t : race_.tier) power += t;
  const bool stock = screen_ == Screen::Board ? boardStock_ : screen_ == Screen::Calibrate ? raceStock() : power == 0;
  v.board = rows; v.boardRows = board(rows, stock); v.boardStock = stock;
  v.paint = duo_ ? 0 : save_.paint; v.stripe = duo_ ? 0 : save_.stripe;   // together, both cars race stock
  v.shop = {&save_, purse::balance(wallet_), item_, &visit_, hintMs_ && (int32_t)(hintMs_ - ms_) > 0,
            garage::anyToBuy(save_, purse::balance(wallet_))};
  v.earned = starsGot_;
  v.rankUp = rankUp_ >= 0 && ms_ - resultsMs_ < paint::BANNER_MS ? rankUp_ : -1; v.newBest = newBest_; v.prevBestMs = prevBestMs_; v.mode = save_.mode % MODES;
  duoView(v.duo);
  if (notice_[0] && (int32_t)(noticeMs_ - ms_) > 0) v.notice = notice_;
  paint::draw(v);
}

const char* Game::screenName() const {
  static const char* const N[] = {"race_tracks", "race_howto", "race_calibrate", "race_play", "race_pause", "race_confirm",
                            "race_results", "race_board", "race_lobby", "race_garage", "race_shop", "race_buy"};
  if (screen_ == Screen::Race && race_.phase == Phase::Countdown) return "race_countdown";
  return N[(int)screen_];
}

void Game::debugPrint() {
  const Car& p = race_.car[PLAYER];
  printf("[racing screen=%s track=%d phase=%d lap=%d place=%u boost=%u rocket=%u speed=%d x=%d dist=%d]\n", screenName(),
         track_, (int)race_.phase, lapOf(race_, PLAYER), race_.place, race_.inventory[BOOST], race_.inventory[ROCKET],
         (int)lroundf(p.speed * 1000), (int)lroundf(p.x * 1000), (int)lroundf(p.dist));
  printf("race_ms=%u finished=%d finish_ms=%u cars_done=%u jumps=%u pickups=%u fired=%u hits=%u impacts=%u\n",
         (unsigned)race_.raceMs, p.finished, (unsigned)p.finishMs, race_.finishedCount, race_.jumps, race_.pickups,
         race_.rocketsFired, race_.rocketHits, race_.impacts);
  tilt::Grav n{};
  const Best& b = save_.best[track_];
  printf("best_race=%u best_lap=%u new_best=%d kids=%d level=%d mode=%s race_level=%d\n", (unsigned)b.raceMs,
         (unsigned)b.lapMs, newBest_, nKids_, (int)save_.level[track_], paint::MODE_NAME[save_.mode % MODES], raceLevel());
  int t[PARTS];
  for (int i = 0; i < PARTS; i++) t[i] = garage::tier(save_, i);
  printf("stars=%u power=%d race_power=%d finishes=%u rank=%s paint=%u stripe=%u tiers=%d%d%d%d%d stock=%u looks=%u "
         "item=%u:%u undo=%d board_stock=%d earned=%u\n", (unsigned)purse::balance(wallet_), garage::power(save_),
         garage::racePower(save_), save_.finishes, garage::RANK_NAME[garage::rank(save_.finishes)], (unsigned)save_.paint,
         (unsigned)save_.stripe, t[0], t[1], t[2], t[3], t[4], (unsigned)save_.stockOn, (unsigned)save_.looks, item_.group,
         item_.index, garage::canUndo(save_, item_, visit_), boardStock_, starsGot_);
  printf("steer=%d grip_lost=%d resume=%u ready=%d calibration=%u neutral=%d,%d,%d\n", (int)lroundf(steer_ * 1000),
         gripLost_, (unsigned)resumeMs_, calibration_.get(&n), calibration_.progress(), neutral_.x, neutral_.y, neutral_.z);
}

// Test fixtures only (reachability, not natural play): they start from the current upright grip.
void Game::debugFixture(const char* cmd) {
  if (controls::upright(gravity())) neutral_ = gravity();
  if (!duo_ || !raced_) startRace();   // together: this race, both devices' own fixture
  race_.phase = Phase::Racing;
  race_.ms = COUNTDOWN_MS;
  const float line = LAPS * race_.track->length;
  if (!strcmp(cmd, "lastlap")) race_.car[PLAYER].dist = line - 60;
  if (!strcmp(cmd, "lastplace")) {   // every rival already home: the player still finishes, fourth
    for (int i = 1; i < CARS; i++) { race_.car[i].dist = line + i; race_.car[i].finished = true; race_.car[i].finishMs = 1000u * (unsigned)i; }
    race_.finishedCount = CARS - 1;
    race_.car[PLAYER].dist = line - 60;
  }
}

void Game::debugCmd(const char* cmd) {
  if (!strncmp(cmd, "track ", 6)) { track_ = abs(atoi(cmd + 6)) % TRACKS; return; }
  if (!strncmp(cmd, "level ", 6)) {   // the next race on this track (a finish moves it on as usual)
    const int n = atoi(cmd + 6);
    save_.level[track_] = (int8_t)clampLevel(n, track_);
    dirty_ = true;   // kept even when the next finish leaves it where it is
    return;
  }
  if (!strncmp(cmd, "stars ", 6)) { purse::fixture(wallet_, (uint32_t)atoi(cmd + 6)); return; }
  if (!strncmp(cmd, "finishes ", 9)) { const int n = atoi(cmd + 9); save_.finishes = (uint8_t)(n < 0 ? 0 : n > 255 ? 255 : n); dirty_ = true; return; }
  if (!strcmp(cmd, "tracks")) { go(Screen::Tracks); return; }
  if (!strcmp(cmd, "calibrate")) { go(Screen::Calibrate); return; }
  if (!strcmp(cmd, "go") || !strcmp(cmd, "lastlap") || !strcmp(cmd, "lastplace")) debugFixture(cmd);
}
}  // namespace racing
