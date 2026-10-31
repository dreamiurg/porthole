// App-level regressions. The required player-led emulator tour is separate evidence.
#include <algorithm>
#include <assert.h>
#include <initializer_list>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "games/toy-town-life/art.h"
#include "games/toy-town-life/content.h"
#include "games/toy-town-life/choices.h"
#include "games/toy-town-life/scene_ui.h"
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
  void tap(const ui::Box& b) {
    const bool dress=!strcmp(game.screenName(),"toytown_doll") || !strcmp(game.screenName(),"toytown_style");
    const bool back=b.x==BACK.x && b.y==BACK.y;
    const ui::Box target=back ? (dress ? (game.state().page==STYLE ? choices::EXIT : BACK) : sceneui::BACK) : b;
    frame(false,0,0,500); press(target);
  }
  void action(int i) {
    if (!strcmp(game.screenName(),"toytown_doll")) {
      if (i==0) tap(choices::DOLLS[(game.state().doll+1)%4]);
      else if (i==1) tap(choices::WARDROBE);
      else { tap(BACK); action(1); }
    } else if (!strcmp(game.screenName(),"toytown_style")) {
      if (i==0) tap(choices::LOOKS[(choices::value(game.state(),game.state().category)+1)%choices::count(game.state().category)]);
      else tap(i==1 ? choices::NEXT : choices::DONE);
    } else tap(ACTIONS[i]);
  }
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
  s.schoolOutcome = page==SCHOOL_RESULT || page==SCHOOL_REPAIR ? 2 : 0; s.variant = 1; s.cart[1] = s.cart[2] = 1;
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
  const int counts[] = {3,4,3,3,3,4,4,4,4};
  for (int category=0;category<9;++category) {
    const State before=p.game.state();
    for(int i=0;i<counts[category];++i) p.action(0);
    const State& after=p.game.state();
    assert(before.hair==after.hair && before.skin==after.skin && before.outfit==after.outfit);
    assert(before.shoes==after.shoes && before.accessory==after.accessory && before.color==after.color);
    assert(before.dollName==after.dollName && before.doll==after.doll && before.hairColor==after.hairColor);
    p.action(1);
  }
  assert(p.game.state().category==0);
  State packing=fixture(PACK); Player bag; bag.start(&packing);
  for(int i=0;i<3;++i) bag.tap(sceneui::card(i,5));
  assert(bag.game.state().bag==7 && slots(bag.game.state())==3);
  bag.tap(sceneui::card(3,5)); assert(bag.game.state().bag==7);
  bag.tap(sceneui::card(0,5)); bag.tap(sceneui::card(3,5));
  assert(bag.game.state().bag==14 && slots(bag.game.state())==3);
  bag.tap(sceneui::DEPART); assert(bag.on(SCHOOL_INTRO));
}

