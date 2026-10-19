#include "render.h"
#include "assets.h"
#include "gfx565.h"
#include "input.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace starfighter::paint {
namespace {
constexpr uint16_t rgb(uint32_t c) { return ((c>>8)&0xf800)|((c>>5)&0x7e0)|((c>>3)&31); }
enum Color : uint8_t {
  SPACE, INK, PANEL, STEEL, SLATE, SILVER, HULL, WHITE,
  DEEP_BLUE, BLUE, AZURE, ICE, DARK_RED, RED, ORANGE, GOLD, CREAM,
  DARK_GREEN, GREEN, LIME, SKIN_DARK, SKIN_SHADE, SKIN, VIOLET
};
// One 51 KB bounded scratch surface. Expand once; sprites never repaint native PSRAM.
uint16_t scene[160*160];
constexpr uint16_t color(Color c) { return rgb(art::COLORS[c]); }
void pixel(int x,int y,uint16_t c) {
  if(unsigned(x)<160&&unsigned(y)<160) scene[y*160+x]=c;
}
void box(int x,int y,int w,int h,Color c) {
  int left=std::max(x,0),right=std::min(x+w,160);
  if(right<=left) return;
  for(int yy=std::max(y,0);yy<std::min(y+h,160);++yy)
    std::fill(scene+yy*160+left,scene+yy*160+right,color(c));
}
void line(int x,int y,int xx,int yy,Color c) {
  int dx=std::abs(xx-x),dy=-std::abs(yy-y),sx=x<xx?1:-1,sy=y<yy?1:-1,err=dx+dy;
  for(;;) {
    pixel(x,y,color(c));
    if(x==xx&&y==yy) break;
    int twice=err*2;
    if(twice>=dy) { err+=dy; x+=sx; }
    if(twice<=dx) { err+=dx; y+=sy; }
  }
}
void disc(int x,int y,int r,Color c) {
  for(int dy=-r;dy<=r;++dy) {
    int dx=int(std::sqrt(float(r*r-dy*dy)));
    box(x-dx,y+dy,dx*2+1,1,c);
  }
}
void ring(int x,int y,int r,Color c) {
  for(int yy=-r;yy<=r;++yy) for(int xx=-r;xx<=r;++xx) {
    int d=xx*xx+yy*yy;
    if(d<=r*r&&d>(r-1)*(r-1)) pixel(x+xx,y+yy,color(c));
  }
}
void sprite(const art::Sprite& s,int cx,int cy,int percent=100,bool retreat=false) {
  int w=s.w*percent/100,h=s.h*percent/100;
  for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
    int sx=x*100/percent,sy=y*100/percent;
    if(retreat) { sx=s.w-1-sx; sy=s.h-1-sy; }
    uint8_t c=s.px[sy*s.w+sx];
    if(c!=255) pixel(cx-w/2+x,cy-h/2+y,rgb(art::COLORS[c]));
  }
}
// Original cut-corner instrument alphabet. The open counters and squared curves
// distinguish it from the shell font; no game font or sprite is reused.
constexpr uint8_t LETTERS[][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{15,16,16,16,16,16,15},
 {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {15,16,16,19,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
 {7,1,1,1,17,17,14},{17,18,20,28,18,17,17},{16,16,16,16,16,16,31},
 {17,27,21,17,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,18,17,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},{17,17,17,17,21,27,17},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{30,1,1,14,16,16,31},
 {30,1,1,14,1,1,30},{17,17,17,31,1,1,1},{31,16,16,30,1,1,30},
 {15,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
 {14,17,17,15,1,1,30},{4,4,4,4,0,0,4},{0,0,4,0,0,4,0},
 {0,0,0,31,0,0,0},{0,4,4,31,4,4,0},{0,0,0,0,0,0,0},
 {1,1,2,4,8,16,16}
};
int glyph(char ch) {
  if(ch>='A'&&ch<='Z') return ch-'A';
  if(ch>='0'&&ch<='9') return ch-'0'+26;
  if(ch=='!') return 36;
  if(ch==':') return 37;
  if(ch=='-') return 38;
  if(ch=='+') return 39;
  if(ch=='/') return 41;
  return 40;
}
void ink(int x,int y,const char* s,Color c,int scale) {
  for(int n=0;s[n];++n) for(int row=0;row<7;++row) for(int col=0;col<5;++col)
    if(LETTERS[glyph(s[n])][row]&(16>>col))
      box(x+n*6*scale+col*scale,y+row*scale,scale,scale,c);
}
void text(int y,const char* s,Color c=WHITE,int scale=1,int cx=80) {
  int w=int(std::strlen(s))*6*scale-scale,x=cx-w/2;
  uint16_t bg=scene[std::clamp(y,0,159)*160+std::clamp(x,0,159)];
  if(gfx::textLogEnabled&&gfx::textLogCount<64)
    gfx::textLog[gfx::textLogCount++]={int16_t(x),int16_t(y),int16_t(w),int16_t(7*scale),
      gfx565::rgb888(color(c)),gfx565::rgb888(bg),true};
  ink(x,y,s,c,scale);
}
// A separate 3x5 instrument face keeps rim counters small and upright.
constexpr uint8_t TINY[][5]={
 {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
 {7,4,7,1,7},{7,4,7,5,7},{7,1,2,2,2},{7,5,7,5,7},{7,5,7,1,7},
 {5,5,7,5,5},{7,5,5,5,7},{4,4,4,4,7},{6,5,5,5,6},
 {5,5,5,7,5},{2,5,7,5,5},{5,5,5,5,2},{7,4,6,4,7}
};
int tinyGlyph(char c) {
  if(c>='0'&&c<='9') return c-'0';
  const char* letters="HOLDWAVE";
  const char* found=std::strchr(letters,c);
  return found?10+int(found-letters):0;
}
void tinyText(int x,int y,const char* s,Color c) {
  int width=int(std::strlen(s))*4-1;
  box(x-1,y-1,width+2,7,SPACE);
  if(gfx::textLogEnabled&&gfx::textLogCount<64)
    gfx::textLog[gfx::textLogCount++]={int16_t(x),int16_t(y),int16_t(width),5,
      gfx565::rgb888(color(c)),gfx565::rgb888(color(SPACE)),true};
  for(int n=0;s[n];++n) for(int row=0;row<5;++row) for(int col=0;col<3;++col)
    if(TINY[tinyGlyph(s[n])][row]&(4>>col)) box(x+n*4+col,y+row,1,1,c);
}
void plaque(int y,const char* s,Color c=WHITE) {
  int w=int(std::strlen(s))*6+7;
  box(80-w/2,y-3,w,13,SPACE); text(y,s,c);
}
void control(const Box& b,const char* s,bool primary) {
  UiAudit::add(b.x,b.y,b.w,b.h);
  Color rim=primary?ICE:SLATE;
  box(b.x+2,b.y,b.w-4,b.h,rim); box(b.x,b.y+2,b.w,b.h-4,rim);
  box(b.x+2,b.y+2,b.w-4,b.h-4,primary?PANEL:INK);
  box(b.x+5,b.y+b.h-4,b.w-10,1,primary?AZURE:STEEL);
  if(*s) text(b.y+8,s,primary?WHITE:HULL,1,b.x+b.w/2);
}
void holdExit(uint16_t progress) {
  UiAudit::add(HOLD.x,HOLD.y,HOLD.w,HOLD.h);
  box(76,11,6,1,SLATE); box(76,11,1,8,SLATE); box(76,18,6,1,SLATE);
  line(79,15,85,15,SILVER); line(83,13,85,15,SILVER); line(83,17,85,15,SILVER);
  tinyText(73,22,"HOLD",SILVER);
  if(progress) box(72,29,int(16*std::min(progress,uint16_t(1000))/1000),1,GOLD);
}
void expand() {
  if(!gfx565::fb) return;
  for(int y=0;y<160;++y) {
    uint16_t* row=gfx565::fb+y*3*480;
    for(int x=0;x<160;++x) std::fill(row+x*3,row+x*3+3,scene[y*160+x]);
    std::memcpy(row+480,row,480*sizeof(uint16_t));
    std::memcpy(row+960,row,480*sizeof(uint16_t));
  }
}
void stars(uint32_t ms,float bank,bool busy) {
  std::fill(scene,scene+160*160,color(SPACE));
  int count=busy?32:58;
  for(int i=0;i<count;++i) {
    int depth=i%3+1;
    int x=(i*71+17-int(bank*depth*4)+320)%160;
    int y=(i*i*19+i*23+int(ms/(95/depth)))%160;
    box(x,y,1,depth==3?2:1,depth==1?STEEL:depth==2?SLATE:HULL);
    if(i%19==0) { box(x-1,y,3,1,SLATE); box(x,y-1,1,3,SLATE); box(x,y,1,1,WHITE); }
  }
}
void moon(int x,int y) {
  disc(x,y,21,STEEL); disc(x-3,y-3,18,SLATE);
  for(int i=0;i<7;++i) {
    int xx=x-13+(i*13)%27,yy=y-13+(i*17)%25,r=i%3+2;
    disc(xx,yy,r,STEEL); box(xx-r+1,yy-r,r+1,1,SILVER);
  }
  // Equatorial station seam and one illuminated docking aperture.
  box(x-20,y+4,40,2,PANEL); box(x-17,y+5,33,1,STEEL);
  box(x+8,y-7,4,3,PANEL); box(x+8,y-7,2,1,ICE);
}
void planet(uint32_t ms,float bank) {
  int cx=6-int(bank*3)+int(ms/12000%5),cy=170;
  for(int y=96;y<160;++y) for(int x=0;x<100;++x) {
    int dx=x-cx,dy=y-cy,d=dx*dx+dy*dy;
    if(d>78*78) continue;
    Color c=d>75*75?ICE:d>70*70?AZURE:d>63*63?BLUE:DEEP_BLUE;
    if(d<70*70&&((x+y*2)/7)%9==0) c=BLUE;
    pixel(x,y,color(c));
  }
}
void space(uint32_t ms,float bank,bool busy) {
  stars(ms,bank,busy); planet(ms,bank); moon(128-int(bank*2),50+int(ms/18000%5));
  for(int i=0;i<5;++i) {
    int x=(i*47+31-int(bank*12)+320)%160,y=(i*61+int(ms/28))%190-15;
    box(x,y,3,2,STEEL); box(x+1,y,1,1,SLATE); box(x+3,y+2,2,1,PANEL);
  }
}
void engineTrails(int x,int y,float bank,uint32_t ms) {
  int pose=bank<-.15f?-1:bank>.15f?1:0;
  for(int side=-1;side<=1;side+=2) for(int engine=0;engine<2;++engine) {
    int sx=side*(engine?20:14),sy=engine?49:39,ax=30+sx,ay=sy;
    if(pose) {
      ax=30+int(std::round(sx*.89f))+int(std::round((28-sy)*pose*.1f));
      ay=sy+int(std::round(sx*pose*.22f));
    }
    int xx=x+ax/2-15,yy=y+ay/2-14;
    int radius=std::max(0,79*79-(xx-80)*(xx-80));
    int floor=79+int(std::sqrt(float(radius)));
    int length=std::max(0,std::min(floor-yy,2+int(ms/80%2)+(std::abs(bank)>.3f?1:0)));
    box(xx-1,yy,3,length,DARK_RED); box(xx,yy,1,length-1,ORANGE);
  }
}
void fighter(int x,int y,float bank,uint32_t ms) {
  engineTrails(x,y,bank,ms);
  sprite(bank<-.15f?art::BANK_LEFT:bank>.15f?art::BANK_RIGHT:art::FIGHTER,x,y,50);
}
void life(int x,int y,bool on) {
  Color c=on?HULL:STEEL;
  box(x+2,y,1,5,c); box(x,y+2,5,1,c); box(x,y+1,1,4,c); box(x+4,y+1,1,4,c);
  box(x+1,y+5,1,1,on?RED:PANEL); box(x+3,y+5,1,1,on?RED:PANEL);
}
void hud(const Run& r) {
  constexpr int lx[]={26,35,45},ly[]={33,24,18},sy[]={15,18,21,25,29};
  for(int i=0;i<3;++i) life(lx[i],ly[i],i<r.lives);
  char score[12]; std::snprintf(score,sizeof(score),"%u",unsigned(std::min<uint32_t>(r.score,99999)));
  int length=int(std::strlen(score)),first=(5-length)/2;
  for(int i=0;i<length;++i) {
    char digit[]={score[i],0}; tinyText(105+(first+i)*5,sy[first+i],digit,SILVER);
  }
  if(r.stage>=Stage::TrenchEntry) return;
  constexpr int wx[]={17,14,12,11},wy[]={47,53,59,65};
  for(int i=0;i<WAVES;++i) box(wx[i],wy[i],3,2,i<=r.wave?ICE:STEEL);
  tinyText(9,72,"WAVE",SLATE);
}
void title(const View& v) {
  space(v.ms,0,false);
  text(20,"BANK TAP FLY",ICE);
  box(15,31,130,22,SPACE);
  ink(17,38,"STARFIGHTER",DARK_RED,2); text(36,"STARFIGHTER",GOLD,2);
  sprite(art::FIGHTER,80,80);
  control(MAIN,"START",true); control(EXIT,"EXIT",false);
}
void hand(int x,bool right) {
  box(x,67,11,29,SKIN_DARK); box(x+2,67,8,26,SKIN);
  box(x+(right?-4:8),70,7,14,SKIN_SHADE); box(x+(right?-3:9),70,5,10,SKIN);
  for(int i=0;i<3;++i) box(x+1,74+i*6,8,1,SKIN_SHADE);
  box(x+2,94,9,8,STEEL); box(x+3,94,7,2,ICE);
}
void arrow(int x,bool right) {
  int s=right?1:-1;
  box(x-7,63,15,3,ICE);
  for(int i=0;i<6;++i) box(x+s*(3+i),60+i,2,2,ICE);
}
void calibration(const View& v) {
  stars(v.ms,0,false); text(28,"BANK TO STEER",WHITE);
  text(40,v.gripReady?"READY TO FLY":v.gripUpright?"HOLD STEADY":"LIFT UPRIGHT",v.gripReady?LIME:ICE);
  disc(80,77,28,STEEL); disc(80,77,25,PANEL); ring(80,77,22,SLATE);
  sprite(v.bank<-.15f?art::BANK_LEFT:v.bank>.15f?art::BANK_RIGHT:art::FIGHTER,80,77,60);
  hand(42,false); hand(108,true); arrow(27,false); arrow(132,true);
  control(MAIN,v.gripReady?"CONTINUE":"WAIT",v.gripReady);
  box(48,128,int(64*std::min(v.calibrationPermille,uint16_t(1000))/1000),2,GOLD);
  control(EXIT,"EXIT",false);
}
void briefing(const View& v) {
  space(v.ms,0,false); text(23,"YOUR MISSION",ICE);
  box(63,35,34,28,PANEL); box(65,37,30,24,INK); sprite(art::PILOT,80,49,80);
  plaque(66,"CLEAR FIGHTERS"); plaque(76,"FLY THE TRENCH");
  plaque(86,"BANK TO STEER",ICE); plaque(96,"TAP TO FIRE",ICE);
  control(MAIN,"FLY",true); control(EXIT,"EXIT",false);
}
// Deliberately discrete depth bands: the outer wall faces, raised rim, deep
// floor and lit rail each move a different distance as the child banks.
void trenchFloor(float bank) {
  int shift=int(bank*11),vanish=80+int(bank*2);
  for(int y=50;y<160;++y) {
    int d=y-50,lean=shift*d/110;
    int a=vanish-7-d*113/100+lean,b=vanish+7+d*113/100+lean;
    int l=vanish-7-d*32/100+lean,r=vanish+7+d*32/100+lean;
    box(0,y,a,1,PANEL); box(a,y,4,1,SILVER); box(a+4,y,l-a-4,1,STEEL);
    box(l,y,r-l,1,INK); box(r,y,b-r-4,1,SLATE); box(b-4,y,4,1,HULL);
    box(b,y,160-b,1,PANEL);
    pixel(l-1,y,color(PANEL)); pixel(r,y,color(STEEL));
    if(y>64) { pixel(l+3,y,color(DEEP_BLUE)); pixel(r-3,y,color(DEEP_BLUE)); }
  }
  for(int s=-1;s<=1;s+=2) {
    line(vanish+s*6,50,vanish+s*23+shift,159,STEEL);
    line(vanish+s*7,50,vanish+s*62+shift,159,PANEL);
    line(vanish+s*8,50,vanish+s*88+shift,159,PANEL);
  }
}
void ribs(uint32_t ms,float bank) {
  for(int i=0;i<9;++i) {
    int depth=(i*111+int(ms/14))%1000;
    int y=51+depth*depth*108/1000000,d=y-50,shift=int(bank*11)*d/110;
    int l=80-7-d*32/100+shift,r=80+7+d*32/100+shift;
    int ty=50+d*60/100,tx=80-7-d*68/100+shift;
    int rx=80+7+d*68/100+shift;
    for(int w=0;w<1+d/35;++w) {
      line(tx-w,ty,l-w,y,PANEL); line(rx+w,ty,r+w,y,PANEL);
    }
    line(tx+2,ty,l-1,y,SILVER); line(rx-2,ty,r+1,y,HULL);
    line(l+2,y,r-2,y,STEEL);
    if(d>28) { box(tx-1,ty,3,2,ICE); box(rx-1,ty,3,2,ICE); }
  }
}
void trench(const Run& r) {
  stars(r.ms,r.player.bank,true); moon(82,40);
  trenchFloor(r.player.bank); ribs(r.ms,r.player.bank);
}
void turretPart(int x,int y,const Box& b,Color c) {
  int left=int(std::round(b.x*.8f)),top=int(std::round(b.y*.8f));
  int right=int(std::round((b.x+b.w)*.8f)),bottom=int(std::round((b.y+b.h)*.8f));
  box(x+left,y+top,std::max(1,right-left),std::max(1,bottom-top),c);
}
void turret(int x,int y) {
  turretPart(x,y,{-8,-4,17,10},INK); turretPart(x,y,{-6,-5,13,9},STEEL);
  turretPart(x,y,{-4,-7,9,7},SLATE); turretPart(x,y,{-3,-7,5,2},HULL);
  turretPart(x,y,{-2,-3,5,3},DARK_RED); turretPart(x,y,{-1,-2,3,2},RED);
  turretPart(x,y,{-4,2,2,7},PANEL); turretPart(x,y,{3,2,2,7},PANEL);
  turretPart(x,y,{-4,7,2,2},GREEN); turretPart(x,y,{3,7,2,2},GREEN);
}
void combat(const Run& r) {
  for(const auto& o:r.obstacles) if(o.active) turret(int(o.x/3),int(o.y/3));
  for(const auto& e:r.enemies) if(e.active)
    sprite(e.heavy?art::HEAVY:art::ENEMY,int(e.x/3),int(e.y/3),80);
  for(const auto& b:r.bolts) if(b.active) {
    int x=int(b.x/3),y=int(b.y/3);
    box(x-1,y-3,3,8,b.enemy?DARK_GREEN:DARK_RED);
    box(x,y-3,1,7,b.enemy?GREEN:RED); box(x,y-3,1,3,b.enemy?LIME:CREAM);
  }
}
void effects(const Run& r) {
  constexpr const art::Sprite* frames[]={&art::BURST_0,&art::BURST_1,&art::BURST_2,&art::BURST_3,&art::BURST_4};
  for(const auto& e:r.effects) if(e.active) {
    uint32_t duration=e.large?720:360;
    unsigned frame=std::min<uint32_t>(4,(duration-std::min(e.ms,duration))*5/duration);
    sprite(*frames[frame],int(e.x/3),int(e.y/3),e.large?135:100);
  }
}
void player(const Run& r) {
  bool blink=r.player.invulnerableMs&&r.ms/100%2;
  if(!blink) fighter(int(r.player.x/3),int(r.player.y/3),r.player.bank,r.ms);
  if(r.player.invulnerableMs>INVULNERABLE_MS-650) plaque(86,"SHIP HIT",GOLD);
}
void target(const Run& r) {
  int x=int(r.targetX/3),y=64;
  box(x-20,y-16,41,33,PANEL); box(x-17,y-13,35,27,STEEL);
  box(x-14,y-11,29,23,INK); ring(x,y,10,RED); ring(x,y,7,ORANGE); disc(x,y,4,DARK_RED);
  Color c=aligned(r)?LIME:ICE;
  int aim=int(r.player.x/3);
  for(int s=-1;s<=1;s+=2) {
    box(aim+s*14-(s<0?0:4),y-13,5,2,c); box(aim+s*14,y-13,2,7,c);
    box(aim+s*14-(s<0?0:4),y+12,5,2,c); box(aim+s*14,y+7,2,7,c);
  }
  plaque(86,r.stage==Stage::Torpedo?"TORPEDO AWAY":aligned(r)?"TAP NOW!":"LINE UP",c);
  if(r.targetPasses&&r.stage!=Stage::Torpedo) plaque(98,"ANOTHER PASS",GOLD);
}
void torpedo(const Run& r) {
  target(r);
  int x=int(r.player.x/3),y=112-int(std::min<uint32_t>(r.stageMs,1400))*48/1400;
  box(x-2,y,5,14,BLUE); box(x-1,y,3,8,ICE); box(x,y,1,5,WHITE);
}
void transition(const Run& r) {
  if(r.stage==Stage::WaveClear) {
    box(12,63,136,26,SPACE); text(68,"WAVE CLEAR",GOLD,2);
    char next[20];
    if(r.wave+1>=WAVES) std::snprintf(next,sizeof(next),"THE TRENCH");
    else std::snprintf(next,sizeof(next),"SQUADRON %u",unsigned(r.wave+2));
    plaque(93,next,ICE);
  }
  if(r.stage==Stage::TrenchEntry) {
    box(12,63,136,26,SPACE); text(68,"THE TRENCH",GOLD,2); plaque(93,"DODGE AND FIRE",ICE);
  }
}
void station(uint32_t ms) {
  ring(80,75,47+int(ms/120%10),DARK_RED);
  disc(80,75,44,STEEL); disc(76,71,40,SLATE);
  box(37,76,86,3,PANEL); box(43,83,75,2,STEEL);
  for(int i=0;i<13;++i) {
    int x=49+i*23%58,y=49+i*17%51;
    box(x,y,7+i%3,2,STEEL); box(x,y+3,3,1,HULL);
  }
  disc(64,60,10,PANEL); disc(66,61,7,STEEL); box(66,60,3,3,INK);
  int frame=int(ms/120%3);
  sprite(frame==0?art::BURST_0:frame==1?art::BURST_1:art::BURST_2,82,74,290);
  sprite(art::BURST_1,106,53,130); sprite(art::BURST_2,53,87,150);
  for(int i=0;i<12;++i) {
    int dx=(i*31%81)-40,dy=(i*23%67)-33;
    int travel=int(ms/60%18);
    box(80+dx+dx*travel/40,75+dy+dy*travel/40,2+i%2,2,ORANGE);
  }
}
void explosionScene(const Run& r) {
  stars(r.ms,0,true); station(r.stageMs);
  int retreat=std::min(23,int(r.stageMs/100));
  sprite(art::FIGHTER,80,114+retreat,70+retreat,true);
  plaque(42,"GREAT SHOT!",CREAM);
}
void endScene(const Run& r) {
  bool won=r.stage==Stage::Victory;
  stars(r.ms,0,false);
  if(won) {
    station(r.ms); sprite(art::BANK_LEFT,80,85,90,true);
  } else {
    planet(r.ms,0); moon(125,63);
    sprite(art::BANK_RIGHT,80,87,80);
    box(64,103,9,2,STEEL); box(100,96,5,3,STEEL); box(53,90,3,2,RED);
  }
  box(11,37,138,22,SPACE); text(41,won?"VICTORY!":"GAME OVER",won?GOLD:WHITE,2);
  char score[24]; std::snprintf(score,sizeof(score),"SCORE %u",unsigned(r.score));
  plaque(63,score,ICE);
  control(MAIN,won?"PLAY AGAIN":"TRY AGAIN",true); control(EXIT,"EXIT",false);
}
void flight(const View& v) {
  if(!v.run) return;
  const Run& r=*v.run;
  if(!playing(r.stage)) { endScene(r); return; }
  if(r.stage==Stage::Explosion) { explosionScene(r); holdExit(v.holdPermille); return; }
  if(r.stage>=Stage::TrenchEntry) trench(r); else space(r.ms,r.player.bank,true);
  combat(r);
  if(r.stage==Stage::Target) target(r);
  if(r.stage==Stage::Torpedo) torpedo(r);
  player(r); effects(r); transition(r); hud(r); holdExit(v.holdPermille);
}
// Launcher icons must use the shell's indexed sprite contract. Map only this
// original craft's pixels to that surface; in-game colors stay independent.
const gfx::Sprite& launcherIcon() {
  static uint8_t px[32*32];
  static bool ready=false;
  static const gfx::Sprite s{32,32,px};
  if(!ready) {
    constexpr uint8_t map[]={C_NAVY,C_NAVY,C_SLATE,C_SLATE,C_BLUE,C_LTGRAY,C_LTGRAY,C_WHITE,
      C_NAVY,C_BLUE,C_BLUE,C_SKY,C_RED,C_RED,C_ORANGE,C_YELLOW,C_CREAM,
      C_DKGREEN,C_GREEN,C_MINT,C_BROWN,C_TAN,C_PEACH,C_PLUM};
    for(int y=0;y<32;++y) for(int x=0;x<32;++x) {
      uint8_t c=art::FIGHTER.px[(y*57/32)*61+x*61/32];
      px[y*32+x]=c==255?C_T:map[c];
    }
    ready=true;
  }
  return s;
}
}  // namespace
void draw(const View& v) {
  switch(v.page) {
    case Page::Title: title(v); break;
    case Page::Calibrate: calibration(v); break;
    case Page::Brief: briefing(v); break;
    case Page::Flight: flight(v); break;
  }
  expand();
}
const gfx::Sprite& icon() { return launcherIcon(); }
}  // namespace starfighter::paint
