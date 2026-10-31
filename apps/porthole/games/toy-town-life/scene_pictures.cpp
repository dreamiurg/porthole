#include "scene_pictures.h"
#include "art.h"
namespace toytown { namespace sceneui {
namespace {
gfx565::Rect half(const gfx565::Rect& r,bool right) {
 return {r.x+(right ? r.w/2 : 0),r.y,r.w/2,r.h};
}
void together(const State& s,int who,const gfx565::Rect& r) {
 art::npcPortrait(who,half(r,false)); art::detail(s,half(r,true),1);
}
void task(int who,int item,const gfx565::Rect& r) {
 art::npcPortrait(who,half(r,false)); art::prop(item,half(r,true));
}
void intro(const State& s,int i,const gfx565::Rect& r) {
 if(i==0) task(art::THEO,art::PICTURES,r);
 else if(i==1) together(s,art::THEO,r);
 else if(i==2) together(s,art::MINA,r);
 else if(i==3) task(art::MINA,art::CUE_CARDS,r);
 else art::prop(art::PAPER,r);
}
void planning(const State& s,int i,const gfx565::Rect& r) {
 if(i==0) together(s,art::MINA,r);
 else if(i==1) art::prop(art::CUE_CARDS,r);
 else if(s.listened) art::prop(art::PICTURES,r);
 else art::npcPortrait(art::MINA,r);
}
bool school(const State& s,int i,const gfx565::Rect& r) {
 if(s.page==SCHOOL_INTRO) intro(s,i,r);
 else if(s.page==SCHOOL_PLAN) planning(s,i,r);
 else if(s.page==SCHOOL_LISTEN) {
  if(i==0) together(s,art::MINA,r);
  else if(i==1) art::prop(art::BOOK,r);
  else task(art::THEO,art::PICTURES,r);
 } else if(s.page==SCHOOL_OTHER) {
  if(i==0) task(art::THEO,art::PICTURES,r);
  else if(i==1) art::npcPortrait(art::MINA,r);
  else art::prop(art::CUE_CARDS,r);
 } else return false;
 return true;
}
bool schoolEnding(const State& s,int i,const gfx565::Rect& r) {
 if(s.page==SCHOOL_PAPER) {
  if(i==1) art::placePicture(2,r);
  else art::prop(i==0 ? art::CUE_CARDS : art::PAPER,r);
 } else if(s.page==SCHOOL_RESULT) {
  if(i==0) { art::npcPortrait(art::THEO,half(r,false)); art::npcPortrait(art::MINA,half(r,true)); }
  else art::prop(art::BOOK,r);
 } else if(s.page==SCHOOL_REPAIR) {
  if(i==2) art::prop(art::BOOK,r);
  else task(i==0 ? art::THEO : art::MINA,i==0 ? art::HUNG_PICTURES : art::CUE_CARDS,r);
 } else return false;
 return true;
}
void comparison(int i,const gfx565::Rect& r) {
 if(i==1) { art::prop(art::TRAY,half(r,false)); art::prop(art::TRAY,half(r,true)); }
 else art::prop(i==0 ? art::SANDWICH : art::TRAY,r);
}
bool cafe(const State& s,int i,const gfx565::Rect& r) {
 if(s.page==CAFE_MENU) art::prop(art::SANDWICH+i,r);
 else if(s.page==CAFE_QUANTITY) art::prop(i==2 ? art::TRAY : art::SANDWICH+s.menuItem,r);
 else if(s.page==CAFE_INTRO) {
  if(i==2) art::npcPortrait(art::MINA,r); else comparison(i,r);
 } else if(s.page==CAFE_CART) comparison(i,r);
 else if(s.page==CAFE_COMPARE) art::prop(i==0 ? art::BOOK : i==1 ? art::SANDWICH : art::TRAY,r);
 else if(s.page==CAFE_RESULT) {
  if(i==1) art::npcPortrait(art::MINA,r); else art::prop(i==0 ? art::BOOK : art::TRAY,r);
 } else return false;
 return true;
}
bool park(const State& s,int i,const gfx565::Rect& r) {
 if(s.page<PARK_INTRO || s.page>PARK_FOUND) return false;
 if(s.page==PARK_FOUND && i==0) { task(art::NORI,art::NOTEBOOK,r); return true; }
 if((s.page==PARK_INTRO && i==0) || (s.page==PARK_WRONG && i==2)) {
  art::npcPortrait(art::NORI,r); return true;
 }
 const int items[][3]={{art::BOOK,art::MAGNIFIER,art::RIBBON},
  {art::MAGNIFIER,art::PICTURES,art::NOTEBOOK},{art::MAGNIFIER,art::NOTEBOOK,art::RIBBON},
  {art::MAGNIFIER,art::PICTURES,art::RIBBON},{art::MAGNIFIER,art::PICTURES,art::BOOK},
  {art::NOTEBOOK,art::RIBBON,art::BOOK}};
 art::prop(items[s.page-PARK_INTRO][i],r); return true;
}
void atHome(const State& s,int i,const gfx565::Rect& r) {
 if(s.page==HUB) {
  if(i==1) art::detail(s,r,0);
  else if(i==0) art::placePicture(0,r);
  else art::prop(art::BOOK,r);
 } else if(s.page==MAP) art::placePicture(i+2,r);
 else if(s.page==PACK) {
  const int items[]={art::NOTEBOOK,art::PENCIL,art::WATER,art::PICNIC_CLOTH,art::MAGNIFIER};
  art::prop(items[i],r);
 } else if(s.page==STOP && i<2) art::placePicture(0,r);
 else art::prop(i==1 ? art::PENCIL : art::BOOK,r);
}
}
void picture(const State& s,int i,const gfx565::Rect& r) {
 if(school(s,i,r) || schoolEnding(s,i,r) || cafe(s,i,r) || park(s,i,r)) return;
 atHome(s,i,r);
}
}}
