#include "game.h"
#include "art.h"
#include "content.h"
#include "choices.h"
#include "scene_ui.h"
#include <stdio.h>
#include <string.h>

namespace toytown {
namespace {

constexpr ui::Box BACK = {68,8,24,22};
const char* PAGES[] = {"hub","doll","style","map","pack","school_intro","school_listen","school_plan",
 "school_other","school_paper","school_result","school_repair","cafe_intro","cafe_menu","cafe_cart",
 "cafe_compare","cafe_result","park_intro","park_search","park_clue","park_guess","park_wrong","park_found",
 "memory","stop","book","memory_edit","cafe_quantity","school_hang"};

}
const gfx::Sprite& Game::icon() const { return art::icon(); }
void Game::enter(const AppEnter& e) {
 s_ = fresh(); const uint8_t id = e.who ? e.who->id : 0;
 for (int i=0;i<e.n;i++) if (e.all[i].id == id) load(e.saves[i].data,e.saves[i].len,s_);
 s_.worldX=(uint8_t)sceneui::walkX(s_,s_.worldX);
 ms_ = e.ms; in_ = {}; title_ = true; pause_ = home_ = false;
 buffers_[0] = buffers_[1] = nullptr; painted_[0] = painted_[1] = 0;
 walking_=false; displayTried_=false; walkX_=targetX_=s_.worldX; walkY_=targetY_=s_.worldY;
 destination_ = s_.destination == HOME ? (uint8_t)SCHOOL : s_.destination; changed();
}
void Game::changed() { dirty_ = true; ++look_; gate_.shown(ms_); describe(); }
void Game::go(uint8_t p) { s_.page = p; s_.worldX=(uint8_t)sceneui::walkX(s_,s_.worldX); if (s_.active && ((p >= SCHOOL_INTRO && p <= MEMORY) || p==MEMORY_EDIT || p==CAFE_QUANTITY || p==SCHOOL_HANG)) s_.resumePage = p; changed(); }
void Game::update(uint32_t, uint32_t ms, const Input& input) {
 ms_ = ms; in_ = input; gate_.filter(in_,ms);
 const bool overlay=title_ || pause_;
 if (!overlay && (s_.page==DOLL || s_.page==STYLE)) { dressInput(); return; }
 tickWalk();
 const bool back = ui::clicked(in_,sceneui::BACK);
 if (back) {
  stopWalk();
  if (title_) { home_=true; changed(); }
  else { pause_=!pause_; changed(); }
  return;
 }
 const int n=overlay ? 3 : sceneui::count(s_);
 int action=-1;
 for(int i=0;i<n;++i) if(ui::clicked(in_,sceneui::card(i,n))) action=i;
 if(action<0) {
  if(!overlay) { if(s_.page==SCHOOL_HANG) displayInput(); else sceneInput(); }
  return;
 }
 stopWalk();
 if(overlayInput(action)) return;
 act(action);
}

void Game::act(int i) {
 if(s_.page==SCHOOL_HANG) { actDisplay(i); return; }
 if(s_.page==CAFE_QUANTITY) {
  if(i<2) { auto& q=s_.cart[s_.menuItem]; if(i==0 && q) --q; if(i==1 && q<3) ++q; changed(); } else go(CAFE_MENU); return;
 }
 if (s_.page <= PACK || s_.page == BOOK || s_.page == STOP) actHome(i);
 else if (s_.page <= SCHOOL_REPAIR) actSchool(i);
 else if (s_.page <= CAFE_RESULT) actCafe(i);
 else if (s_.page <= PARK_FOUND) actPark(i);
 else actMemory(i);
}
void Game::actHome(int i) {
 if (s_.page<=STYLE) actDress(i); else actNavigate(i);
}
void Game::actDress(int i) {
 switch (s_.page) {
 case HUB: if (i==0) go(s_.active ? s_.resumePage : (uint8_t)MAP); else go(i==1 ? DOLL : BOOK); break;

 }
}
void Game::dollInput(bool exit) {
 int picked=-1;
 for (int i=0;i<4;++i) if (ui::clicked(in_,choices::DOLLS[i])) picked=i;
 const bool wardrobe=ui::clicked(in_,choices::WARDROBE);
 if (exit) { pause_=true; changed(); }
 else if (wardrobe) go(STYLE);
 else if (picked>=0) { s_.doll=(uint8_t)picked; changed(); }
}
bool Game::dressInput() {
 const bool exit=ui::clicked(in_,s_.page==DOLL ? BACK : choices::EXIT);
 if (s_.page==DOLL) { dollInput(exit); return true; }
 const bool done=ui::clicked(in_,choices::DONE);
 const bool previous=ui::clicked(in_,choices::PREVIOUS), next=ui::clicked(in_,choices::NEXT);
 int picked=-1;
 for (int i=0;i<choices::count(s_.category);++i) if (ui::clicked(in_,choices::LOOKS[i])) picked=i;
 if (exit) { pause_=true; changed(); }
 else if (done) go(DOLL);
 else if (previous || next) { s_.category=(s_.category+(previous ? choices::CATEGORIES-1 : 1))%choices::CATEGORIES; changed(); }
 else if (picked>=0) { choices::value(s_,s_.category)=(uint8_t)picked; changed(); }
 return true;
}
void Game::actNavigate(int i) {
 if (s_.page==PACK) { actPack(i); return; }
 switch (s_.page) {
 case MAP:
  destination_=(uint8_t)(i+1); s_.destination=destination_; go(PACK);
  break;
 case BOOK: if (i==0 && s_.count) { s_.bookIndex=(s_.bookIndex+1)%s_.count; changed(); } else if (i==2) go(HUB); else changed(); break;
 case STOP: if (i==0) { home_=true; changed(); } else go(i==1 ? HUB : BOOK); break;
 }
}
void Game::actPack(int i) {
 const int bit=1<<i;
 if((s_.bag&bit)||slots(s_)<3) s_.bag^=bit;
 changed();
}
void Game::actSchool(int i) {
 if (s_.page>=SCHOOL_PLAN && s_.page<=SCHOOL_PAPER) { actSchoolPlan(i); return; }
 switch (s_.page) {
 case SCHOOL_INTRO: actSchoolIntro(i); break;
 case SCHOOL_LISTEN: go(i==2 ? SCHOOL_INTRO : i==0 ? SCHOOL_PLAN : SCHOOL_LISTEN); break;
 case SCHOOL_RESULT:
  if(i==0 && s_.schoolOutcome==4 && !(s_.repaired&2)) {
   repairSchool(s_,2); go(SCHOOL_REPAIR); beginWalk(61,80);
  } else go(i==0 ? SCHOOL_REPAIR : i==1 ? MEMORY : SCHOOL_RESULT);
  break;
 case SCHOOL_REPAIR: actSchoolRepair(i); break;
 }
}
void Game::actSchoolIntro(int i) {
  if(i==0) openDisplay(false);
  else if(i==1) go(SCHOOL_OTHER);
  else if(i==2) { s_.listened=1; go(SCHOOL_LISTEN); }
  else if(i==3) { go(SCHOOL_PLAN); beginWalk(61,80); }
  else { borrowPaper(s_); go(SCHOOL_PAPER); }

}
void Game::actSchoolPlan(int i) {
 switch (s_.page) {
 case SCHOOL_PLAN:
  if (i==1 && !hasPaper(s_)) { borrowPaper(s_); go(SCHOOL_PAPER); }
  else if (i==2) {
   if(!s_.listened) { s_.listened=1; go(SCHOOL_LISTEN); }
   else { s_.schoolOutcome=3; go(SCHOOL_RESULT); }
  }
  else { finishSchool(s_,(uint8_t)i); go(SCHOOL_RESULT); beginWalk(61,80); }
  break;
 case SCHOOL_OTHER:
  if (i==0) openDisplay(false);
  else if (i==1) { s_.listened=1; go(SCHOOL_LISTEN); } else go(SCHOOL_PLAN);
  break;
 case SCHOOL_PAPER: if(i==0) { borrowPaper(s_); go(SCHOOL_PLAN); } else go(i==1 ? SCHOOL_INTRO : SCHOOL_PAPER); break;
 }
}
void Game::actCafe(int i) {
 if (s_.page==CAFE_CART || s_.page==CAFE_COMPARE) { actCafeTray(i); return; }
 switch (s_.page) {
 case CAFE_INTRO: go(i==0 ? CAFE_MENU : i==1 ? CAFE_COMPARE : CAFE_INTRO); break;
 case CAFE_MENU:
  if(s_.ordered) go(CAFE_RESULT);
  else { s_.menuItem=(uint8_t)i; go(CAFE_QUANTITY); }
  break;
 case CAFE_RESULT: go(i==0 ? MEMORY : i==1 ? CAFE_RESULT : CAFE_CART); break;
 }
}
void Game::actCafeTray(int i) {
 if (s_.ordered) { if (i==1) changed(); else go(i==0 ? CAFE_RESULT : MEMORY); return; }
 switch (s_.page) {
 case CAFE_CART: if (i<2) go(i==0 ? CAFE_MENU : CAFE_COMPARE); else if (canOrder(s_)) { s_.ordered=1; go(CAFE_RESULT); } else go(CAFE_MENU); break;
 case CAFE_COMPARE:
  if (i==0) { memcpy(s_.compare,s_.cart,4); s_.compared=1; changed(); }
  else go(i==1 ? CAFE_MENU : CAFE_CART);
  break;
 }
}
void Game::actPark(int i) {
 if (s_.page>=PARK_GUESS) { actParkGuess(i); return; }
 switch (s_.page) {
 case PARK_INTRO: if (i==0) { s_.listened=1; changed(); } else go(i==1 ? PARK_SEARCH : PARK_CLUE); break;
 case PARK_SEARCH: if (i==0) { s_.clues|=1<<s_.clue; go(PARK_CLUE); } else if (i==1) { s_.clue=(s_.clue+1)%3; changed(); } else go(PARK_GUESS); break;
 case PARK_CLUE: if(i==1) s_.guess=s_.clue; go(i==0 ? PARK_SEARCH : i==1 ? PARK_GUESS : PARK_CLUE); break;
 }
}
void Game::actParkGuess(int i) {
 switch (s_.page) {
 case PARK_GUESS:
  if (i==0) go(s_.guess==s_.variant ? PARK_FOUND : PARK_WRONG);
  else if (i==1) { s_.guess=(s_.guess+1)%3; changed(); } else go(PARK_SEARCH);
  break;
 case PARK_WRONG:
  s_.revised=1; if(i==1) s_.guess=(s_.guess+1)%3;
  go(i==0 ? PARK_SEARCH : i==1 ? PARK_GUESS : PARK_INTRO); break;
 case PARK_FOUND: go(i==0 ? MEMORY : i==1 ? PARK_CLUE : PARK_FOUND); break;
 }
}
void Game::actMemory(int i) {
 if (s_.page==MEMORY_EDIT) {
  if (i==0) s_.caption=(s_.caption+1)%3;
  else if (i==1) s_.sticker=(s_.sticker+1)%3;
  else { go(MEMORY); return; }
  changed(); return;
 }
 if (i==1) { go(MEMORY_EDIT); return; }
 if (i==0) complete(s_);
 else { s_.active=0; s_.recorded=1; s_.completed|=1<<s_.destination; }
 go(STOP);
}
bool Game::takeSave(const void** data, size_t* len, bool allowed) {
 if (!dirty_ || !allowed) return false;
 seal(s_); *data=&s_; *len=sizeof s_; dirty_=false; return true;
}
const char* Game::screenName() const { if (title_) return "toytown_title"; if (pause_) return "toytown_pause"; static char page[48]; snprintf(page,sizeof page,"toytown_%s",PAGES[s_.page]); return page; }
void Game::debugPrint() {
 printf("toytown page=%s doll=%u hair=%u skin=%u outfit=%u shoes=%u accessory=%u color=%u name=%u hairColor=%u category=%u destination=%u bag=%u pack=%u listened=%u outcome=%u repaired=%u total=%d compared=%u ordered=%u variant=%u clue=%u clues=%u guess=%u revised=%u active=%u recorded=%u count=%u resume=%u hangMask=%u picture=%u pictures=%u task=%u\n",
 screenName(),s_.doll,s_.hair,s_.skin,s_.outfit,s_.shoes,s_.accessory,s_.color,s_.dollName,s_.hairColor,s_.category,s_.destination,s_.bag,s_.packItem,s_.listened,s_.schoolOutcome,s_.repaired,total(s_),s_.compared,s_.ordered,s_.variant,s_.clue,s_.clues,s_.guess,s_.revised,s_.active,s_.recorded,s_.count,s_.resumePage,s_.schoolHangMask,s_.schoolPicture,s_.schoolPictures,s_.schoolTask);
}
} // namespace toytown