void picturedSelections() {
 Player p; p.start(); p.action(1);
 for (int i=0;i<4;++i) { p.tap(choices::DOLLS[i]); assert(p.game.state().doll==i); }
 p.tap(choices::WARDROBE);
 for (int c=0;c<choices::CATEGORIES;++c) {
  assert(p.game.state().category==c);
  for (int i=0;i<choices::count(c);++i) {
   p.tap(choices::LOOKS[i]); assert(choices::value(p.game.state(),c)==i);
   p.tap(choices::LOOKS[i]); assert(choices::value(p.game.state(),c)==i);
  }
  p.tap(choices::NEXT);
 }
 assert(p.game.state().category==0); p.tap(choices::PREVIOUS); assert(p.game.state().category==8);
 State saved=p.snapshot(); assert(saved.hairColor==3 && saved.color==3 && saved.doll==3);
 Player reopened; reopened.start(&saved); assert(reopened.game.state().hairColor==3);
 reopened.tap(choices::DONE); assert(reopened.on(DOLL));
}
// Scene-first contracts deliberately exercise ordinary pictured targets.
void finishDisplayWithPictures(Player& p) {
  assert(p.on(SCHOOL_HANG));
  for(int slot=0;slot<3;++slot) {
    p.tap(sceneui::card(slot,4)); p.tap(sceneui::FRAMES[slot]);
  }
  p.tap(sceneui::card(3,4));
}
void orderedCafeActor() {
  State s=fixture(CAFE_RESULT,CAFE); s.ordered=1; s.cart[0]=s.cart[2]=1;
  Player p; p.start(&s); p.tap(sceneui::MINA);
  assert(p.on(CAFE_RESULT) && p.game.state().ordered);
  p.action(1); assert(p.on(CAFE_RESULT));
  assert(!memcmp(s.cart,p.game.state().cart,4) && !p.game.state().compared);
}
void changeParkIdeaImmediately() {
  for(uint8_t guess=0;guess<3;++guess) {
    State s=fixture(PARK_WRONG,PARK); s.guess=guess; s.variant=(guess+1)%3;
    Player p; p.start(&s); p.action(1);
    assert(p.on(PARK_GUESS) && p.game.state().revised);
    assert(p.game.state().guess==(guess+1)%3);
    p.action(0); assert(p.on(PARK_FOUND)); // confirm the new visible idea without a compensating cycle
  }
}
void visibleParkEvidence(Game& game,int prop,const gfx565::Rect& box) {
  std::vector<uint16_t> actual(480*480),expected(480*480,0xf81f);
  gfx565::target(actual.data()); game.render();
  gfx565::target(expected.data()); art::prop(prop,box);
  int opaque=0,visible=0,pink=0,visiblePink=0;
  for(int y=box.y;y<box.y+box.h;++y) for(int x=box.x;x<box.x+box.w;++x) {
    const int i=y*480+x; if(expected[i]==0xf81f) continue;
    ++opaque; visible+=actual[i]==expected[i];
    const uint32_t color=gfx565::rgb888(expected[i]);
    const int red=(color>>16)&255,green=(color>>8)&255,blue=color&255;
    if(red>green+25 && blue>green+10 && red>blue) {
      ++pink; visiblePink+=actual[i]==expected[i];
    }
  }
  assert(opaque>30 && visible*5>=opaque*4);
  assert(pink>5 && visiblePink*5>=pink*4); // the matching pink ribbon must remain legible too
}
void matchingEvidenceSurvivesApproach() {
  for(uint8_t variant=0;variant<3;++variant) {
    State s=fixture(PARK_INTRO,PARK); s.variant=variant; Player p; p.start(&s);
    visibleParkEvidence(p.game,art::RIBBON,sceneui::ribbon(variant));
    p.tap(sceneui::SPOTS[variant]); p.frame(false,0,0,1000);
    assert(p.on(PARK_CLUE) && p.game.state().clue==variant);
    visibleParkEvidence(p.game,art::NOTEBOOK,sceneui::notebook(variant));
  }
}
void inspectedParkIdea() {
  for(uint8_t spot=0;spot<3;++spot) {
    State s=fixture(PARK_CLUE,PARK); s.clue=spot; s.clues=1<<spot; s.guess=(spot+1)%3;
    Player p; p.start(&s); p.action(1);
    assert(p.on(PARK_GUESS) && p.game.state().guess==spot);
    State saved=p.snapshot(); Player resumed; resumed.start(&saved);
    assert(resumed.on(PARK_GUESS) && resumed.game.state().guess==spot);
  }
}
void scrapbookWorldUsesEntry() {
  State first=fixture(BOOK,SCHOOL); first.count=1; first.entries[0]={SCHOOL,1,0,0};
  first.worldX=80; first.worldY=80; first.schoolOutcome=1;
  State later=first; later.destination=PARK; later.variant=2; later.clues=7;
  later.schoolOutcome=4; later.repaired=3; later.cart[3]=3; later.worldX=110; later.worldY=60;
  Player a,b; a.start(&first); b.start(&later);
  std::vector<uint16_t> oldFrame(480*480),newFrame(480*480);
  gfx565::target(oldFrame.data()); a.game.render(); gfx565::target(newFrame.data()); b.game.render();
  for(int y=162;y<264;++y) for(int x=70;x<410;++x)
    assert(oldFrame[y*480+x]==newFrame[y*480+x]);
}
void schoolDisplayActivity() {
  State s=fixture(SCHOOL_INTRO); Player p; p.start(&s); p.tap(sceneui::card(0,5));
  assert(p.on(SCHOOL_HANG) && !p.game.state().schoolOutcome);
  p.tap(sceneui::card(3,4)); assert(p.on(SCHOOL_HANG) && !p.game.state().schoolOutcome);
  const auto& frames=sceneui::FRAMES;
  for(int i=1;i<3;++i) assert(frames[i].x-(frames[i-1].x+frames[i-1].w)>=2);
  p.tap(sceneui::card(1,4)); p.tap(frames[0]);
  assert(p.game.state().schoolHangMask==1 && (p.game.state().schoolPictures&3)==1);
  p.tap(sceneui::card(2,4)); p.tap(frames[0]);
  assert(p.game.state().schoolHangMask==1 && (p.game.state().schoolPictures&3)==2);
  p.tap(sceneui::card(3,4)); assert(p.on(SCHOOL_HANG) && !p.game.state().schoolOutcome);
  State saved=p.snapshot(),other=fixture(PARK_CLUE,PARK); p.saves[0]={&saved,sizeof saved};
  p.enter(&other,1); p.action(0); assert(p.on(PARK_CLUE) && !p.game.state().schoolHangMask);
  p.enter(nullptr,0); p.action(0);
  assert(p.on(SCHOOL_HANG) && p.game.state().schoolHangMask==1 && p.game.state().schoolPicture==2);
  p.tap(sceneui::card(0,4)); p.tap(frames[1]); p.tap(sceneui::card(1,4)); p.tap(frames[2]);
  assert(p.on(SCHOOL_HANG) && p.game.state().schoolHangMask==7 && !p.game.state().schoolOutcome);
  assert(p.game.state().schoolPictures==18); // rabbit, flowers, rocket in the three chosen slots
  p.tap(sceneui::card(3,4)); assert(p.on(SCHOOL_RESULT) && p.game.state().schoolOutcome==4);
}
void arrangePicturesPreservesCreation() {
  State s=fixture(SCHOOL_INTRO); Player p; p.start(&s); p.tap(sceneui::card(0,5));
  finishDisplayWithPictures(p);
  assert(p.on(SCHOOL_RESULT) && p.game.state().schoolPictures==36);
  p.action(0); assert(p.on(SCHOOL_REPAIR) && p.game.state().repaired==2);
  p.action(0); // Arrange pictures reopens the child's existing display.
  assert(p.on(SCHOOL_HANG) && p.game.state().schoolHangMask==7);
  assert(p.game.state().schoolPictures==36 && p.game.state().schoolPicture==2);
  p.tap(sceneui::card(0,4)); p.tap(sceneui::FRAMES[1]);
  assert(p.game.state().schoolPictures==32 && p.game.state().schoolHangMask==7);
  p.tap(BACK); assert(!strcmp(p.game.screenName(),"toytown_pause"));
  State saved=p.snapshot(); Player resumed; resumed.start(&saved);
  assert(resumed.on(SCHOOL_HANG) && resumed.game.state().schoolHangMask==7);
  assert(resumed.game.state().schoolPictures==32 && !resumed.game.state().schoolPicture);
  resumed.tap(sceneui::card(3,4));
  assert(resumed.on(SCHOOL_REPAIR) && resumed.game.state().repaired==3);
  resumed.action(0); assert(resumed.on(SCHOOL_HANG));
  assert(resumed.game.state().schoolPictures==32 && resumed.game.state().schoolHangMask==7);
}
void displayPicturesAreVisible() {
  std::vector<uint16_t> images[3];
  for(int picture=0;picture<3;++picture) {
    State s=fixture(SCHOOL_HANG); s.schoolTask=1; s.schoolHangMask=7;
    s.schoolPictures=picture*21; Player p; p.start(&s); images[picture].resize(480*480);
    gfx565::target(images[picture].data()); p.game.render();
  }
  for(const auto& frame:sceneui::FRAMES) for(int picture=1;picture<3;++picture) {
    int changed=0;
    for(int y=frame.y*3;y<(frame.y+frame.h)*3;++y)
      for(int x=frame.x*3;x<(frame.x+frame.w)*3;++x)
        changed+=images[picture][y*480+x]!=images[picture-1][y*480+x];
    assert(changed>100); // flowers, rocket and rabbit are different pictures in each real frame
  }
}
void literalPracticeWithMina() {
  for(int repaired : {0,1}) {
    State s=fixture(SCHOOL_RESULT); s.schoolOutcome=4; s.repaired=repaired;
    Player p; p.start(&s); std::vector<uint16_t> before(480*480),after(480*480);
    gfx565::target(before.data()); p.game.render();
    p.action(0); assert(p.on(SCHOOL_REPAIR) && p.game.state().repaired==(repaired|2));
    p.frame(false,0,0,1000);
    assert(p.game.state().worldX==61 && p.game.state().worldY==80);
    gfx565::target(after.data()); p.game.render(); int changed=0;
    const auto& mina=sceneui::MINA_SPRITE;
    for(int y=mina.y;y<mina.y+mina.h;++y) for(int x=mina.x;x<mina.x+mina.w;++x)
      changed+=before[y*480+x]!=after[y*480+x];
    assert(changed>100); // the promised rehearsal visibly happens when the child taps it
    State saved=p.snapshot(); Player reopened; reopened.start(&saved);
    assert(reopened.on(SCHOOL_REPAIR) && reopened.game.state().repaired==(repaired|2));
    reopened.action(1); assert(reopened.game.state().repaired==(repaired|2));
  }
}
void schoolRepairsBothOrders() {
  for(int first : {0,1}) {
    State s=fixture(SCHOOL_REPAIR); s.schoolOutcome=1; Player p; p.start(&s);
    for(int task : {first,1-first}) {
      p.action(task); if(task==0) finishDisplayWithPictures(p);
      assert(p.on(SCHOOL_REPAIR));
    }
    assert(p.game.state().repaired==3);
    State saved=p.snapshot(); Player reopened; reopened.start(&saved);
    assert(reopened.on(SCHOOL_REPAIR) && reopened.game.state().repaired==3);
  }
}
void repairedPropsKeepBoth() {
  std::vector<uint16_t> frames[4];
  for(int repair=0;repair<4;++repair) {
    State s=fixture(SCHOOL_REPAIR); s.schoolOutcome=1; s.repaired=repair;
    Player p; p.start(&s); frames[repair].resize(480*480);
    gfx565::target(frames[repair].data()); p.game.render();
  }
  const gfx565::Rect crops[]={{327,170,51,50},{136,203,38,39}};
  for(int task=0;task<2;++task) {
    int changed=0; const auto& crop=crops[task];
    for(int y=crop.y;y<crop.y+crop.h;++y) for(int x=crop.x;x<crop.x+crop.w;++x) {
      const int pixel=y*480+x;
      changed+=frames[task+1][pixel]!=frames[0][pixel];
      assert(frames[3][pixel]==frames[task+1][pixel]);
    }
    assert(changed>100); // each repaired object must remain visible after helping the other friend
  }
}
void explicitSchoolScene() {
  for (int action = 0; action < 5; ++action) {
    State s = fixture(SCHOOL_INTRO); s.bag = 14;
    Player p; p.start(&s);
    p.tap(sceneui::card(action,5));
    const uint8_t pages[]={SCHOOL_HANG,SCHOOL_OTHER,SCHOOL_LISTEN,SCHOOL_PLAN,SCHOOL_PAPER};
    assert(p.on(pages[action]));
    assert(!p.game.state().schoolOutcome);
    assert(p.game.state().active && !p.game.state().recorded && !p.game.state().count);
    assert(p.game.state().bag == 14);
    if (action == 4) {
      assert(p.game.state().schoolPaper && !p.game.state().schoolOutcome);
      State saved=p.snapshot(); Player resumed; resumed.start(&saved);
      assert(resumed.game.state().schoolPaper && resumed.game.state().bag==14);
    }
  }
}
void sceneMovement() {
  State s=fixture(SCHOOL_INTRO); Player p; p.start(&s);
  const uint8_t x=s.worldX,y=s.worldY;
  p.frame(false,0,0,500); p.frame(true,80,55); p.frame(true,150,150,40); p.frame(false,150,150);
  p.frame(false,0,0,1000);
  assert(p.game.state().worldX==x && p.game.state().worldY==y);
  const void* data=nullptr; size_t bytes=0; assert(p.game.takeSave(&data,&bytes,true));
  p.tap({68,44,24,22});
  assert(!p.game.saveUrgent());
  assert(p.game.state().worldX==x && p.game.state().worldY==y);
  p.frame(false,0,0,1000);
  assert(p.game.saveUrgent());
  assert(p.game.state().worldX==80 && p.game.state().worldY==55);
  State saved=p.snapshot(); Player resumed; resumed.start(&saved);
  assert(resumed.game.state().worldX==80 && resumed.game.state().worldY==55);
  resumed.tap({0,0,2,2}); resumed.frame(false,0,0,1000);
  assert(resumed.game.state().worldX==80 && resumed.game.state().worldY==55);
}
void movementInterruptionAndRetarget() {
  State s=fixture(SCHOOL_INTRO); Player p; p.start(&s);
  p.frame(false,0,0,500); p.press({68,44,24,22});
  p.frame(false,0,0,220);
  assert(p.game.state().worldY==80); // animation frames do not repeatedly persist positions
  p.press(sceneui::BACK);
  assert(!strcmp(p.game.screenName(),"toytown_pause"));
  const int stopped=p.game.state().worldY;
  assert(stopped>55 && stopped<80);
  p.frame(false,0,0,1000); assert(p.game.state().worldY==stopped);
  State saved=p.snapshot(); Player resumed; resumed.start(&saved);
  assert(resumed.game.state().worldY==stopped);
  resumed.frame(false,0,0,500); resumed.press({68,44,24,22});
  resumed.frame(false,0,0,200);
  resumed.press({68,63,24,22}); // a new floor destination replaces the unfinished walk
  resumed.frame(false,0,0,1000);
  assert(resumed.game.state().worldX==80 && resumed.game.state().worldY==74);
  resumed.frame(false,0,0,500); resumed.frame(true,0,0);
  resumed.frame(true,80,55,40); resumed.frame(false,80,55);
  resumed.frame(false,0,0,1000);
  assert(resumed.game.state().worldY==74); // entering floor from outside cannot start movement
}
void cafeTrayTouchAndWalking() {
  for(uint8_t page : {uint8_t(CAFE_INTRO),uint8_t(CAFE_MENU),uint8_t(CAFE_QUANTITY),uint8_t(CAFE_RESULT)}) {
    State s=fixture(page,CAFE); s.ordered=page==CAFE_RESULT;
    Player p; p.start(&s);
    p.tap({121,79,2,2}); // physical (366,240): the visible tray, not the header button
    assert(p.on(s.ordered ? CAFE_RESULT : CAFE_CART));
    p.frame(false,0,0,1000);
    assert(p.game.state().worldX==80 && p.game.state().worldY==80);
  }
  State s=fixture(CAFE_MENU,CAFE); Player p; p.start(&s);
  p.frame(false,0,0,500); p.frame(true,122,80);
  p.frame(true,80,55,40); p.frame(false,80,55); p.frame(false,0,0,1000);
  assert(p.on(CAFE_MENU) && p.game.state().worldX==80); // canceled tray drag
  p.tap({126,55,2,2}); // approach from the free floor above the tray
  p.frame(false,0,0,1000);
  assert(p.game.state().worldX==sceneui::CAFE_WALK_MAX_X);
  assert(p.game.state().worldY==56);
  p.tap({126,61,2,2}); // another floor tap cannot leave the doll over food
  p.frame(false,0,0,250); p.press(sceneui::BACK);
  assert(p.game.state().worldX<=sceneui::CAFE_WALK_MAX_X);
  State saved=p.snapshot(); saved.worldX=122; saved.worldY=80;
  Player restored; restored.start(&saved); // repair an older save parked on the tray
  assert(restored.game.state().worldX==sceneui::CAFE_WALK_MAX_X);
  restored.tap({126,55,2,2}); restored.frame(false,0,0,1000);
  assert(restored.game.state().worldX==sceneui::CAFE_WALK_MAX_X);
}
void cafeResumeAfterHomeWalk() {
  State s=fixture(CAFE_MENU,CAFE); Player p; p.start(&s);
  p.tap(BACK); p.action(1); assert(p.on(HUB));
  p.tap({121,79,2,2}); p.frame(false,0,0,1000);
  assert(p.game.state().worldX==122 && p.game.state().worldPlace==HOME);
  p.action(0); assert(p.on(CAFE_MENU));
  assert(p.game.state().worldX==sceneui::CAFE_WALK_MAX_X);
  p.tap({126,55,2,2}); p.frame(false,0,0,1000);
  assert(p.game.state().worldX==sceneui::CAFE_WALK_MAX_X);
  for(uint8_t page : {uint8_t(HUB),uint8_t(DOLL),uint8_t(STYLE)}) {
    State legacy=fixture(CAFE_MENU,CAFE); legacy.page=page;
    legacy.worldX=122; legacy.worldPlace=HOME;
    Player resumed; resumed.start(&legacy);
    assert(resumed.game.state().worldX==122); // Home and wardrobe remain unrestricted
    if(page!=HUB) { resumed.tap(BACK); resumed.action(1); }
    resumed.action(0); assert(resumed.on(CAFE_MENU));
    assert(resumed.game.state().worldX==sceneui::CAFE_WALK_MAX_X);
  }
  for(int i=0;i<4;++i) {
    const ui::Box card=sceneui::card(i,4);
    assert(sceneui::TRAY_TOUCH.y+sceneui::TRAY_TOUCH.h+2<=card.y);
  }
}
void explicitQuantityEditor() {
  State s=fixture(CAFE_MENU,CAFE); memset(s.cart,0,4);
  Player p; p.start(&s);
  p.tap(sceneui::card(0,4));
  assert(p.on(CAFE_QUANTITY) && p.game.state().menuItem==0);
  p.action(0); assert(!p.game.state().cart[0]); // minus clamps at zero
  for (int i=0;i<4;++i) p.action(1);
  assert(p.game.state().cart[0]==3 && total(p.game.state())==15);
  State saved=p.snapshot(); Player resumed; resumed.start(&saved);
  assert(resumed.on(CAFE_QUANTITY) && resumed.game.state().cart[0]==3);
  resumed.action(0); assert(resumed.game.state().cart[0]==2);
  resumed.action(0); assert(resumed.game.state().cart[0]==1);
  State first=resumed.snapshot(),other=fixture(PARK_CLUE,PARK);
  other.worldX=110; other.worldY=70; other.variant=2; other.clues=4; other.doll=3;
  resumed.saves[0]={&first,sizeof first}; resumed.enter(&other,1); resumed.action(0);
  assert(resumed.on(PARK_CLUE) && resumed.game.state().worldX==110 && resumed.game.state().doll==3);
  resumed.enter(nullptr,0); resumed.action(0);
  assert(resumed.on(CAFE_QUANTITY) && resumed.game.state().cart[0]==1);
  assert(resumed.game.state().worldX==first.worldX && resumed.game.state().worldY==first.worldY);
}
void school() {
  Player p; State s=fixture(SCHOOL_INTRO); p.start(&s);
  p.tap(sceneui::card(2,5)); assert(p.game.state().listened && p.on(SCHOOL_LISTEN));
  p.action(0); assert(p.on(SCHOOL_PLAN)); p.action(1); assert(p.on(SCHOOL_PAPER));
  p.action(0); assert(p.game.state().schoolPaper && !p.game.state().schoolOutcome);
  p.action(1); assert(p.on(SCHOOL_RESULT) && p.game.state().schoolOutcome==2);
  p.action(0); p.action(0); finishDisplayWithPictures(p); assert(p.game.state().repaired==1);
  p.action(1); assert(p.game.state().repaired==3); p.action(2);
  assert(p.on(MEMORY)); p.action(1); assert(p.on(MEMORY_EDIT));
  p.action(0); p.action(1); p.action(2); p.action(0);
  assert(p.on(STOP) && p.game.state().count==1 && !p.game.state().active);
  assert(p.game.state().entries[0].caption==1 && p.game.state().entries[0].sticker==1);
  p.press(ACTIONS[0]); assert(p.on(STOP) && p.game.state().count==1);
}

