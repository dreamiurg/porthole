#pragma once
#include "ui.h"
#include "state.h"
#include "gfx565.h"
namespace toytown { namespace sceneui {
constexpr ui::Box BACK={40,14,24,22}, DEPART={100,14,24,22};
inline int count(const State& s) {
 if(s.page==SCHOOL_INTRO || s.page==PACK) return 5;
 if(s.page==CAFE_MENU || s.page==SCHOOL_HANG) return 4;
 return 3;
}
inline ui::Box card(int i,int count) {
 if(count<=3) { const ui::Box b[]={{24,105,54,24},{82,105,54,24},{49,131,62,22}}; return b[i]; }
 if(count==4) { const ui::Box b[]={{35,88,44,30},{81,88,44,30},{37,120,40,27},{83,120,40,27}}; return b[i]; }
 const ui::Box b[]={{9,87,46,30},{57,87,46,30},{105,87,46,30},{37,119,40,28},{83,119,40,28}}; return b[i];
}
constexpr gfx565::Rect MINA_SPRITE={95,166,60,94}, THEO_SPRITE={318,166,60,94}, NORI_SPRITE={153,170,58,84};
// Keep the doll's right edge clear of the tray, including its quantity badges.
constexpr ui::Box TRAY={96,65,50,28};
constexpr ui::Box TRAY_TOUCH={96,64,50,22};
inline int place(const State& s) {
 if(s.page==BOOK && s.count) return s.entries[s.bookIndex%s.count].place;
 if(s.page>=SCHOOL_INTRO && s.page<=SCHOOL_REPAIR) return SCHOOL;
 if((s.page>=CAFE_INTRO && s.page<=CAFE_RESULT)||s.page==CAFE_QUANTITY) return CAFE;
 if(s.page>=PARK_INTRO && s.page<=PARK_FOUND) return PARK;
 return s.page<=PACK ? (int)HOME : (int)s.destination;
}
constexpr int CAFE_WALK_MAX_X=(TRAY.x*3-26)/3;
inline int walkX(const State& s,int x) {
 return place(s)==CAFE && x>CAFE_WALK_MAX_X ? CAFE_WALK_MAX_X : x;
}
constexpr ui::Box MINA={31,55,24,30}, THEO={106,55,24,30};
constexpr ui::Box NORI={49,56,24,28};
constexpr ui::Box FRAMES[]={{43,59,24,22},{69,59,24,22},{95,59,24,22}};
constexpr int EVIDENCE_X[]={50,430,318};
inline gfx565::Rect notebook(int variant) { return {EVIDENCE_X[variant]-13,239,26,24}; }
inline gfx565::Rect ribbon(int variant) { return {EVIDENCE_X[variant]-9,239,18,14}; }
constexpr ui::Box SPOTS[]={{12,64,24,22},{119,64,28,22},{80,72,32,22}};
}}
