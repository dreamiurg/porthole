// App-level regressions. The required player-led emulator tour is separate evidence.
#include <algorithm>
#include <assert.h>
#include <initializer_list>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "games/toy-town-life/art.h"
#include "games/toy-town-life/content.h"
#include "games/toy-town-life/game.h"
#include "gfx565.h"
using namespace toytown;
namespace {
constexpr uint32_t NOW = 1790000000;
const ui::Box ACTIONS[] = {{24,105,54,24},{82,105,54,24},{49,131,62,22}};
constexpr ui::Box BACK = {68,8,24,22};
struct Player {
  Game game;
  Profile profiles[2] = {{3,"Reader",0,8,1},{1,"Other",1,9,1}};
  SaveSlot saves[2]{};
  InputTracker touch;
  uint32_t ms = 0, now = NOW;
  void enter(State* saved = nullptr, int profile = 0) {
    if (saved) { seal(*saved); saves[profile] = {saved, sizeof *saved}; }
    game.enter({&profiles[profile], profiles, saves, 2, now, ms, true});
  }
  void frame(bool down = false, int x = 0, int y = 0, uint32_t delta = 20) {
    ms += delta; game.update(now, ms, touch.step(down, x, y, ms));
  }
  void press(const ui::Box& b, uint32_t held = 40) {
    const int x = b.x + b.w / 2, y = b.y + b.h / 2;
    frame(true,x,y); frame(true,x,y,held); frame(false,x,y);
  }
  void tap(const ui::Box& b) { frame(false,0,0,500); press(b); }
  void action(int i) { tap(ACTIONS[i]); }
  void start(State* saved = nullptr) { enter(saved); action(0); }
  bool on(uint8_t page) const { return game.state().page == page; }
  State snapshot() {
    const void* data = nullptr; size_t bytes = 0;
    game.leave(); assert(game.takeSave(&data,&bytes,true));
    State saved{}; assert(load(data,bytes,saved)); return saved;
  }
};
State fixture(uint8_t page, uint8_t place = SCHOOL) {
  State s = fresh(); start(s,place); s.page = page; s.resumePage = page;
  s.schoolOutcome = 2; s.variant = 1; s.cart[1] = s.cart[2] = 1;
  return s;
}
void identityAndSave() {
  Player p; p.enter();
  assert(!strcmp(p.game.name(),"Toy Town") && !strcmp(p.game.store(),"toytown"));
  assert(p.game.surface() == SURFACE_RGB565 && p.game.tint() == TINT_DAY);
  assert(!p.game.asleep() && !p.game.soundOn(0) && !p.game.soundOn(0xffffffffu));
  assert(p.game.icon().w > 0);
  const void* data = nullptr; size_t bytes = 0;
  assert(p.game.saveUrgent() && !p.game.takeSave(&data,&bytes,false));
  assert(p.game.saveUrgent() && p.game.takeSave(&data,&bytes,true));
  assert(!p.game.saveUrgent() && !p.game.takeSave(&data,&bytes,true));
  p.action(0); p.action(1); p.action(0); p.action(1); p.action(0);
  State chosen = p.snapshot(); assert(chosen.doll == 1 && chosen.hair == 1);
  Player reopened; reopened.start(&chosen);
  assert(reopened.on(STYLE) && reopened.game.state().doll == 1 && reopened.game.state().hair == 1);
  reopened.tap(BACK); reopened.action(2);
  assert(reopened.game.wantsHome() && !reopened.game.wantsHome());
  State other = fresh(); other.doll = 3; seal(other);
  p.saves[1] = {&other,sizeof other}; p.enter(nullptr,1); p.action(0);
  assert(p.game.state().doll == 3);
  p.enter(); p.action(0); assert(p.game.state().doll == 0); // profile ids are not slot indexes
}
void inputLifecycle() {
  Player p; p.enter(); p.press(ACTIONS[0]); assert(!strcmp(p.game.screenName(),"toytown_title"));
  p.action(0); assert(p.on(HUB)); p.press(ACTIONS[0]); assert(p.on(HUB));
  p.frame(false,0,0,500); p.frame(true,51,117); p.frame(true,0,0,40); p.frame(false);
  assert(p.on(HUB)); // dragging off cancels a press
  p.frame(false,0,0,500); p.frame(true,0,0); p.frame(true,51,117,40); p.frame(false);
  assert(p.on(HUB)); // entering a target from outside does not click it
  p.frame(false,0,0,500); p.press(ACTIONS[0],900); assert(p.on(MAP)); // small fingers hold
  p.action(1); assert(p.on(PACK)); p.tap(BACK);
  assert(!strcmp(p.game.screenName(),"toytown_pause"));
  p.tap(BACK); assert(p.on(PACK) && !strcmp(p.game.screenName(),"toytown_pack"));
  p.tap(BACK); p.action(1); assert(p.on(HUB));
  p.tap(BACK); p.action(0); assert(p.on(HUB));
  Player title; title.enter(); title.action(2); assert(title.game.wantsHome());
  Player back; back.enter(); back.tap(BACK); assert(back.game.wantsHome());
}
void invalidAppSave() {
  State broken = fixture(PARK_CLUE,PARK), other = fixture(SCHOOL_PLAN);
  broken.doll=3; seal(broken); broken.version=VERSION+1;
  broken.crc=os::crc32(&broken,sizeof broken-4); seal(other);
  Player p; p.saves[0]={&broken,sizeof broken}; p.saves[1]={&other,sizeof other}; p.start();
  assert(p.on(HUB) && p.game.state().doll == 0 && !p.game.state().active);
  State saved = p.snapshot(); assert(saved.seed == fresh().seed);
}
void wardrobeAndBag() {
  Player p; p.start(); p.action(1); p.action(1); assert(p.on(STYLE));
  const int counts[] = {3,4,3,3,3,4,4,4};
  for (int category = 0; category < 8; ++category) {
    const State before = p.game.state();
    for (int i = 0; i < counts[category]; ++i) p.action(0);
    const State& after = p.game.state();
    assert(before.hair == after.hair && before.skin == after.skin && before.outfit == after.outfit);
    assert(before.shoes == after.shoes && before.accessory == after.accessory && before.color == after.color);
    assert(before.dollName == after.dollName && before.doll == after.doll);
    p.action(1);
  }
  assert(p.game.state().category == 0); p.action(2); p.action(2); p.action(0);
  assert(p.on(MAP)); p.action(2); assert(p.on(HUB)); p.action(0); p.action(1);
  for (int i = 0; i < 3; ++i) { p.action(0); p.action(1); }
  assert(slots(p.game.state()) == 3); const uint8_t bag = p.game.state().bag;
  p.action(0); assert(p.game.state().bag == bag); // full bags do not silently lose an item
  p.action(1); p.action(1); p.action(0); assert(slots(p.game.state()) == 2);
  p.action(1); p.action(1); p.action(1); p.action(0); assert(slots(p.game.state()) == 3);
  p.action(2); assert(p.on(SCHOOL_INTRO));
}
void school() {
  Player p; State s = fixture(SCHOOL_INTRO); p.start(&s);
  p.action(0); assert(p.game.state().listened && p.on(SCHOOL_LISTEN));
  p.action(1); assert(p.on(SCHOOL_LISTEN)); p.action(0); p.action(1);
  assert(p.on(SCHOOL_PAPER)); p.action(2); assert(p.on(SCHOOL_PAPER));
  p.action(1); p.action(1); p.action(0);
  assert(p.on(SCHOOL_RESULT) && p.game.state().schoolOutcome == 2);
  p.action(2); assert(p.on(SCHOOL_RESULT)); p.action(0); p.action(0);
  assert(p.game.state().repaired == 1); p.action(1); assert(p.game.state().repaired == 2);
  p.action(2); assert(p.on(MEMORY)); p.action(1); assert(p.on(MEMORY_EDIT));
  p.action(0); p.action(1); p.action(2); p.action(0);
  assert(p.on(STOP) && p.game.state().count == 1 && !p.game.state().active);
  assert(p.game.state().entries[0].caption == 1 && p.game.state().entries[0].sticker == 1);
  p.press(ACTIONS[0]); assert(p.on(STOP) && p.game.state().count == 1);
  p.action(2); assert(p.on(BOOK)); p.action(0); p.action(1); p.action(2); assert(p.on(HUB));
  p.action(2); p.action(2); p.action(0); p.action(1); p.action(2); assert(p.on(SCHOOL_INTRO));
}
void schoolOtherPlans() {
  Player p; State s = fixture(SCHOOL_INTRO); p.start(&s);
  p.action(2); assert(p.on(SCHOOL_OTHER)); p.action(2); assert(p.on(SCHOOL_PLAN));
  p.action(2); p.action(1); assert(p.on(SCHOOL_LISTEN) && p.game.state().listened);
  p.action(2); p.action(0); assert(p.game.state().schoolOutcome == 4);
  Player q; s = fixture(SCHOOL_PLAN); s.bag = 1; q.start(&s); q.action(1);
  assert(q.on(SCHOOL_RESULT) && q.game.state().schoolOutcome == 3);
  Player r; s = fixture(SCHOOL_PLAN); r.start(&s); r.action(0);
  assert(r.game.state().schoolOutcome == 1); r.action(1); r.action(2); r.action(0);
  assert(r.game.wantsHome());
}
void cafeOrders() {
  for (int a = 0; a < 4; ++a) for (int b = 0; b < 4; ++b)
    for (int c = 0; c < 4; ++c) for (int d = 0; d < 4; ++d) {
      State s = fixture(CAFE_CART,CAFE); s.cart[0]=a; s.cart[1]=b; s.cart[2]=c; s.cart[3]=d;
      Player p; p.start(&s); p.action(2);
      assert(p.on(canOrder(s) ? CAFE_RESULT : CAFE_CART));
      assert(bool(p.game.state().ordered) == canOrder(s));
      assert(!memcmp(s.cart,p.game.state().cart,4));
      assert(!p.game.state().count); // no partial order or duplicate scrapbook credit
    }
}
void cafeCompareAndFinish() {
  Player p; State s = fixture(CAFE_INTRO,CAFE); memset(s.cart,0,4); p.start(&s);
  p.action(2); assert(p.on(CAFE_INTRO)); p.action(1); p.action(0); assert(p.game.state().compared);
  p.action(1); assert(p.on(CAFE_MENU));
  p.action(0); p.action(1); p.action(0); p.action(1); p.action(0); p.action(0);
  assert(total(p.game.state()) == 10); p.action(2); p.action(1); p.action(0);
  assert(!memcmp(p.game.state().cart,p.game.state().compare,4));
  p.action(2); p.action(2); assert(p.on(CAFE_RESULT));
  const State lunch = p.game.state(); p.action(2); // seeing lunch must not allow ordering a second tray
  assert(p.game.state().ordered && !memcmp(lunch.cart,p.game.state().cart,4));
  p.action(0); assert(p.on(CAFE_RESULT));
  p.action(1); p.action(0); assert(p.on(MEMORY)); p.action(0);
  assert(p.game.state().count == 1 && p.game.state().entries[0].outcome == 0);
  p.action(1); assert(p.on(HUB));
}
void park() {
  for (uint8_t variant = 0; variant < 3; ++variant) {
    Player p; State s = fixture(PARK_INTRO,PARK); s.variant = variant; p.start(&s);
    p.action(0); assert(p.game.state().listened); p.action(1); assert(p.on(PARK_SEARCH));
    for (int clue = 0; clue < 3; ++clue) {
      p.action(0); assert(p.on(PARK_CLUE)); p.action(2); assert(p.on(PARK_CLUE));
      p.action(0); p.action(1);
    }
    assert(p.game.state().clues == 7); p.action(2); assert(p.on(PARK_GUESS));
    while (p.game.state().guess == variant) p.action(1);
    p.action(0); assert(p.on(PARK_WRONG)); p.action(2); assert(p.game.state().revised);
    p.action(2); p.action(1); assert(p.on(PARK_GUESS));
    while (p.game.state().guess != variant) p.action(1);
    p.action(0); assert(p.on(PARK_FOUND)); p.action(2); p.action(1); p.action(1); p.action(0);
    assert(p.on(PARK_FOUND)); p.action(0); p.action(0);
    assert(p.game.state().entries[0].outcome == variant && p.game.state().count == 1);
  }
}
void interruptedPages() {
  for (uint8_t page = HUB; page < PAGE_COUNT; ++page) {
    const uint8_t place = page >= PARK_INTRO && page <= PARK_FOUND ? PARK : page >= CAFE_INTRO && page <= CAFE_RESULT ? CAFE : SCHOOL;
    Player p; State s = fixture(page,place); s.bag=5; s.listened=1; p.start(&s);
    const State saved = p.snapshot();
    Player resumed; State copy = saved; resumed.now = NOW + 14 * 86400; resumed.start(&copy);
    assert(!memcmp(&saved,&resumed.game.state(),sizeof saved));
    resumed.now = NOW - 86400; resumed.frame(false,0,0,500);
    assert(!memcmp(&saved,&resumed.game.state(),sizeof saved));
    resumed.tap(BACK); resumed.action(1); assert(resumed.on(HUB));
    if ((page >= SCHOOL_INTRO && page <= MEMORY) || page == MEMORY_EDIT) {
      resumed.action(0); assert(resumed.on(page));
    }
  }
}
void optionalMemory() {
  State s = fixture(MEMORY,PARK); s.completed = 1 << SCHOOL;
  s.count = 8; s.bookIndex = 7;
  for (auto& entry : s.entries) entry = {SCHOOL,2,1,1};
  Player skip; skip.start(&s); skip.action(2);
  assert(skip.on(STOP) && !skip.game.state().active && skip.game.state().recorded);
  assert(skip.game.state().completed == ((1 << SCHOOL) | (1 << PARK)));
  assert(skip.game.state().count == 8 && !memcmp(skip.game.state().entries,s.entries,sizeof s.entries));
  skip.press(ACTIONS[2]); assert(skip.on(STOP) && skip.game.state().count == 8);
  State saved = skip.snapshot(); Player reopened; reopened.start(&saved);
  assert(reopened.on(STOP) && reopened.game.state().count == 8 && reopened.game.state().recorded);
  Player empty; s = fixture(MEMORY,PARK); empty.start(&s); empty.action(2);
  assert(empty.on(STOP) && !empty.game.state().count && empty.game.state().recorded);
  Player simple; s = fixture(MEMORY,CAFE); simple.start(&s); simple.action(0);
  assert(simple.on(STOP) && simple.game.state().count == 1);
  assert(!simple.game.state().entries[0].caption && !simple.game.state().entries[0].sticker);
}
void memoryEditResume() {
  State s = fixture(MEMORY,PARK); Player p; p.start(&s); p.action(1);
  assert(p.on(MEMORY_EDIT) && !strcmp(p.game.screenName(),"toytown_memory_edit"));
  assert(p.game.state().resumePage == MEMORY_EDIT);
  p.action(0); p.action(0); p.action(1);
  State saved = p.snapshot(); Player reopened; reopened.start(&saved);
  assert(reopened.on(MEMORY_EDIT) && reopened.game.state().caption == 2 && reopened.game.state().sticker == 1);
  reopened.tap(BACK); reopened.action(1); reopened.action(0); assert(reopened.on(MEMORY_EDIT));
  reopened.action(2); assert(reopened.on(MEMORY)); reopened.action(0);
  assert(reopened.game.state().count == 1 && reopened.game.state().entries[0].caption == 2);
  assert(reopened.game.state().entries[0].sticker == 1);
}
void personalizeDuringOuting() {
  State s = fixture(PARK_GUESS,PARK); s.clues=5; s.guess=2; s.seed=0xfedcba98;
  Player p; p.start(&s); p.tap(BACK); p.action(1); p.action(1); p.action(1); p.action(0);
  assert(p.on(STYLE) && p.game.state().hair == 1);
  p.tap(BACK); p.action(1); p.action(0);
  assert(p.on(PARK_GUESS) && p.game.state().active);
  assert(p.game.state().seed == s.seed && p.game.state().variant == s.variant);
  assert(p.game.state().clues == 5 && p.game.state().guess == 2);
  State saved = p.snapshot(); Player reopened; reopened.start(&saved);
  assert(reopened.on(PARK_GUESS) && reopened.game.state().hair == 1);
}
void render(Game& game) {
  static std::vector<uint16_t> buffers[2] = {std::vector<uint16_t>(480*480),std::vector<uint16_t>(480*480)};
  std::fill(buffers[0].begin(),buffers[0].end(),0xf81f);
  std::fill(buffers[1].begin(),buffers[1].end(),0x07e0);
  gfx565::target(buffers[0].data()); game.render(); gfx565::target(buffers[1].data()); game.render();
  assert(buffers[0] == buffers[1]);
  gfx565::target(buffers[0].data()); game.render(); assert(buffers[0] == buffers[1]);
}
void textAvoidsControls(const gfx::TextBox& b, const char* screen) {
  for (int j = 0; j < UiAudit::count; ++j) {
    const auto& target = UiAudit::regions[j];
    const bool outside = b.x >= target.x+target.w || b.x+b.w <= target.x || b.y >= target.y+target.h || b.y+b.h <= target.y;
    const bool contained = b.x >= target.x && b.x+b.w <= target.x+target.w && b.y >= target.y && b.y+b.h <= target.y+target.h;
    if (!outside && !contained) fprintf(stderr,"%s text/control overlap %d,%d,%d,%d\n",screen,b.x,b.y,b.w,b.h);
    assert(outside || contained);
  }
}
void geometry(Player& p) {
  UiAudit::enabled=true; UiAudit::reset(); gfx::textLogEnabled=true; gfx::textLogReset();
  p.frame(); UiAudit::enabled=false; render(p.game); assert(UiAudit::count == 4 && gfx::textLogCount > 0);
  for (int i = 0; i < UiAudit::count; ++i) {
    const auto& b = UiAudit::regions[i]; assert(b.w >= 24 && b.h >= 22);
    for (int x : {int(b.x),b.x+b.w-1}) for (int y : {int(b.y),b.y+b.h-1}) assert(gfx::inCircle(x,y));
  }
  for (int i = 0; i < gfx::textLogCount; ++i) {
    const auto& b = gfx::textLog[i];
    for (int x : {int(b.x),b.x+b.w}) for (int y : {int(b.y),b.y+b.h}) {
      if (!gfx::inCircle(x,y,1)) fprintf(stderr,"%s clipped text %d,%d,%d,%d\n",p.game.screenName(),b.x,b.y,b.w,b.h);
      assert(gfx::inCircle(x,y,1));
    }
    textAvoidsControls(b,p.game.screenName());
  }
  UiAudit::enabled=false; gfx::textLogEnabled=false;
}
void renderedPages() {
  Player title; title.enter(); geometry(title); title.tap(BACK); title.action(0);
  for (uint8_t page = HUB; page < PAGE_COUNT; ++page) {
    State s = fixture(page); s.count=1; s.entries[0]={PARK,1,2,2};
    Player p; p.start(&s); geometry(p); p.tap(BACK); geometry(p);
  }
}
void renderedVariants() {
  for (int outcome = 1; outcome <= 4; ++outcome) {
    State s = fixture(SCHOOL_RESULT); s.schoolOutcome = outcome; Player p; p.start(&s); geometry(p);
  }
  for (int repaired = 0; repaired <= 2; ++repaired) {
    State s = fixture(SCHOOL_REPAIR); s.repaired = repaired; Player p; p.start(&s); geometry(p);
  }
  for (int quantity : {0,1,3}) {
    State s = fixture(CAFE_CART,CAFE); memset(s.cart,quantity,4); Player p; p.start(&s); geometry(p);
    p.action(1); p.action(0); geometry(p);
  }
  for (uint8_t variant = 0; variant < 3; ++variant) for (uint8_t clue = 0; clue < 3; ++clue) {
    State s = fixture(PARK_CLUE,PARK); s.variant=variant; s.clue=clue; s.clues=7;
    Player p; p.start(&s); geometry(p);
  }
  for (uint8_t category = 0; category < 8; ++category) {
    State s = fixture(STYLE); s.category=category; s.doll=3; s.hair=s.outfit=s.shoes=s.accessory=2;
    s.skin=s.color=s.dollName=3; Player p; p.start(&s); geometry(p);
  }
}
void contentGlyphs(const char* text) {
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
    assert(*p < 128); // authored copy is ASCII; catch a missing smart quote before it becomes a box
    if (*p >= 32) { assert(font::glyphIndex(art::BODY,*p)); assert(font::glyphIndex(art::TITLE,*p)); }
  }
}
void content() {
  for (const auto& scene : SCENES) { contentGlyphs(scene.title); contentGlyphs(scene.words); for (const auto* a : scene.actions) contentGlyphs(a); }
  for (const auto* words : LAST_SEEN) contentGlyphs(words);
  for (const auto* words : CLUES) contentGlyphs(words);
  for (const auto* words : EMPTY_CLUES) contentGlyphs(words);
  for (const auto* words : SCHOOL_ENDINGS) contentGlyphs(words);
}
void cosmeticPixels() {
  const size_t pixels = 480 * 480;
  std::vector<uint16_t> canvas(pixels + 128,0x55aa);
  gfx565::target(canvas.data()+64);
  art::background(0);
  const std::vector<uint16_t> street(canvas.begin()+64,canvas.end()-64);
  for (int scene = 1; scene < 5; ++scene) {
    art::background(scene);
    assert(!std::equal(street.begin(),street.end(),canvas.begin()+64));
  }
  struct Choice { uint8_t State::*member; int count; };
  const Choice choices[] = {{&State::hair,3},{&State::skin,4},{&State::outfit,3},
    {&State::shoes,3},{&State::accessory,3},{&State::color,4}};
  for (uint8_t doll = 0; doll < 4; ++doll) {
    State base = fresh(); base.doll = doll;
    std::fill(canvas.begin()+64,canvas.end()-64,0); art::doll(base,100,0,1);
    const std::vector<uint16_t> original(canvas.begin()+64,canvas.end()-64);
    for (const auto& choice : choices) for (int value = 1; value < choice.count; ++value) {
      State changed = base; changed.*choice.member = value;
      std::fill(canvas.begin()+64,canvas.end()-64,0); art::doll(changed,100,0,1);
      assert(!std::equal(original.begin(),original.end(),canvas.begin()+64));
    }
    art::doll(base,-80,300,2); // sprites clipped by the panel keep memory outside it intact
  }
  for (int i = 0; i < 64; ++i) { assert(canvas[i] == 0x55aa); assert(canvas[pixels+64+i] == 0x55aa); }
}
} // namespace
int main() {
  identityAndSave(); inputLifecycle(); invalidAppSave(); wardrobeAndBag(); school(); schoolOtherPlans();
  cafeOrders(); cafeCompareAndFinish(); park(); interruptedPages(); personalizeDuringOuting();
  optionalMemory(); memoryEditResume();
  renderedPages(); renderedVariants(); content(); cosmeticPixels();
  puts("Toy Town App/UI contracts passed");
}
