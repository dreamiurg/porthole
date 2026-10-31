#include <chrono>
#include <cstdio>
#include "gfx565.h"
#include "games/biscuit/generated/scenes.h"
#include "games/biscuit/generated/discovery_art.h"
static uint16_t pixels[gfx565::W*gfx565::H];
int main() {
  gfx565::target(pixels);
  const auto started=std::chrono::steady_clock::now();
  constexpr int loops=12;
  for(int n=0;n<loops;n++) for(int s=0;s<3;s++) for(int t=0;t<3;t++) for(int a=0;a<14;a++) for(int f=0;f<4;f++)
#ifdef PACKED
    packed565::draw(biscuit::SCENES[s][t][a][f],0,0,3);
#else
    gfx565::blitRle(biscuit::SCENES[s][t][a][f],0,0,3);
#endif
  const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
  printf("%d scenes %.3f ms %.3f us/frame last=%u\n",loops*504,ms,ms*1000/(loops*504),pixels[0]);
}
