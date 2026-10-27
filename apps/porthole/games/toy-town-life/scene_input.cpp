#include "game.h"
#include "scene_ui.h"
namespace toytown {
void Game::beginWalk(int x,int y) {
 if(walking_) {
  const int t=(int)(ms_-walkFrom_);
  walkX_+=(targetX_-walkX_)*t/600; walkY_+=(targetY_-walkY_)*t/600;
 } else { walkX_=s_.worldX; walkY_=s_.worldY; }
 targetX_=x; targetY_=y; walkFrom_=ms_; walking_=true; ++look_;
}
void Game::stopWalk() {
 if(!walking_) return;
 const int t=(int)(ms_-walkFrom_);
 const int x=walkX_+(targetX_-walkX_)*t/600;
 const int y=walkY_+(targetY_-walkY_)*t/600;
 moveTo(s_,(uint8_t)x,(uint8_t)y,s_.page==HUB ? (uint8_t)HOME : s_.destination);
 walking_=false; dirty_=true;
}
void Game::tickWalk() {
 if(!walking_ || pause_ || title_) return;
 ++look_;
 if(ms_-walkFrom_<600) return;
 walking_=false;
 moveTo(s_,(uint8_t)targetX_,(uint8_t)targetY_,s_.page==HUB ? (uint8_t)HOME : s_.destination);
 dirty_=true;
}
bool Game::overlayInput(int action) {
 if(title_) { title_=false; if(action==1) go(DOLL); else if(action==2) { home_=true; changed(); } else changed(); return true; }
 if(pause_) { pause_=false; if(action==1) go(HUB); else { home_=action==2; changed(); } return true; }
 return false;
}
bool Game::sceneDepart() {
 if(s_.page!=PACK && s_.page!=CAFE_MENU) return false;
 if(!ui::clicked(in_,sceneui::DEPART)) return false;
 if(s_.page==PACK) { start(s_,s_.destination); go(s_.page); }
 else go(CAFE_CART);
 return true;
}
bool Game::sceneNpc() {
 const bool school=s_.page>=SCHOOL_INTRO && s_.page<=SCHOOL_REPAIR;
 const bool park=s_.page>=PARK_INTRO && s_.page<=PARK_FOUND;
 if(s_.page<SCHOOL_INTRO || s_.page>PARK_FOUND) return false;
 const bool mina=ui::clicked(in_,park ? sceneui::NORI : sceneui::MINA);
 const bool theo=school && ui::clicked(in_,sceneui::THEO);
 if(mina) {
  beginWalk(61,80);
  if(school) {
   if(s_.page==SCHOOL_RESULT || s_.page==SCHOOL_REPAIR) go(SCHOOL_REPAIR);
   else { s_.listened=1; go(SCHOOL_LISTEN); }
  }
  else if(park) { s_.listened=1; go(PARK_INTRO); }
  else go(s_.ordered ? CAFE_RESULT : CAFE_INTRO);
  return true;
 }
 if(theo) { beginWalk(100,80); go(SCHOOL_OTHER); return true; }
 return false;
}
bool Game::sceneLandmark() {
 if(s_.page>=PARK_INTRO && s_.page<=PARK_FOUND) {
  int picked=-1;
  for(int i=0;i<3;++i) if(ui::clicked(in_,sceneui::SPOTS[i])) picked=i;
  if(picked>=0) {
   if(s_.page==PARK_WRONG) s_.revised=1;
   s_.clue=(uint8_t)picked; s_.clues|=1<<picked; go(PARK_CLUE);
   const int x[]={32,128,92}; beginWalk(x[picked],80);
   return true;
  }
 }
 return false;
}
void Game::sceneInput() {
 if(sceneDepart() || sceneNpc() || sceneLandmark()) return;
 // Floor is continuous walking space; controls above receive input first.
 if(!in_.tap) return;
 if(in_.downX<WORLD_MIN_X || in_.downX>WORLD_MAX_X) return;
 if(in_.downY<WORLD_MIN_Y || in_.downY>WORLD_MAX_Y) return;
 if(in_.x<WORLD_MIN_X || in_.x>WORLD_MAX_X) return;
 if(in_.y<WORLD_MIN_Y || in_.y>WORLD_MAX_Y) return;
 beginWalk(in_.x,in_.y);
}
}
