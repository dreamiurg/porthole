#include "game.h"
#include "content.h"
#include <stdio.h>
namespace toytown {
void Game::describeSceneSchool() {
 if(s_.page==SCHOOL_INTRO) {
  snprintf(words_,sizeof words_,"Theo: Can we hang my pictures?");
  const char* names[]={"Help Theo\nhang pictures","Plan with\nTheo","Ask Mina\nfor help","Practice\nwith Mina","Borrow\npaper"};
  for(int i=0;i<5;++i) snprintf(labels_[i],sizeof labels_[i],"%s",names[i]);
 }
 if(s_.page==SCHOOL_LISTEN) snprintf(words_,sizeof words_,"Mina: Yes! I need a note.\nMy first words are tricky.");
 if(s_.page==SCHOOL_OTHER) {
  snprintf(words_,sizeof words_,"Theo: Hang pictures first?\nThen I can practice with Mina.");
  snprintf(labels_[0],sizeof labels_[0],"Hang pictures");
  snprintf(labels_[1],sizeof labels_[1],"Ask Mina");
  snprintf(labels_[2],sizeof labels_[2],"Other plans");
 }
 if(s_.page==SCHOOL_PLAN) {
  snprintf(words_,sizeof words_,s_.listened ? "Mina: A note helps me start.\nWhat shall we try?" : "Mina: Let us rehearse my start.\nWhat shall we try?");
  snprintf(labels_[2],sizeof labels_[2],s_.listened ? "Picture cards" : "Ask Mina first");
 }
 if(s_.page==SCHOOL_PAPER) {
  snprintf(words_,sizeof words_,"Teacher: Here is spare paper.\nNow choose your plan.");
  snprintf(labels_[0],sizeof labels_[0],"Make a plan");
  snprintf(labels_[1],sizeof labels_[1],"School");
  snprintf(labels_[2],sizeof labels_[2],"Read again");
 }
}
void Game::describeScene() {
 describeSceneSchool();
 if(s_.page==MAP) for(int i=0;i<3;++i) snprintf(labels_[i],sizeof labels_[i],"%s",PLACES[i+1]);
 if(s_.page==PACK) {
  for(int i=0;i<5;++i) snprintf(labels_[i],sizeof labels_[i],"%s%s",(s_.bag&(1<<i)) ? "+ " : "",ITEMS[i]);
  snprintf(words_,sizeof words_,"My bag: %d of 3.\nTap to pack or unpack.",slots(s_));
 }
 if(s_.page==CAFE_MENU) {
  for(int i=0;i<4;++i) snprintf(labels_[i],sizeof labels_[i],"%s\n%d coin%s",FOOD[i],PRICES[i],PRICES[i]==1 ? "" : "s");
  snprintf(words_,sizeof words_,"Mina: Sandwich or fruit,\nplus water. Budget: 10 coins.");
 }
 if(s_.page==CAFE_QUANTITY) {
  snprintf(heading_,sizeof heading_,"%s",FOOD[s_.menuItem]);
  snprintf(words_,sizeof words_,"Coins: %u x %d = %d\nTray %d | Left %d",s_.cart[s_.menuItem],PRICES[s_.menuItem],s_.cart[s_.menuItem]*PRICES[s_.menuItem],total(s_),10-total(s_));
  if(total(s_)>10) snprintf(words_,sizeof words_,"Coins: %u x %d = %d\nTray %d: %d over budget",s_.cart[s_.menuItem],PRICES[s_.menuItem],s_.cart[s_.menuItem]*PRICES[s_.menuItem],total(s_),total(s_)-10);
  snprintf(labels_[0],sizeof labels_[0],"- Remove");
  snprintf(labels_[1],sizeof labels_[1],"+ Add");
  snprintf(labels_[2],sizeof labels_[2],"Done");
 }
 if(s_.page==STOP) snprintf(words_,sizeof words_,"Your town is saved.\nStop here, or keep exploring.");
 if(s_.page==HUB && !s_.active) snprintf(words_,sizeof words_,"Tap the floor to walk.\nPick a picture for your day.");
}
}
