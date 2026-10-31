#pragma once
#include "state.h"
#include "ui.h"
namespace toytown { namespace choices {
constexpr ui::Box DONE={90,15,32,22}, EXIT={42,15,24,22};
constexpr ui::Box PREVIOUS={16,66,29,28}, NEXT={115,66,29,28};
constexpr ui::Box LOOKS[]={{24,106,26,28},{53,106,26,28},{82,106,26,28},{111,106,26,28}};
constexpr ui::Box DOLLS[]={{38,43,40,42},{82,43,40,42},{38,87,40,42},{82,87,40,42}};
constexpr ui::Box WARDROBE={49,131,62,22};
constexpr int CATEGORIES=9;
inline const char* category(int c) {
 const char* names[]={"Hair","Skin","Fabric","Shoes","Accessory","Color","Name","Material","Hair color"};
 return names[c%CATEGORIES];
}
inline int count(int c) { const int counts[]={3,4,3,3,3,4,4,4,4}; return counts[c%CATEGORIES]; }
inline uint8_t& value(State& s,int c) {
 uint8_t* values[]={&s.hair,&s.skin,&s.outfit,&s.shoes,&s.accessory,&s.color,&s.dollName,&s.doll,&s.hairColor};
 return *values[c%CATEGORIES];
}
inline int value(const State& s,int c) { State copy=s; return value(copy,c); }
inline const char* label(int c,int i) {
 const char* labels[][4]={{"Loose","Buns","Pony",""},{"As is","Warm","Tan","Deep"},
 {"Plain","Lines","Plaid",""},{"As is","Rose","Dark",""},{"None","Specs","Petal",""},
 {"As is","Teal","Lilac","Gold"},{"Emi","Pip","Kai","Lulu"},{"Doll","Plush","Clay","Mini"},
 {"As is","Dark","Gold","Ink"}};
 return labels[c%CATEGORIES][i];
}
}} // namespace toytown::choices
