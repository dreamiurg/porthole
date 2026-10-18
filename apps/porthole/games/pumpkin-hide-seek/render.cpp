#include "render.h"
#include "ghost.h"
#include "pie.h"
#include "rules.h"
#include "gfx565.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
namespace pumpkin { namespace paint { namespace {
constexpr uint16_t rgb(unsigned c) { return ((c>>8)&0xf800)|((c>>5)&0x7e0)|((c>>3)&31); }
constexpr uint16_t INK=rgb(0x201d32),WALL=rgb(0x463451),PURPLE=rgb(0x815573),LILAC=rgb(0xb9809a);
constexpr uint16_t WOOD=rgb(0x925743),FLOOR=rgb(0x633e43),ORANGE=rgb(0xf58b32),RUST=rgb(0xb74f30);
constexpr uint16_t GOLD=rgb(0xffc875),CREAM=rgb(0xffedc2),SAGE=rgb(0x87a878),GREEN=rgb(0x49674d);
constexpr uint16_t NIGHT=rgb(0x263449),BLUE=rgb(0x53637a),SKIN=rgb(0xe6b79d),GREY=rgb(0xb6acb4),BEAM=rgb(0x997752);
constexpr uint16_t SKY=rgb(0x9ad4e4);  // sweat drops
void box(int x,int y,int w,int h,uint16_t c) { gfx565::rect(x*3,y*3,w*3,h*3,c); }
void oval(int x,int y,int rx,int ry,uint16_t c) {
  for(int dy=-ry;dy<=ry;++dy) {
    int dx=static_cast<int>(rx*std::sqrt(1.f-float(dy*dy)/float(ry*ry)));
    box(x-dx,y+dy,2*dx+1,1,c);
  }
}
// Original five-column, seven-row block lettering. A-Z, 0-9, !, ?, &, space.
constexpr uint8_t GLYPHS[][7]={
{14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
{30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{31,4,4,4,4,4,31},
{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
{17,27,21,21,17,17,17},{17,25,25,21,19,19,17},{14,17,17,17,17,17,14},
{30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
{17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
{17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
{14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
{14,17,17,15,1,1,14},{4,4,4,4,4,0,4},{14,17,1,2,4,0,4},
{12,18,20,8,21,18,13},{0,0,0,0,0,0,0}};
int glyph(char c) {
 if(c>='A'&&c<='Z') return c-'A';
 if(c>='0'&&c<='9') return c-'0'+26;
 if(c=='!') return 36;
 if(c=='?') return 37;
 if(c=='&') return 38;
 return 39;
}
void text(int y,const char* s,uint16_t color,int scale=1,int cx=80) {
 int width=static_cast<int>(std::strlen(s))*6*scale-scale,x=(2*cx-width)/2;
 uint16_t bg=gfx565::fb[(y*3)*480+x*3];
 if(gfx::textLogEnabled&&gfx::textLogCount<64)
  gfx::textLog[gfx::textLogCount++]={int16_t(x),int16_t(y),int16_t(width),int16_t(7*scale),gfx565::rgb888(color),gfx565::rgb888(bg),true};
 for(int n=0;s[n];++n) for(int row=0;row<7;++row) for(int col=0;col<5;++col)
  if(GLYPHS[glyph(s[n])][row]&(16>>col)) box(x+n*6*scale+col*scale,y+row*scale,scale,scale,color);
}
void label(int y,const char* s,int cx=80) {
 int w=static_cast<int>(std::strlen(s))*6+7;
 box((2*cx-w)/2,y-3,w,13,INK); text(y,s,CREAM,1,cx);
}
void button(const char* s) {
 box(40,119,80,24,INK); box(42,121,76,20,CREAM); box(42,139,76,2,GOLD); text(127,s,INK);
}
void home(float progress) {
 box(68,8,24,22,INK); box(70,10,20,18,PURPLE);
 for(int i=0;i<5;++i) box(74-i,17+i,2,1,CREAM);
 box(74,17,12,2,CREAM); box(84,17,2,7,CREAM); box(76,23,8,2,CREAM);
 box(70,27,static_cast<int>(20*std::min(1.f,progress)),2,GOLD);
}
// Classic candy corn, point up: cream tip, orange middle, gold base; 9x8 with its outline.
void candyCorn(int x,int y,bool filled) {
 constexpr uint8_t EDGE[8]={3,5,5,7,7,9,9,9},FILL[7]={1,3,3,5,5,7,7};
 for(int r=0;r<8;++r) box(x-EDGE[r]/2,y-4+r,EDGE[r],1,INK);
 for(int r=0;r<7;++r) box(x-FILL[r]/2,y-3+r,FILL[r],1,!filled?WALL:r<2?CREAM:r<5?ORANGE:GOLD);
}
// The window sits right of the door; its frame starts at x=23 so it clears the glass.
void window() {
 box(23,30,30,40,INK); box(25,32,26,36,WOOD); box(28,35,20,30,NIGHT);
 oval(43,58,4,4,CREAM); oval(45,57,3,3,NIGHT);  // the moon, in the lower right pane: the ghost has the upper left
 box(45,38,1,1,CREAM); box(31,56,1,1,GOLD); box(32,61,2,1,CREAM);
 box(37,34,2,32,WOOD); box(27,50,22,2,WOOD); box(21,67,34,4,INK);
 box(23,67,30,2,GOLD); box(20,31,5,35,PURPLE); box(51,31,5,35,PURPLE);
}
// The GPT-drawn ghost at the window, centred in its once-a-round tap box {22,30,24,24}. It bobs while it can
// be tapped (lively: in play) and holds still off play, where taps do nothing. Once used it is a two-tone
// silhouette in the night colours, far away and plainly not a button.
void windowGhost(bool ready,bool lively,uint32_t tick) {
 const int left=34-ghost::W/2,top=31+(ready&&lively?int(tick/400%2):0);
 for(int y=0;y<ghost::H;++y) for(int x=0;x<ghost::W;++x) {
  const uint8_t c=ghost::PX[y*ghost::W+x];
  if(c==ghost::CLEAR) continue;
  const uint16_t color=rgb(ghost::COLORS[c]);
  box(left+x,top+y,1,1,ready?color:color==INK?NIGHT:BLUE);
 }
}
void teacup(int x,int y) {
 box(x,y,12,9,INK); box(x+1,y+1,10,7,CREAM); box(x+3,y+3,6,2,LILAC); box(x+2,y+1,8,1,GOLD);
 box(x+12,y+1,3,1,INK); box(x+12,y+6,3,1,INK); box(x+14,y+2,1,4,INK);
}
// Chunky eight-spoke starburst where the cup hit the floor, pulsing while the crash is fresh.
void burst(int x,int y,uint32_t tick) {
 int s=8+int(tick/100%2)*3;
 for(int pass=0;pass<2;++pass) for(int k=0;k<8;++k) {
  int len=k%2?s-2:s; float a=float(k)*0.7854f+0.3927f*float(tick/100%2);
  for(int t=3;t<=len;++t) {
   int px=x+int(std::lround(t*std::cos(a))),py=y+int(std::lround(t*std::sin(a)));
   if(pass==0) box(px-1,py-1,3,3,INK); else box(px,py,1,1,k%2?GOLD:CREAM);
  }
 }
 oval(x,y,4,4,INK); oval(x,y,3,3,GOLD); oval(x,y,1,1,CREAM);
}
// Grandma's teacup on a shelf at the right wall. The ghost tips it off (distract: slide, fall, crash);
// the shards stay on the floor for the rest of the round.
void shelf(const View& v) {
 box(100,40,22,3,INK); box(101,41,20,1,WOOD); box(103,43,3,3,INK); box(116,43,3,3,INK);
 int e=v.distract?int(DISTRACT_MS-v.distract):v.ghostReady?0:int(DISTRACT_MS);
 int x=101-std::min(e,300)/50,y=31+std::max(0,std::min(e-300,400))*57/400;
 if(e<700) { teacup(x,y); return; }
 if(v.distract) burst(104,86,v.animationMs);
 box(94,92,5,3,INK); box(95,92,3,2,CREAM); box(96,93,1,1,LILAC); box(101,94,6,3,INK); box(102,94,4,2,CREAM);
 box(103,95,2,1,LILAC); box(108,91,4,4,INK); box(109,91,2,3,CREAM); box(113,93,3,1,INK); box(115,91,1,3,INK);
}
// A few rustle marks beside the furniture the hidden pumpkin is shaking.
void rustleMarks(int spot,uint32_t tick) {
 constexpr int Y[]={62,72,58};
 int cx=static_cast<int>(SPOTS[spot]),p=int(tick/120%2)*2,y=Y[spot];
 for(int s=spot==0?1:-1;s<=1;s+=2) {
  int x=cx+s*(18+p); box(x,y,1,4,CREAM); box(x+s*2,y-3,1,3,CREAM); box(x+s*2,y+5,1,3,CREAM);
 }
}
// The door at the left wall, x 4..17, y 57..100: the lane ends here (DOOR_X=24).
void door(bool open,uint32_t tick) {
 box(4,57,14,44,INK);
 if(!open) {
  box(6,59,10,40,WOOD); box(8,62,6,14,RUST); box(8,80,6,14,RUST); box(13,77,2,2,GOLD);
  return;
 }
 box(6,59,10,40,GOLD); box(8,61,6,36,CREAM); box(6,59,2,40,ORANGE);
 oval(16,103,12,3,GOLD); oval(14,103,8,2,CREAM);
 int bob=int(tick/200%2);
 for(int i=0;i<4;++i) { box(7+i+bob,78-i,2,2,INK); box(7+i+bob,77+i,2,2,INK); }
 box(9+bob,77,6,3,INK);
}
void room(bool doorOpen,uint32_t tick) {
 gfx565::clear(WALL);
 for(int x=0;x<160;x+=12) box(x,0,1,88,PURPLE);
 box(0,82,160,5,INK); box(0,84,160,2,WOOD); box(0,87,160,73,FLOOR);
 for(int y=91;y<160;y+=13) {
  box(0,y,160,1,WOOD);
  for(int x=(y%2)*19;x<160;x+=37) box(x,y,1,12,INK);
 }
 window(); door(doorOpen,tick); box(66,43,25,22,INK); box(68,45,21,18,GOLD); box(70,47,17,14,PURPLE);
 oval(78,54,5,5,ORANGE); box(78,47,2,3,SAGE);
 oval(80,108,68,18,INK); oval(80,106,65,16,PURPLE); oval(80,106,57,12,LILAC); oval(80,106,52,10,PURPLE);
 for(int x=33;x<132;x+=11) box(x,105,4,2,LILAC);
}
void chair(bool highlight,int dx) {
 auto b=[dx](int x,int y,int w,int h,uint16_t c) { box(x+dx,y,w,h,c); };
 b(21,59,30,39,INK); b(24,61,24,27,PURPLE); b(26,63,20,2,LILAC);
 b(27,69,2,2,LILAC); b(42,69,2,2,LILAC); b(34,78,2,2,LILAC);
 b(18,80,7,17,INK); b(20,82,3,12,LILAC); b(48,80,7,17,INK); b(50,82,3,12,LILAC);
 b(23,90,26,7,LILAC); b(24,97,4,8,INK); b(45,97,4,8,INK);
 if(highlight) b(25,94,22,2,GOLD);
}
void table(bool highlight,int dx) {
 auto b=[dx](int x,int y,int w,int h,uint16_t c) { box(x+dx,y,w,h,c); };
 b(61,78,38,7,INK); b(63,79,34,3,WOOD); b(65,85,4,20,INK); b(91,85,4,20,INK);
 b(67,85,2,17,WOOD); b(91,85,2,17,WOOD); b(73,76,15,2,CREAM);
 b(77,69,6,7,INK); b(78,70,4,5,GOLD); b(83,70,3,4,INK);
 if(highlight) b(70,83,20,2,GOLD);
}
void plant(bool highlight,int dx) {
 auto b=[dx](int x,int y,int w,int h,uint16_t c) { box(x+dx,y,w,h,c); };
 auto o=[dx](int x,int y,int rx,int ry,uint16_t c) { oval(x+dx,y,rx,ry,c); };
 b(121,63,4,29,GREEN);
 o(114,71,8,4,INK); o(114,70,7,3,SAGE); o(130,65,8,4,INK); o(130,64,7,3,GREEN);
 o(121,58,5,9,INK); o(121,58,3,7,SAGE); o(132,78,8,4,INK); o(131,77,7,3,SAGE);
 b(111,87,25,6,INK); b(113,89,21,3,GOLD); b(115,93,17,12,INK); b(117,93,13,10,RUST);
 b(119,94,2,7,ORANGE); if(highlight) b(115,89,17,2,CREAM);
}
void pumpkinFace(int x,int y,int squash,bool peek) {
 int ry=peek?6:10-squash,rx=peek?9:12+squash;
 oval(x,y+ry,rx,3,INK); oval(x,y,rx,ry,INK); oval(x,y-1,rx-2,ry-2,RUST); oval(x,y-1,rx-5,ry-2,ORANGE);
 box(x-3,y-ry-4,4,5,INK); box(x-2,y-ry-4,2,4,SAGE);
 box(x-6,y-4,3,4,INK); box(x+3,y-4,3,4,INK); box(x-5,y-4,1,1,CREAM); box(x+4,y-4,1,1,CREAM);
 box(x-4,y+3,8,2,INK); box(x-2,y+5,4,1,INK); box(x-1,y+3,2,1,CREAM);
 box(x-9,y,2,2,GOLD); box(x+7,y,2,2,RUST);
}
// Her arm out, flashlight first, stepping down toward the spot she is checking.
void pointArm(int x,int spot) {
 int fall=spot==2?3:spot==1?1:0;
 for(int i=0;i<5;++i) box(x-15-4*i,65+fall*i,6,5,INK);
 for(int i=0;i<5;++i) box(x-14-4*i,66+fall*i,4,3,PURPLE);
 int ex=x-34,ey=66+fall*4;
 box(ex-1,ey-1,6,5,INK); box(ex,ey,4,3,SKIN); box(ex-6,ey-2,6,7,INK); box(ex-5,ey-1,3,5,GOLD);
}
// She leans in on Catch, points when checking a spot, and looks over at the crash during the distraction.
void grandma(const View& v) {
 int lean=v.page==Page::Catch?int(v.pageMs/50):0,point=v.checking,x=141-lean,bob=(v.animationMs/300)%2;
 int look=point>=0||v.distract?1:0;
 if(v.heard) {  // "!" beside her head: she heard the rustle
  oval(x-13,40,5,6,INK); box(x-9,41,3,2,INK); oval(x-13,40,3,4,CREAM); box(x-13,37,1,4,INK); box(x-13,42,1,1,INK);
 }
 oval(x+6,44,6,6,INK); oval(x+6,43,4,4,GREY); oval(x,53,10,12,INK); oval(x,52,8,10,SKIN);
 box(x-7,43,13,5,GREY); box(x-10,50,9,6,INK); box(x+1,50,8,6,INK);
 box(x-8,51,5,3,CREAM); box(x+3,51,4,3,CREAM); box(x-1,52,3,1,INK);
 box(x-6-look,52,2,2,INK); box(x+3-look,52,2,2,INK); box(x-3,59,5,1,INK);
 box(x-10,64,22,27,INK); box(x-8,65,18,23,PURPLE); box(x-5,70,12,19,CREAM);
 for(int y=73;y<87;y+=5) for(int dx=-3;dx<6;dx+=5) box(x+dx,y,2,2,LILAC);
 box(x-8,91,6,8+bob,INK); box(x+5,91,5,8-bob,INK);
 box(x-10,98+bob,8,3,LILAC); box(x+4,98-bob,8,3,LILAC);
 if(point>=0) { pointArm(x,point); return; }
 box(x-8,65,12,5,SKIN); box(x-5,63,8,6,INK); box(x-5,64,3,4,GOLD);
}
// A pulsing "?" bubble over the furniture Grandma just named.
void ask(int spot,uint32_t tick) {
 int cx=static_cast<int>(SPOTS[spot]),cy=68,p=int(tick/250%2);
 oval(cx,cy,9+p,7+p,INK); box(cx-2,cy+6+p,4,4,INK); oval(cx,cy,7+p,5+p,CREAM); box(cx-1,cy+6+p,2,2,CREAM);
 for(int row=0;row<7;++row) for(int col=0;col<5;++col)
  if(GLYPHS[37][row]&(16>>col)) box(cx-2+col,cy-3+row,1,1,INK);
}
void candies(const View& v) {
 for(int i=0;i<CANDIES;++i) if(!(v.got>>i&1)) candyCorn(static_cast<int>(v.candy[i]),101,true);
}
// Four sparks flying out from the pumpkin right after a pickup.
void sparkle(int x,uint32_t sinceCandy) {
 if(sinceCandy>=400) return;
 int r=10+static_cast<int>(sinceCandy/40);
 const int at[4][2]={{x-r,96-r/2},{x+r,96-r/2},{x-r/2,92-r},{x+r/2,92-r}};
 for(auto& p:at) { box(p[0]-2,p[1],5,1,GOLD); box(p[0],p[1]-2,1,5,GOLD); box(p[0],p[1],1,1,CREAM); }
}
void sweat(int x,uint32_t tick) {
 int bob=int(tick/120%2)*2;
 const int at[2][2]={{x+10,88+bob},{x-12,91-bob}};
 for(auto& p:at) { box(p[0]-1,p[1]-1,4,5,INK); box(p[0],p[1],2,3,SKY); box(p[0],p[1],1,1,CREAM); }
}
void light(const View& v) {
 if(!v.searching) return;
 for(int y=65;y<=111;++y) {
  float t=float(y-65)/34.f;
  int left=static_cast<int>(138+(v.beamX-20-138)*t),right=static_cast<int>(138+(v.beamX+20-138)*t);
  box(left,y,right-left+1,1,BEAM);
 }
 oval(static_cast<int>(v.beamX),107,22,4,GOLD);
}
// The three hiding spots; the one hiding a rustling pumpkin jiggles 1 px.
void furniture(const View& v) {
 int j=v.rustling?int(v.animationMs/80%2):0;
 chair(v.nearby==0,v.hidden==0?j:0); table(v.nearby==1,v.hidden==1?j:0); plant(v.nearby==2,v.hidden==2?j:0);
 if(v.rustling&&v.hidden>=0) rustleMarks(v.hidden,v.animationMs);
}
void scene(const View& v) {
 room(v.doorOpen,v.animationMs); windowGhost(v.ghostReady,v.page==Page::Play,v.animationMs); light(v); shelf(v); furniture(v); candies(v);
 // She also turns to look at the crash, so the kid sees what the ghost did (no beam: she is resting).
 if(v.searching||v.warning||v.page==Page::Catch||v.distract) grandma(v);
 if(v.checking>=0&&v.warning) ask(v.checking,v.animationMs);
 if(v.page==Page::Won) return;
 int squash=std::abs(v.velocity)>5?int((v.animationMs/100)%3)-1:0,x=static_cast<int>(v.x);
 if(v.shiver||v.rustling) { x+=int(v.animationMs/(v.shiver?60:80)%2); if(v.shiver) sweat(x,v.animationMs); }
 pumpkinFace(x,v.hidden>=0?101:99,squash,v.hidden>=0);
 if(v.hidden>=0) box(x-10,107,21,2,INK);
 sparkle(x,v.sinceCandy);
}
void intro() {
 box(40,35,80,21,INK); text(37,"PUMPKIN",GOLD); text(47,"HIDE & SEEK",CREAM);
 label(110,"TILT TO ROLL"); button("START"); label(146,"GET CANDY");
}
void calibrate(const View& v) {
 label(37,"HOLD IT COMFY"); label(51,"AND STILL");
 oval(80,79,14,13,INK); box(70,69,20,18,CREAM); box(72,71,16,14,BLUE);
 box(78,87,4,2,GOLD); box(65,79,5,7,SKIN); box(90,79,5,7,SKIN);
 label(108,v.ready?"READY!":"KEEP STILL"); button(v.ready?"READY":"WAIT");
}
// Three candy corn slots under Home: outlines fill in as the candy is picked up.
void candyHud(uint8_t got) {
 box(60,32,40,14,INK); box(62,34,36,10,PURPLE);
 for(int i=0;i<CANDIES;++i) candyCorn(68+12*i,39,got>>i&1);
}
// The urgent cues, most urgent first. A rustle outranks the crash and PHEW!, since the kid's own tilt is
// the one thing to fix.
const char* alarm(const View& v) {
 if(v.exposure>0) return v.hidden>=0?"RUN AWAY!":"QUICK! HIDE!";
 if(v.heard&&v.hidden>=0&&v.rustling) return "SHE HEARD YOU!";
 if(v.rustling) return "SHH!";
 if(v.distract) return "WHAT WAS THAT?";
 if(v.phew) return "PHEW!";
 return nullptr;
}
const char* playLabel(const View& v) {
 constexpr const char* ASK[]={"THE CHAIR?","THE TABLE?","THE PLANT?"};
 if(const char* s=alarm(v)) return s;
 if(v.checking>=0&&v.warning) return ASK[v.checking];
 if(v.warning) return "GRANDMA!";
 if(v.searching) return "HMM?";
 if(v.heard&&v.hidden>=0) return "SHE HEARD YOU!";
 if(v.hidden>=0) return "HIDDEN!";
 if(v.doorOpen) return "TO THE DOOR!";
 return nullptr;
}
void play(const View& v) {
 candyHud(v.got);
 // While the ghost can be tapped the row sits right of middle, so even a 14-letter plate (x 46..136) clears its
 // box {22,30,24,24}; once it's used the box is gone and the row centres, off Grandma's glasses.
 if(const char* s=playLabel(v)) label(49,s,v.ghostReady?92:80);
 button(v.hidden>=0?"COME OUT":v.nearby>=0?"HIDE":"FIND COVER");
}
void icing(uint32_t ms) {
 for(int x=0;x<160;++x) {
  int dx=x-80,rim=80-static_cast<int>(std::sqrt(float(std::max(0,6400-dx*dx))));
  int wave=static_cast<int>(8+9*std::sin(float(x)*.19f)+5*std::sin(float(x)*.43f));
  int depth=std::max(0,static_cast<int>(ms)*190/1600+wave-16);
  if(depth==0) continue;
  box(x,rim,1,depth,ORANGE); box(x,rim+depth-2,1,3,GOLD);
 }
}
// GPT-drawn, converted by tools/gpt_art.py; it sits just under the YOU LOST plate.
void pie() {
 const int left=80-pie::W/2,top=59;
 for(int y=0;y<pie::H;++y) for(int x=0;x<pie::W;++x) {
  const uint8_t c=pie::PX[y*pie::W+x];
  if(c!=pie::CLEAR) box(left+x,top+y,1,1,rgb(pie::COLORS[c]));
 }
}
constexpr const char* CATCH_QUIP[QUIPS]={"THERE YOU ARE!","FOUND YOU!","PEEKABOO!","GOTCHA DEAR!"};
constexpr const char* LOST_QUIP[QUIPS]={"OH MY! PIE!","PIE TIME!","SWEET PUMPKIN!","YUM YUM!"};
constexpr const char* WON_QUIP[QUIPS][2]={{"YOU ESCAPED!","WITH THE CANDY"},{"SNEAKY PUMPKIN!","CANDY IS YOURS"},
 {"FREEDOM!","AND ALL THE CANDY"},{"YOU DID IT!","GRANDMA IS PUZZLED"}};
void endPage(const View& v) {
 int q=v.quip%QUIPS;
 if(v.page==Page::Lost) {
  gfx565::clear(ORANGE); icing(1600); pie(); box(30,39,100,19,INK); text(42,"YOU LOST",CREAM,2);
  label(108,LOST_QUIP[q]); button("TRY AGAIN");
 } else {
  label(37,WON_QUIP[q][0]); label(51,WON_QUIP[q][1]); pumpkinFace(80,86,1,false); button("PLAY AGAIN");
  box(73,88,14,4,INK); box(75,89,10,2,CREAM); box(79,89,2,2,INK);  // a wide grin over the usual mouth
  for(int x=62;x<=98;x+=18) candyCorn(x,x==80?103:100,true);
  for(int x=32;x<136;x+=22) box(x,62+(x%13),2,3,GOLD);
 }
}
void mask() {
 for(int y=0;y<160;++y) for(int x=0;x<160;++x) if(!gfx::inCircle(x,y)) box(x,y,1,1,0);
}
}  // namespace
void draw(const View& v) {
 gfx::textLogReset(); scene(v);
 switch(v.page) {
  case Page::Intro: intro(); break;
  case Page::Calibrate: calibrate(v); break;
  case Page::Play: play(v); break;
  case Page::Catch: label(38,CATCH_QUIP[v.quip%QUIPS]); break;
  case Page::Icing: icing(v.pageMs); break;
  case Page::Lost: case Page::Won: endPage(v); break;
 }
 home(v.homeHold); mask();
}
namespace {
uint8_t iconPixel(int x,int y) {
 int dx=x-12,dy=y-13;
 if(std::abs(x-12)<=1&&std::abs(y-3)<=2) return C_LEAF;
 bool eye=(x==7||x==8||x==15||x==16);
 if(std::abs(y-11)<=1&&eye) return C_NAVY;
 if(y==17&&std::abs(x-12)<=3) return C_NAVY;
 if(dx*dx*49+dy*dy*81<=3969) return C_ORANGE;
 if(dx*dx*64+dy*dy*100<=6400) return C_PLUM;
 return C_T;
}
}
const gfx::Sprite& icon() {
 // Shell icons share its indexed palette; in-game art uses our own RGB565 palette.
 static uint8_t px[24*24]; static gfx::Sprite sprite{24,24,px};
 for(int y=0;y<24;++y) for(int x=0;x<24;++x) px[y*24+x]=iconPixel(x,y);
 return sprite;
}
} }  // namespace pumpkin::paint
