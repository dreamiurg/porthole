#include "game.h"
#include "art.h"
#include "content.h"
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
void button(const ui::Box& b, const char* label, bool pressed) {
 gfx565::Rect r={b.x*3,b.y*3,b.w*3,b.h*3};
 gfx565::roundRect(r,18,EDGE); gfx565::roundRect({r.x+2,r.y+2,r.w-4,r.h-4},16,pressed ? MINT : PAPER);
 const font::Box box={&art::BODY,(int16_t)(r.w-16),66,0,font::Align::CENTER};
 const int h=font::textHeight(art::BODY,label,r.w-16,0);
 font::textBox(box,label,r.x+8,r.y+(r.h-h)/2,INK);
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
 if (s_.page==STYLE) {
  const char* categories[]={"Hair","Skin","Fabric","Shoes","Accessory","Color","Name","Material"};
  const uint8_t values[]={s_.hair,s_.skin,s_.outfit,s_.shoes,s_.accessory,s_.color,s_.dollName,s_.doll};
  const char* styles[]={"loose","buns","side pony"}; const char* outfits[]={"plain","stripes","plaid"};
  const char* shoes[]={"original","rose","cocoa"}; const char* accessories[]={"none","glasses","flower"};
  const char* colors[]={"original","teal","lavender","sunshine"}; const char* skins[]={"original","peach","brown","deep"};
  const char* choices[]={styles[s_.hair],skins[s_.skin],outfits[s_.outfit],shoes[s_.shoes],accessories[s_.accessory],colors[s_.color],NAMES[s_.dollName],(s_.doll==0 ? "Fashion" : s_.doll==1 ? "Plush" : s_.doll==2 ? "Clay" : "Keychain")};
  const int c=s_.category%8; snprintf(heading_,sizeof heading_,"%s: %s",categories[c],choices[c]);
  snprintf(words_,sizeof words_,"%s's choice %u. Try anything. Your style never changes what you can do.",NAMES[s_.dollName],values[c]+1);
 }
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
  button(b,labels_[i],pressed);
 }
 button(BACK,"<",false);
}
void Game::draw() {
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