void schoolOtherPlans() {
  for(int outcome=1;outcome<=4;++outcome) {
    State s=fixture(outcome==4 ? SCHOOL_INTRO : SCHOOL_PLAN); s.bag=1; s.listened=outcome==2;
    Player p; p.start(&s);
    if(outcome==4) { p.tap(sceneui::card(0,5)); finishDisplayWithPictures(p); }
    else p.action(outcome==1 ? 0 : 1);
    assert(p.on(SCHOOL_RESULT) && p.game.state().schoolOutcome==outcome);
  }
}

void cafeOrders() {
  for (int a = 0; a < 4; ++a) for (int b = 0; b < 4; ++b)
    for (int c = 0; c < 4; ++c) for (int d = 0; d < 4; ++d) {
      State s = fixture(CAFE_CART,CAFE); s.cart[0]=a; s.cart[1]=b; s.cart[2]=c; s.cart[3]=d;
      Player p; p.start(&s); p.action(2);
      assert(p.on(canOrder(s) ? CAFE_RESULT : CAFE_MENU));
      assert(bool(p.game.state().ordered) == canOrder(s));
      assert(!memcmp(s.cart,p.game.state().cart,4));
      assert(!p.game.state().count); // no partial order or duplicate scrapbook credit
    }
}
void cafeCompareAndFinish() {
  State s=fixture(CAFE_MENU,CAFE); memset(s.cart,0,4); Player p; p.start(&s);
  for(int item=0;item<4;++item) {
    p.tap(sceneui::card(item,4)); assert(p.on(CAFE_QUANTITY) && p.game.state().menuItem==item);
    p.action(1); p.action(2); assert(p.on(CAFE_MENU));
  }
  assert(total(p.game.state())==11); p.tap(sceneui::DEPART); p.action(2);
  assert(p.on(CAFE_MENU) && !p.game.state().ordered);
  p.tap(sceneui::DEPART); p.action(1); p.action(0); assert(p.game.state().compared);
  p.action(1); p.tap(sceneui::card(3,4)); p.action(0); p.action(2);
  assert(total(p.game.state())==9); p.tap(sceneui::DEPART); p.action(2);
  assert(p.on(CAFE_RESULT) && p.game.state().ordered);
  const State lunch=p.game.state(); p.action(2); p.action(0);
  assert(!memcmp(lunch.cart,p.game.state().cart,4));
  p.action(0); assert(p.on(MEMORY)); p.action(0);
  assert(p.game.state().count==1 && p.game.state().entries[0].outcome==1);
}

