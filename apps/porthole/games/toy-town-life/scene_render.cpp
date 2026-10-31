#include "game.h"
#include "art.h"
#include "scene_ui.h"
#include "scene_pictures.h"
#include "content.h"
#include "font.h"
#include "gfx565.h"
#include <stdio.h>
#include <string.h>
namespace toytown {
namespace {
constexpr uint16_t INK=0x2928,PAPER=0xff9b,EDGE=0xc452,MINT=0xb733;
void text(const char* label,int x,int y,int w,int h) {
 const font::Box box={&art::ACTION,(int16_t)w,(int16_t)h,0,font::Align::CENTER};
 font::textBox(box,label,x,y,INK);
}
void frame(const ui::Box& b,bool down,bool selected=false) {
 gfx565::roundRect({b.x*3,b.y*3,b.w*3,b.h*3},13,INK);
 gfx565::roundRect({b.x*3+3,b.y*3+3,b.w*3-6,b.h*3-9},10,EDGE);
 gfx565::roundRect({b.x*3+6,b.y*3+6,b.w*3-12,b.h*3-15},8,down || selected ? MINT : PAPER);
}
bool held(const Input& in,const ui::Box& b) { return in.down && in.x>=b.x && in.x<b.x+b.w && in.y>=b.y && in.y<b.y+b.h; }
void schoolProps(const State& s) {
 art::prop(art::PICTURES,{366,164,59,83});
 if(s.schoolOutcome==4 || (s.repaired&1)) {
  if(!displayReady(s)) art::prop(art::HUNG_PICTURES,{392,164,60,91});
  else for(int i=0;i<3;++i) art::prop(art::FLOWER_PICTURE+((s.schoolPictures>>(2*i))&3),{399,164+i*30,44,29});
 }
 if(s.schoolOutcome==2 || (s.repaired&2)) art::prop(art::CUE_CARDS,{136,203,38,39});
 if(s.schoolOutcome==3) art::prop(art::PICTURES,{147,215,42,37});
 if(s.page==SCHOOL_PLAN || s.schoolOutcome==4) art::prop(art::CUE_CARDS,{156,236,37,29});
}
void parkProps() {
 for(int i=0;i<3;++i) {
  const auto& b=sceneui::SPOTS[i];
  gfx565::circle(b.x*3+36,b.y*3+33,15,PAPER);
  art::prop(art::MAGNIFIER,{b.x*3+23,b.y*3+20,26,26});
 }
}
void parkEvidence(const State& s) {
 art::prop(art::RIBBON,sceneui::ribbon(s.variant));
 if((s.clues&(1<<s.variant)) || s.page==PARK_FOUND)
  art::prop(art::NOTEBOOK,sceneui::notebook(s.variant));
}
void tray(const State& s) {
 const auto& b=sceneui::TRAY;
 art::prop(art::TRAY,{b.x*3,b.y*3,b.w*3,b.h*3-1});
 for(int i=0;i<4;++i) if(s.cart[i]) {
  art::prop(art::SANDWICH+i,{293+i*33,205,32,32});
  char q[8]; snprintf(q,sizeof q,"x%u",s.cart[i]);
  gfx565::roundRect({292+i*33,237,34,25},5,PAPER); text(q,292+i*33,237,34,24);
 }
}
const char* headerLabel(const State& s,int p,bool title,const char* heading) {
 if(title) return "Toy Town";
 if(s.page==MAP) return "Town";
 if(s.page==PACK) return "Bag";
 if(s.page==CAFE_CART || s.page==CAFE_COMPARE || s.page==CAFE_QUANTITY || s.page==BOOK) return heading;
 if(s.page>=PARK_SEARCH && s.page<=PARK_FOUND) return heading;
 return PLACES[p];
}
void sceneHeader(const State& s,int p,bool title,bool pause,const char* heading) {
 const bool depart=!title && !pause && (s.page==PACK || s.page==CAFE_MENU);
 const int w=depart ? 96 : 194;
 gfx565::roundRect({198,57,w,35},13,PAPER);
 text(headerLabel(s,p,title,heading),202,61,w-8,28);
}
void sceneActors(const State& s,int p) {
 if(p==SCHOOL) {
  const int pose=s.schoolOutcome==2 || (s.repaired&2) ? 2 : 1;
  art::npc(art::MINA,sceneui::MINA_SPRITE,pose);
  art::npc(art::THEO,sceneui::THEO_SPRITE,s.schoolOutcome==4 || (s.repaired&1) ? 2 : 0);
  schoolProps(s);
 } else if(p==CAFE) {
  art::npc(art::MINA,sceneui::MINA_SPRITE,s.ordered ? 0 : 1);
  art::npc(art::SERVER,{338,160,58,82}); tray(s);
 } else if(p==PARK) {
  const bool returned=s.page==MEMORY || s.page==MEMORY_EDIT || s.page==STOP || s.page==BOOK;
  art::npc(art::NORI,sceneui::NORI_SPRITE,returned ? 2 : 1);
  if(returned) art::prop(art::NOTEBOOK,{168,211,30,29});
  else parkProps();
 }
}
State rememberedScene(const State& current) {
 State s=current;
 if(s.page!=BOOK || !s.count) return s;
 const Entry& e=s.entries[s.bookIndex%s.count];
 s.destination=e.place; s.worldX=s.worldY=80; s.worldPlace=e.place;
 s.repaired=s.schoolPaper=0;
 s.schoolHangMask=s.schoolPicture=s.schoolPictures=s.schoolTask=0;
 s.schoolOutcome=e.place==SCHOOL ? e.outcome : 0;
 s.variant=e.place==PARK ? e.outcome : 0; s.clue=s.guess=s.variant;
 s.clues=1<<s.variant; memset(s.cart,0,sizeof s.cart);
 s.ordered=e.place==CAFE;
 return s;
}
void speech(const char* words) {
 gfx565::roundRect({78,108,324,53},10,PAPER);
 text(words,84,112,312,48);
 const bool theo=!strncmp(words,"Theo:",5), mina=!strncmp(words,"Mina:",5), nori=!strncmp(words,"Nori:",5);
 if(!theo && !mina && !nori) return;
 const int x=theo ? 334 : nori ? 174 : 121;
 for(int d=0;d<9;++d) gfx565::hline(x+d/2,160+d,12-d,PAPER);
}
}
void Game::drawSceneCard(int i,int n) {
  const auto b=sceneui::card(i,n);
  const bool packed=!title_ && !pause_ && s_.page==PACK && (s_.bag&(1<<i));
  frame(b,held(in_,b),packed || (!title_ && !pause_ && s_.page==SCHOOL_INTRO && i==0));
  const int w=b.w*3-14;
  const int h=font::textHeight(art::ACTION,labels_[i],w,0);
  const int pictureH=b.h*3-h-14;
  const gfx565::Rect picture={b.x*3+8,b.y*3+5,w,pictureH};
  if(pictureH>=12) {
   if(title_ || pause_) {
    if(i==1 && title_) art::detail(s_,picture,0);
    else art::placePicture(i==0 ? s_.destination+1 : 0,picture);
   } else sceneui::picture(s_,i,picture);
  }
  text(labels_[i],b.x*3+7,(b.y+b.h)*3-h-9,w,h);
}
void Game::drawSceneCards() {
 const int n=title_ || pause_ ? 3 : sceneui::count(s_);
 for(int i=0;i<n;++i) drawSceneCard(i,n);
 frame(sceneui::BACK,held(in_,sceneui::BACK)); text("<",sceneui::BACK.x*3+3,sceneui::BACK.y*3+12,66,40);
 if(!title_ && !pause_ && (s_.page==PACK || s_.page==CAFE_MENU)) {
  frame(sceneui::DEPART,held(in_,sceneui::DEPART));
  text(s_.page==PACK ? "Go" : "Tray",sceneui::DEPART.x*3+6,sceneui::DEPART.y*3+17,60,30);
 }
}
void Game::drawScene() {
 if(!title_ && !pause_ && s_.page==SCHOOL_HANG) { drawDisplay(); return; }
 const State visual=rememberedScene(s_);
 const int p=title_ ? (int)HOME : sceneui::place(visual);
 art::background(title_ || s_.page==MAP ? 0 : p==HOME ? 1 : p+1);
 sceneHeader(visual,p,title_,pause_,heading_); sceneActors(visual,p);
 int x=visual.worldX,y=visual.worldY;
 if(walking_) { const int t=(int)(ms_-walkFrom_); x=walkX_+(targetX_-walkX_)*t/600; y=walkY_+(targetY_-walkY_)*t/600; }
 x=sceneui::walkX(visual,x);
 art::walkingDoll(visual,{x*3-26,y*3-66,52,86},walking_ ? (int)(ms_-walkFrom_) : 0);
 if(p==PARK && visual.page>=PARK_INTRO && visual.page<=PARK_FOUND) parkEvidence(visual);
 if(p==SCHOOL && visual.schoolPaper) art::prop(art::PAPER,{x*3+12,y*3-28,26,23});
 speech(words_);
 drawSceneCards();
}
}
