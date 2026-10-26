#include "game.h"
#include "art.h"
#include "content.h"
#include "choices.h"
#include "font.h"
#include "gfx565.h"
#include <stdio.h>
#include <string.h>

namespace toytown {
namespace {
constexpr uint16_t INK=0x2928, PAPER=0xff9b, EDGE=0xc452, PINK=0xf695, MINT=0xb733, BLUE=0xbedf;
constexpr ui::Box BUTTONS[] = {{24,105,54,24},{82,105,54,24},{49,131,62,22}};
constexpr ui::Box BACK={68,8,24,22};
void copy(char* to, size_t n, const char* from) { snprintf(to,n,"%s",from); }
void button(const ui::Box& b, const char* label, bool pressed, bool primary=false) {
 gfx565::Rect r={b.x*3,b.y*3,b.w*3,b.h*3};
 gfx565::roundRect(r,18,INK);
 gfx565::roundRect({r.x+3,r.y+3,r.w-6,r.h-9},16,EDGE);
 gfx565::roundRect({r.x+6,r.y+6+(pressed ? 3 : 0),r.w-12,r.h-17},13,pressed ? BLUE : primary ? MINT : PAPER);
 const font::Box box={&art::BODY,(int16_t)(r.w-16),66,0,font::Align::CENTER};
 const int h=font::textHeight(art::BODY,label,r.w-16,0);
 font::textBox(box,label,r.x+8,r.y+(r.h-h)/2,INK);
}
void caption(const char* text,int x,int y,int w,bool title=false) {
 const font::Box box={title ? &art::TITLE : &art::BODY,(int16_t)w,30,0,font::Align::CENTER};
 font::textBox(box,text,x,y,INK);
}
void pictureFrame(const ui::Box& b,bool selected,bool pressed) {
 const int x=b.x*3,y=b.y*3,w=b.w*3,h=b.h*3;
 gfx565::roundRect({x,y,w,h},14,INK);
 gfx565::roundRect({x+3,y+3,w-6,h-9},12,selected ? 0x456b : EDGE);
 gfx565::roundRect({x+7,y+7+(pressed ? 3 : 0),w-14,h-17},9,selected || pressed ? MINT : PAPER);
}
void selectedMark(const ui::Box& b) {
 const int x=(b.x+b.w)*3-15,y=b.y*3+14;
 gfx565::circle(x,y,10,0x456b);
 for (int i=0;i<3;++i) { gfx565::pixel(x-5+i,y+i,PAPER); gfx565::pixel(x-5+i,y+i+1,PAPER); }
 for (int i=0;i<7;++i) { gfx565::pixel(x-3+i,y+2-i,PAPER); gfx565::pixel(x-3+i,y+3-i,PAPER); }
}
bool pressed(const Input& in,const ui::Box& b) { return in.down && in.x>=b.x && in.x<b.x+b.w && in.y>=b.y && in.y<b.y+b.h; }
int detailPart(int category) { return category==2 || category==5 ? 2 : category==3 ? 3 : category==7 ? 0 : 1; }
void optionPicture(const State& state,int category,const ui::Box& b,bool tab=false) {
 State s=state;
 const int x=b.x*3+9,y=b.y*3+9,w=b.w*3-18,h=b.h*3-43;
 if (category==1 || category==5 || category==8) {
  if (choices::value(s,category)==0) art::detail(s,{x,y,w,h},detailPart(category));
  else { gfx565::circle(x+w/2,y+h/2,h/2-2,INK); gfx565::circle(x+w/2,y+h/2,h/2-5,art::swatch(s,category)); }
 } else if (category==6) caption(tab ? "Aa" : choices::label(category,s.dollName),x,y+h/2-10,w);
 else art::detail(s,{x,y,w,h},detailPart(category));
}
int trayTotal(const uint8_t* c) { return c[0]*5+c[1]*3+c[2]+c[3]*2; }
int sceneFor(const State& s,uint8_t destination) {
 if (s.page>=SCHOOL_INTRO && s.page<=SCHOOL_REPAIR) return 2;
 if (s.page>=CAFE_INTRO && s.page<=CAFE_RESULT) return 3;
 if (s.page>=PARK_INTRO && s.page<=PARK_FOUND) return 4;
 if (s.page==DOLL || s.page==STYLE) return 1;
 if (s.page==BOOK && s.count) return s.entries[s.bookIndex%s.count].place+1;
 return s.page==MAP ? destination+1 : 0;
}
int parkSpot(const State& s) {
 if (s.page==PARK_GUESS || s.page==PARK_WRONG) return s.guess;
 return s.page==PARK_FOUND ? s.variant : s.clue;
}
void ribbon(int x,int y) {
 gfx565::roundRect({x,y,22,13},3,PINK); gfx565::circle(x+3,y+6,8,PINK);
 gfx565::circle(x+20,y+6,8,PINK); gfx565::rect(x+9,y+9,5,17,PINK);
}
void landmark(int spot, bool evidence) {
 const int x=300,y=94;
 if (spot==0) { gfx565::roundRect({x-50,y,100,13},4,0xba48); gfx565::rect(x-40,y+13,9,30,0x7185); gfx565::rect(x+32,y+13,9,30,0x7185); }
 if (spot==1) { gfx565::circle(x,y+23,44,0xaebd); gfx565::circle(x,y+18,29,0x769a); gfx565::rect(x-5,y-30,10,48,0x86bc); }
 if (spot==2) { gfx565::rect(x+12,y-38,14,66,0xaa26); gfx565::circle(x+18,y-37,32,0x6c6c); gfx565::roundRect({x-60,y+23,130,15},7,0xe6d2); }
 if (evidence) { gfx565::roundRect({x-25,y+23,42,28},3,0xfef7); ribbon(x-15,y+22); }
}
}
void Game::describe() {
 const Scene& scene=SCENES[s_.page]; copy(heading_,sizeof heading_,scene.title); copy(words_,sizeof words_,scene.words);
 for (int i=0;i<3;i++) copy(labels_[i],sizeof labels_[i],scene.actions[i]);
 if (s_.page<=PACK || s_.page==BOOK) describeHome();
 describeSchool();
 if (s_.page>=CAFE_INTRO && s_.page<=CAFE_RESULT) describeCafe();
 if (s_.page>=PARK_INTRO && s_.page<=PARK_FOUND) describePark();
 if (s_.page==MEMORY) snprintf(words_,sizeof words_,"Keep a memory, or skip.\n%s\nSticker: %s",CAPTIONS[s_.caption],STICKERS[s_.sticker]);
 if (s_.page==MEMORY_EDIT) snprintf(words_,sizeof words_,"%s\nSticker: %s\nTry another detail.",CAPTIONS[s_.caption],STICKERS[s_.sticker]);
 describeOverlay();
}
void Game::describeSchool() {
 if (s_.page==SCHOOL_RESULT) {
  const char* endings[]={"", "Mina feels less alone.\nShe pauses at her start.\nTheo's display waits.", "Cue cards help Mina start.\nTheo hangs fewer pictures.", "Your picture cards look fun.\nMina forgets her start.\nAsk what would help.", "Theo hangs pictures first.\nMina waits, then practices\nher first words with him."};
  copy(words_,sizeof words_,endings[s_.schoolOutcome]);
 }
 if (s_.page==SCHOOL_PAPER) copy(labels_[1],sizeof labels_[1],"Another plan");
 if (s_.page==SCHOOL_INTRO) copy(words_,sizeof words_,"Mina: Will you sit with me? I'm nervous! You promised Theo help, too.");
 if (s_.page==SCHOOL_OTHER) copy(words_,sizeof words_,"Theo: Help hang pictures first? Then I can rehearse with Mina. She will wait.");
 if (s_.page==SCHOOL_REPAIR) {
  const char* text[]={"A promise still needs care. What shall we do after class?","You help Theo finish. He smiles: We made room for both promises.","Mina: Next time, ask me first. We can practice together after class."};
  copy(words_,sizeof words_,text[s_.repaired]);
 }
}
void Game::describeOverlay() {
 if (title_) {
  copy(heading_,sizeof heading_,"Toy Town Life"); copy(words_,sizeof words_,"Your doll. Your plans. Three little stories in a town that remembers.");
  copy(labels_[0],sizeof labels_[0],s_.active ? "Continue my story" : "Enter Toy Town"); copy(labels_[1],sizeof labels_[1],"Meet my doll"); copy(labels_[2],sizeof labels_[2],"Leave");
 }
 if (pause_) {
  copy(heading_,sizeof heading_,"A little pause"); copy(words_,sizeof words_,"Everything stays just here. Your story will wait for you.");
  copy(labels_[0],sizeof labels_[0],"Resume"); copy(labels_[1],sizeof labels_[1],"Town"); copy(labels_[2],sizeof labels_[2],"Save & leave");
 }
}
void Game::describeHome() {
 if (s_.page==HUB && s_.active) { copy(labels_[0],sizeof labels_[0],"Resume my outing"); copy(words_,sizeof words_,"Your outing is waiting. Dress up, read a memory, or pick up your story."); }
 if (s_.page==DOLL) { snprintf(words_,sizeof words_,"%s, the %s. Every material can play every story.",NAMES[s_.dollName],DOLLS[s_.doll]); }
 if (s_.page==STYLE) describeStyle();
 if (s_.page==MAP) { snprintf(heading_,sizeof heading_,"Let's visit %s",PLACES[destination_]); copy(words_,sizeof words_,"Choose a place, then pack up to three things for your outing."); }
 if (s_.page==PACK) {
  snprintf(heading_,sizeof heading_,"Bag: %d of 3 packed",slots(s_));
  snprintf(words_,sizeof words_,"%s: %s. %s",ITEMS[s_.packItem],(s_.bag&(1<<s_.packItem)) ? "packed" : "not packed",slots(s_)==3 ? "Bag full? Unpack one to swap." : "Forgot something? You can borrow.");
 }
 if (s_.page==BOOK) {
  if (!s_.count) copy(words_,sizeof words_,"Blank pages for your stories. A memory can be kept after an outing.");
  else { const Entry& e=s_.entries[s_.bookIndex%s_.count]; snprintf(heading_,sizeof heading_,"%s | memory %u",PLACES[e.place],s_.bookIndex%s_.count+1); describeEntry(e); }
 }
}
void Game::describeEntry(const Entry& e) {
 const char* school[]={"", "We sat beside Mina.", "Cue cards helped Mina.", "We tried picture cards.", "Theo helped after displays."};
 char memory[80];
 if (e.place==SCHOOL) copy(memory,sizeof memory,school[e.outcome]);
 else if (e.place==CAFE) snprintf(memory,sizeof memory,"Lunch: %u coins left.",e.outcome);
 else snprintf(memory,sizeof memory,"Found by the %s.",SPOTS[e.outcome]);
 snprintf(words_,sizeof words_,"%s\n%s\nSticker: %s",memory,CAPTIONS[e.caption],STICKERS[e.sticker]);
}
void Game::describeStyle() {
 const int c=s_.category;
 snprintf(heading_,sizeof heading_,"%s: %s",choices::category(c),choices::label(c,choices::value(s_,c)));
 copy(words_,sizeof words_,"Tap a look to wear it.");
}
void Game::describeCafe() {
 if (s_.page==CAFE_MENU) {
  snprintf(heading_,sizeof heading_,"%s | %d coins",FOOD[s_.menuItem],PRICES[s_.menuItem]);
  snprintf(words_,sizeof words_,"Qty: %u | Tray: %d/10\nSandwich 5 | Fruit 3\nWater 1 | Cookie 2",s_.cart[s_.menuItem],total(s_));
 }
 if (s_.page==CAFE_CART) {
  snprintf(heading_,sizeof heading_,"Tray: %d / 10 coins",total(s_));
  if (total(s_)>10) snprintf(words_,sizeof words_,"Need %d more coins. Change a quantity to fit our pretend budget.",total(s_)-10);
  else if (!canOrder(s_)) copy(words_,sizeof words_,"Lunch needs food and water. Add a sandwich or fruit, and water for Mina.");
  else snprintf(words_,sizeof words_,"Food and water are ready. %d coins left. We can change our plan or order.",10-total(s_));
 }
 if (s_.page==CAFE_COMPARE) {
  if (s_.compared) {
   const int difference=total(s_)-trayTotal(s_.compare); char comparison[40];
   if (!difference) copy(comparison,sizeof comparison,"the same cost");
   else snprintf(comparison,sizeof comparison,"%d %s coins",difference<0 ? -difference : difference,difference<0 ? "fewer" : "more");
   snprintf(words_,sizeof words_,"Saved idea: %d coins.\nThis tray: %d coins.\nThat's %s.",trayTotal(s_.compare),total(s_),comparison);
  }
  else copy(words_,sizeof words_,"Keep this tray as an idea. Change quantities, then compare the two costs.");
 }
 if (s_.ordered && s_.page==CAFE_CART) {
  copy(labels_[0],sizeof labels_[0],"Back to lunch"); copy(labels_[1],sizeof labels_[1],"Read again"); copy(labels_[2],sizeof labels_[2],"Remember");
 }
 if (s_.page==CAFE_RESULT) snprintf(words_,sizeof words_,"Mina: Water, just right! Lunch cost %d coins. %d coins left.",total(s_),10-total(s_));
}
void Game::describePark() {
 if (s_.page==PARK_INTRO && s_.listened) copy(words_,sizeof words_,LAST_SEEN[s_.variant]);
 if (s_.page==PARK_SEARCH) { snprintf(heading_,sizeof heading_,"Look: %s",SPOTS[s_.clue]); copy(words_,sizeof words_,"Notice the scene. Is there a pink ribbon? Look closely before choosing."); }
 if (s_.page==PARK_CLUE) {
  if (!(s_.clues&(1<<s_.clue))) copy(words_,sizeof words_,"No clue checked here yet. Look closely at the bench, fountain or path.");
  else copy(words_,sizeof words_,s_.clue==s_.variant ? CLUES[s_.clue] : EMPTY_CLUES[s_.clue]);
 }
 if (s_.page==PARK_GUESS) { snprintf(heading_,sizeof heading_,"My idea: %s",SPOTS[s_.guess]); copy(words_,sizeof words_,"We can test an idea and change it. Where might Nori's sketchbook be?"); }
 if (s_.page==PARK_WRONG) copy(words_,sizeof words_,"No sketchbook here. A useful clue! We can look again and change our idea.");
 if (s_.page==PARK_CLUE && (s_.bag&16) && (s_.clues&(1<<s_.clue)) && s_.clue==s_.variant) copy(words_,sizeof words_,"Your magnifier shows a pink ribbon on a little sketchbook. A closer look!");
 if (s_.page==PARK_FOUND) snprintf(words_,sizeof words_,"Found by the %s!\nA pink-ribbon sketchbook!\nLet's return it to Nori.",SPOTS[s_.variant]);
}
void Game::drawHero() {
 const int scene=sceneFor(s_,destination_);
 art::background(scene); art::doll(s_,120,12,1);
 if (s_.page>=PARK_SEARCH && s_.page<=PARK_FOUND) {
  const int spot=parkSpot(s_);
  landmark(spot,spot==s_.variant && (s_.clues&(1<<spot) || s_.page==PARK_FOUND));
 }
 if (s_.page==BOOK && s_.count) { const Entry& e=s_.entries[s_.bookIndex%s_.count]; if (e.place==PARK) landmark(e.outcome,true); }
 if (s_.page==STYLE) { gfx565::circle(314,87,21,PINK); gfx565::circle(354,100,19,MINT); gfx565::circle(328,126,16,BLUE); }
}
void Game::drawButtons() {
 for (int i=0;i<3;i++) {
  const ui::Box& b=BUTTONS[i]; const bool pressed=in_.down && in_.x>=b.x && in_.x<b.x+b.w && in_.y>=b.y && in_.y<b.y+b.h;
  button(b,labels_[i],pressed,(title_ || s_.page==HUB) && i==0);
 }
 button(BACK,"<",false);
}
void Game::drawDress() {
 art::background(1);
 if (s_.page==DOLL) {
  gfx565::roundRect({92,94,296,31},13,PAPER); caption("Choose your doll",95,96,290);
  for (int i=0;i<4;++i) {
   const auto& b=choices::DOLLS[i]; State look=s_; look.doll=i;
   pictureFrame(b,s_.doll==i,pressed(in_,b));
   art::detail(look,{b.x*3+8,b.y*3+7,b.w*3-16,95},0);
   caption(choices::label(7,i),b.x*3+5,(b.y+b.h)*3-32,b.w*3-10);
   if (s_.doll==i) selectedMark(b);
  }
  button(BACK,"<",pressed(in_,BACK));
  button(choices::WARDROBE,"Dress up",pressed(in_,choices::WARDROBE),true);
  return;
 }
 button(choices::EXIT,"<",pressed(in_,choices::EXIT));
 button(choices::DONE,"Done",pressed(in_,choices::DONE),true);
 gfx565::roundRect({120,116,240,31},13,PAPER);
 char title[48]; snprintf(title,sizeof title,"%s %u/9",choices::category(s_.category),s_.category+1);
 caption(title,125,119,230);
 gfx565::roundRect({177,151,126,166},20,PAPER);
 art::doll(s_,192,153,1);
 const ui::Box tabs[]={choices::PREVIOUS,choices::NEXT};
 for (int i=0;i<2;++i) {
  const auto& b=tabs[i]; const int c=(s_.category+(i ? 1 : choices::CATEGORIES-1))%choices::CATEGORIES;
  State icon=s_; if (c==4) icon.accessory=1;
  pictureFrame(b,false,pressed(in_,b)); optionPicture(icon,c,b,true);
  const char* tabNames[]={"Hair","Skin","Fabric","Shoes","Extras","Color","Name","Doll","Dye"};
  caption(tabNames[c],b.x*3+4,(b.y+b.h)*3-32,b.w*3-8);
  const int ax=(b.x+(i ? b.w-5 : 5))*3,ay=b.y*3+15;
  for (int d=0;d<5;++d) gfx565::hline(ax+(i ? -d : d),ay+d,2,INK);
 }
 for (int i=0;i<choices::count(s_.category);++i) {
  const auto& b=choices::LOOKS[i]; State look=s_; choices::value(look,s_.category)=i;
  pictureFrame(b,choices::value(s_,s_.category)==i,pressed(in_,b));
  optionPicture(look,s_.category,b);
  if (s_.category!=6) caption(choices::label(s_.category,i),b.x*3+4,(b.y+b.h)*3-32,b.w*3-8);
  if (choices::value(s_,s_.category)==i) selectedMark(b);
 }
 gfx565::roundRect({144,423,192,28},12,PAPER); caption("Tap to try",145,427,190);
}
void Game::draw() {
 if (!title_ && !pause_ && (s_.page==DOLL || s_.page==STYLE)) { drawDress(); return; }
 if (!pause_ && (title_ || s_.page==HUB)) {
  art::background(0);
  gfx565::roundRect({90,97,300,34},14,PAPER); caption("Toy Town Life",94,102,292,true);
  art::detail(s_,{165,133,150,174},0);
  drawButtons(); return;
 }
 drawHero();
 gfx565::roundRect({32,180,416,300},30,PAPER);
 const font::Box head={&art::TITLE,344,34,0,font::Align::CENTER};
 font::textBox(head,heading_,68,188,INK);
 const font::Box prose={&art::BODY,324,84,0,font::Align::CENTER};
 font::textBox(prose,words_,78,228,INK);
 drawButtons();
}
void Game::render() {
 int slot=buffers_[0]==gfx565::fb ? 0 : buffers_[1]==gfx565::fb ? 1 : buffers_[0] ? 1 : 0;
 const uint32_t revision=(look_<<17) ^ (in_.down ? 0x10000u|((uint32_t)in_.x<<8)|(uint32_t)in_.y : 0);
 if (buffers_[slot]!=gfx565::fb || painted_[slot]!=revision || UiAudit::enabled) {
  draw(); buffers_[slot]=gfx565::fb; painted_[slot]=revision;
 }
}
} // namespace toytown
