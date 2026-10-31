#include "game.h"
#include "art.h"
#include "scene_ui.h"
#include "font.h"
#include "gfx565.h"
#include <stdio.h>
namespace toytown {
namespace {
constexpr uint16_t INK=0x2928,PAPER=0xff9b,EDGE=0xc452,MINT=0xb733;
void label(const char* text,const gfx565::Rect& r) {
 const font::Box box={&art::ACTION,(int16_t)r.w,(int16_t)r.h,0,font::Align::CENTER};
 font::textBox(box,text,r.x,r.y,INK);
}
bool held(const Input& in,const ui::Box& b) {
 return in.down && in.x>=b.x && in.x<b.x+b.w && in.y>=b.y && in.y<b.y+b.h;
}
void frame(const ui::Box& b,bool selected,bool down) {
 gfx565::roundRect({b.x*3,b.y*3,b.w*3,b.h*3},12,INK);
 gfx565::roundRect({b.x*3+3,b.y*3+3,b.w*3-6,b.h*3-9},9,EDGE);
 gfx565::roundRect({b.x*3+6,b.y*3+6,b.w*3-12,b.h*3-15},7,selected || down ? MINT : PAPER);
}
void displayFrame(const State& s,int i,const Input& in) {
 const auto& b=sceneui::FRAMES[i];
 frame(b,false,held(in,b));
 if(s.schoolHangMask&(1<<i)) {
  const int picture=(s.schoolPictures>>(i*2))&3;
  art::prop(art::FLOWER_PICTURE+picture,{b.x*3+6,b.y*3+5,b.w*3-12,b.h*3-14});
 } else label("Tap",{b.x*3+6,b.y*3+22,b.w*3-12,26});
}
void displayCard(const State& s,int i,const Input& in) {
 const auto b=sceneui::card(i,4);
 frame(b,i<3 && s.schoolPicture==i,held(in,b));
 art::prop(i<3 ? art::FLOWER_PICTURE+i : art::HUNG_PICTURES,{b.x*3+9,b.y*3+7,b.w*3-18,b.h*3-43});
 const char* names[]={"Flowers","Rocket","Rabbit","Done"};
 label(names[i],{b.x*3+7,(b.y+b.h)*3-32,b.w*3-14,26});
}
int emptyFrames(const State& s) {
 int count=0;
 for(int i=0;i<3;++i) if(!(s.schoolHangMask&(1<<i))) ++count;
 return count;
}
}
void Game::openDisplay(bool repair) {
 stopWalk(); displayTried_=false;
 beginDisplay(s_,repair); go(SCHOOL_HANG);
}
void Game::actSchoolRepair(int i) {
 if(i==0) openDisplay(true);
 else if(i==1) { repairSchool(s_,2); changed(); beginWalk(61,80); }
 else go(MEMORY);
}
void Game::displayInput() {
 int picked=-1;
 for(int i=0;i<3;++i) if(ui::clicked(in_,sceneui::FRAMES[i])) picked=i;
 if(picked<0) return;
 hangPicture(s_,picked); displayTried_=false; changed();
}
void Game::actDisplay(int i) {
 if(i<3) { s_.schoolPicture=(uint8_t)i; displayTried_=false; changed(); return; }
 if(!displayReady(s_)) { displayTried_=true; changed(); return; }
 if(finishDisplay(s_)) { displayTried_=false; go(s_.page); }
}
void Game::drawDisplay() {
 art::background(2);
 gfx565::roundRect({198,57,194,35},13,PAPER);
 label("Art display",{202,61,186,28});
 gfx565::roundRect({78,108,324,53},10,PAPER);
 const char* speech="Theo: Choose a picture.\nThen tap a frame.";
 char empty[72];
 if(displayReady(s_)) speech="Theo: Our display looks great!\nMove pictures, or tap Done.";
 else if(displayTried_) {
  snprintf(empty,sizeof empty,"Theo: We have %d empty frames.\nTap a picture, then a frame.",emptyFrames(s_)); speech=empty;
 }
 label(speech,{84,112,312,48});
 gfx565::roundRect({126,172,234,84},8,INK);
 gfx565::roundRect({129,175,228,78},6,0xd5b2);
 art::walkingDoll(s_,{65,174,48,80},0);
 art::npc(art::THEO,{364,166,48,84},2);
 for(int i=0;i<3;++i) displayFrame(s_,i,in_);
 for(int i=0;i<4;++i) displayCard(s_,i,in_);
 frame(sceneui::BACK,false,held(in_,sceneui::BACK));
 label("<",{sceneui::BACK.x*3+3,sceneui::BACK.y*3+12,66,40});
}
}