void discoverParkInScene() {
  for(uint8_t variant=0;variant<3;++variant) {
    State s=fixture(PARK_INTRO,PARK); s.variant=variant;
    Player p; p.start(&s); p.tap(sceneui::SPOTS[variant]);
    assert(p.on(PARK_CLUE) && p.game.state().clue==variant);
    assert(p.game.state().clues==(1<<variant));
    p.action(1); assert(p.on(PARK_GUESS));
    while(p.game.state().guess==variant) p.action(1);
    p.action(0); assert(p.on(PARK_WRONG));
    p.action(0); assert(p.game.state().revised && p.on(PARK_SEARCH));
    p.tap(sceneui::SPOTS[variant]); assert(p.on(PARK_CLUE));
    p.action(1); assert(p.game.state().guess==variant);
    p.action(0); assert(p.on(PARK_FOUND));
    State saved=p.snapshot(); Player reopened; reopened.start(&saved);
    assert(reopened.on(PARK_FOUND) && reopened.game.state().variant==variant);
    assert(reopened.game.state().clues==(1<<variant) && reopened.game.state().revised);
  }
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
    p.tap(sceneui::SPOTS[variant]); p.action(1);
    assert(p.on(PARK_GUESS) && p.game.state().guess==variant);
    p.action(0); assert(p.on(PARK_FOUND)); p.action(2); p.action(1); p.action(1); p.action(0);
    assert(p.on(PARK_FOUND)); p.action(0); p.action(0);
    assert(p.game.state().entries[0].outcome == variant && p.game.state().count == 1);
  }
}
void interruptedPages() {
  for (uint8_t page = HUB; page < PAGE_COUNT; ++page) {
    const uint8_t place = page >= PARK_INTRO && page <= PARK_FOUND ? PARK : ((page >= CAFE_INTRO && page <= CAFE_RESULT) || page == CAFE_QUANTITY) ? CAFE : SCHOOL;
    Player p; State s = fixture(page,place); s.bag=5; s.listened=1; p.start(&s);
    const State saved = p.snapshot();
    Player resumed; State copy = saved; resumed.now = NOW + 14 * 86400; resumed.start(&copy);
    assert(!memcmp(&saved,&resumed.game.state(),sizeof saved));
    resumed.now = NOW - 86400; resumed.frame(false,0,0,500);
    assert(!memcmp(&saved,&resumed.game.state(),sizeof saved));
    resumed.tap(BACK); resumed.action(1); assert(resumed.on(HUB));
    if ((page >= SCHOOL_INTRO && page <= MEMORY) || page == MEMORY_EDIT || page == CAFE_QUANTITY || page == SCHOOL_HANG) {
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
void visibleScenePeopleAndWorld() {
  struct Scene { uint8_t page,place; int npc,pose; gfx565::Rect box; };
  const Scene scenes[]={{SCHOOL_INTRO,SCHOOL,art::MINA,1,sceneui::MINA_SPRITE},
                        {SCHOOL_INTRO,SCHOOL,art::THEO,0,sceneui::THEO_SPRITE},
                        {CAFE_INTRO,CAFE,art::MINA,1,sceneui::MINA_SPRITE},
                        {PARK_INTRO,PARK,art::NORI,1,sceneui::NORI_SPRITE}};
  std::vector<uint16_t> actual(480*480),person(480*480),background(480*480);
  for(const auto& scene:scenes) {
    State s=fixture(scene.page,scene.place); Player p; p.start(&s);
    gfx565::target(actual.data()); p.game.render();
    std::fill(person.begin(),person.end(),0xf81f);
    gfx565::target(person.data()); art::npc(scene.npc,scene.box,scene.pose);
    int opaque=0,visible=0;
    for(int y=scene.box.y;y<scene.box.y+scene.box.h;++y)
      for(int x=scene.box.x;x<scene.box.x+scene.box.w;++x) {
        const int i=y*480+x;
        if(person[i]!=0xf81f) { ++opaque; visible+=actual[i]==person[i]; }
      }
    if(!(opaque>100 && visible*5>opaque*4)) fprintf(stderr,"npc %d opaque %d visible %d\n",scene.npc,opaque,visible);
    assert(opaque>100 && visible*5>opaque*4); // real production characters remain in the world
    gfx565::target(background.data()); art::background(scene.place+1);
    int floor=0;
    for(int y=135;y<260;++y) for(int x=70;x<410;++x)
      floor+=actual[y*480+x]==background[y*480+x];
    assert(floor>12000); // a prose panel must not hide the scene
  }
}
void visibleStoryProps() {
  std::vector<uint16_t> before(480*480),after(480*480);
  for(int scenario=0;scenario<2;++scenario) {
    State empty=fixture(scenario ? CAFE_MENU : SCHOOL_INTRO,scenario ? CAFE : SCHOOL);
    memset(empty.cart,0,4); State supplied=empty;
    if(scenario) supplied.cart[3]=1; else supplied.schoolPaper=1;
    Player a,b; a.start(&empty); b.start(&supplied);
    gfx565::target(before.data()); a.game.render();
    gfx565::target(after.data()); b.game.render();
    int changed=0;
    // This crop excludes speech, quantity text and action cards: the actual object must change.
    const int bottom=scenario ? 256 : 264;
    for(int y=162;y<bottom;++y) for(int x=70;x<410;++x)
      changed+=before[y*480+x]!=after[y*480+x];
    assert(changed>100);
  }
}
void illustrationLabelSpace(const ui::Box& card,const char* screen,int index) {
  int firstLine=(card.y+card.h)*3;
  for(int line=0;line<gfx::textLogCount;++line) {
    const auto& text=gfx::textLog[line];
    if(text.x>=card.x && text.x+text.w<=card.x+card.w && text.y>=card.y && text.y<card.y+card.h)
      firstLine=std::min(firstLine,int(text.y)*3);
  }
  // Observe the rendered labels: leave a 24px illustration below the 5px top inset.
  if(firstLine-card.y*3<29) fprintf(stderr,"%s card %d lacks a 24px illustration\n",screen,index);
  assert(firstLine<(card.y+card.h)*3 && firstLine-card.y*3>=29);
}
std::vector<uint16_t> illustrationCrop(const std::vector<uint16_t>& pixels,const ui::Box& card) {
  std::vector<uint16_t> colors;
  for(int y=card.y*3+7;y<card.y*3+29;++y)
    for(int x=(card.x*3+card.w*3/2)-45;x<(card.x*3+card.w*3/2)+45;++x) colors.push_back(pixels[y*480+x]);
  return colors;
}
void distinctFoodPictures(const std::vector<uint16_t>& colors,std::vector<uint16_t>* previousFoods,int index) {
  for(int other=0;other<index;++other) {
    int different=0;
    for(size_t pixel=0;pixel<colors.size();++pixel) different+=colors[pixel]!=previousFoods[other][pixel];
    assert(different>64); // four foods must not share a generic menu illustration
  }
  previousFoods[index]=colors;
}
void illustratedActionCards() {
  std::vector<uint16_t> pixels(480*480);
  for(int place : {int(SCHOOL),int(CAFE)}) {
    State s=fixture(place==SCHOOL ? SCHOOL_INTRO : CAFE_MENU,place);
    Player p; p.start(&s); gfx::textLogEnabled=true; gfx::textLogReset();
    gfx565::target(pixels.data()); p.game.render(); gfx::textLogEnabled=false;
    const int count=place==SCHOOL ? 5 : 4;
    std::vector<uint16_t> previousFoods[4];
    for(int i=0;i<count;++i) {
      const auto card=sceneui::card(i,count);
      illustrationLabelSpace(card,p.game.screenName(),i);
      auto colors=illustrationCrop(pixels,card);
      if(place==CAFE) distinctFoodPictures(colors,previousFoods,i);
      std::sort(colors.begin(),colors.end());
      const auto unique=std::unique(colors.begin(),colors.end())-colors.begin();
      assert(unique>=4); // actual food/character/object pixels, beyond a flat button frame
    }
  }
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
  p.frame(); UiAudit::enabled=false; render(p.game); assert(UiAudit::count >= 4 && gfx::textLogCount > 0);
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
  for (int repaired = 0; repaired <= 3; ++repaired) {
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
  for (uint8_t category = 0; category < 9; ++category) {
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
  for (int c=0;c<choices::CATEGORIES;++c) for (int i=0;i<choices::count(c);++i) {
    assert(font::textWidth(art::BODY,choices::label(c,i))<=70);
  }
  for (const auto& scene : SCENES) { contentGlyphs(scene.title); contentGlyphs(scene.words); for (const auto* a : scene.actions) contentGlyphs(a); }
  for (const auto* words : LAST_SEEN) contentGlyphs(words);
  for (const auto* words : CLUES) contentGlyphs(words);
  for (const auto* words : EMPTY_CLUES) contentGlyphs(words);
  for (const auto* words : SCHOOL_ENDINGS) contentGlyphs(words);
}
void previewMatchesDoll() {
 std::vector<uint16_t> hero(480*480),preview(480*480);
 for (int doll=0;doll<4;++doll) for (int hair=0;hair<3;++hair) for (int accessory=0;accessory<3;++accessory) {
  State s=fresh(); s.doll=doll; s.hair=hair; s.accessory=accessory; s.hairColor=3;
  std::fill(hero.begin(),hero.end(),0xf81f); std::fill(preview.begin(),preview.end(),0xf81f);
  gfx565::target(hero.data()); art::doll(s,100,100,1);
  gfx565::target(preview.data()); art::detail(s,{100,100,96,160},0);
  assert(hero==preview);
 }
 State plush=fresh(); plush.doll=1; plush.skin=3;
 std::fill(hero.begin(),hero.end(),0); gfx565::target(hero.data()); art::doll(plush,100,100,1);
 plush.hairColor=2;
 std::fill(preview.begin(),preview.end(),0); gfx565::target(preview.data()); art::doll(plush,100,100,1);
 assert(hero[150*480+122]!=preview[150*480+122]); // outer ear tint reacts
 assert(hero[160*480+148]==preview[160*480+148]); // independently chosen face stays intact

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
    {&State::shoes,3},{&State::accessory,3},{&State::color,4},{&State::hairColor,4}};
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
  orderedCafeActor(); changeParkIdeaImmediately(); matchingEvidenceSurvivesApproach(); inspectedParkIdea(); scrapbookWorldUsesEntry(); schoolDisplayActivity(); arrangePicturesPreservesCreation(); displayPicturesAreVisible(); schoolRepairsBothOrders(); literalPracticeWithMina(); repairedPropsKeepBoth();
  explicitSchoolScene(); cafeTrayTouchAndWalking(); cafeResumeAfterHomeWalk(); explicitQuantityEditor(); sceneMovement(); movementInterruptionAndRetarget();
  identityAndSave(); inputLifecycle(); invalidAppSave(); wardrobeAndBag(); school(); schoolOtherPlans();
  cafeOrders(); cafeCompareAndFinish(); discoverParkInScene(); park(); interruptedPages(); personalizeDuringOuting();
  optionalMemory(); memoryEditResume();
  picturedSelections(); visibleScenePeopleAndWorld(); visibleStoryProps(); illustratedActionCards(); renderedPages(); renderedVariants(); content(); cosmeticPixels(); previewMatchesDoll();
  puts("Toy Town App/UI contracts passed");
}
