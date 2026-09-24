#include <Arduino.h>
#include <Preferences.h>
#include <lvgl.h>
#include <esp_timer.h>
#include <esp_heap_caps.h>
#include <time.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "board.h"
#include "pet.h"
#include "assets.h"
#include "fonts.h"
#define lv_font_montserrat_16 biscuit_font_16
#define lv_font_montserrat_20 biscuit_font_20
#define lv_font_montserrat_24 biscuit_font_24
#define lv_font_montserrat_28 biscuit_font_28

namespace a = biscuitassets;
static pet::Pet puppy;
static Preferences prefs;
static bool savedOK = true, clockOK = false, testing = false;
static pet::Pet testBackup;
static uint64_t testEpochBase=0,testEpochMicros=0;
static bool testClockOK=false;
static uint8_t testBrightness=75;
static uint64_t epochBase = 0, epochMicros = 0;
static uint8_t brightness = 75;
static const char* timezoneRule = "PST8PDT,M3.2.0,M11.1.0";
static uint64_t nowSeconds() { return epochBase + (esp_timer_get_time() - epochMicros) / 1000000; }
static int32_t dayOrdinal() {
  time_t t = nowSeconds(); tm local; localtime_r(&t, &local);
  const int year=local.tm_year+1900;
  return 365*(year-1970)+(year-1)/4-1969/4-(year-1)/100+1969/100+(year-1)/400-1969/400+local.tm_yday;
}
static uint32_t checksum(const void* ptr, size_t size) {
  uint32_t hash = 2166136261u;
  const uint8_t* p = static_cast<const uint8_t*>(ptr);
  while (size--) hash = (hash ^ *p++) * 16777619u;
  return hash;
}
struct Save { uint32_t magic; pet::Pet value; uint32_t hash; };
static void savePet() {
  if (testing) return;
  Save s{}; s.magic = 0x5a4f4531; s.value = puppy;
  s.hash = checksum(&s.value, sizeof(s.value));
  savedOK = prefs.putBytes("pet1", &s, sizeof(s)) == sizeof(s);
  if (!savedOK) Serial.println("ERROR save failed");
}
static void setClock(uint64_t epoch) {
  if (epoch < 946684800ULL || epoch >= 4102444800ULL) return;
  clockOK = testing ? true : boardClockSet(epoch);
  epochBase = epoch; epochMicros = esp_timer_get_time();
  // Clock corrections must not charge the puppy for elapsed time that did not happen.
  puppy.updatedAt = epoch;
  if (puppy.createdAt > epoch) puppy.createdAt = epoch;
  const int32_t day=dayOrdinal();
  if(day!=puppy.lastVisitDay){puppy.lastVisitDay=day;puppy.dailyCompleted=puppy.dailyClaimed=0;}
  savePet();
}

enum View { Home, World, Library, Story, Choice, Ending, Discoveries, Topics, Discovery,
  Source, Today, Word, Tricks, Training, Profile, Stickers, Settings, Clock };
static View view = Home;
static int listPage = 0, storyIndex = 0, storyPage = 0;
static int factIndex = 0, factPage = -1, topicIndex = -1, factMode = 0;
static int trickIndex = 0, trainStep = 0, fetchScore = -1;
static bool trainWatching = true;
static int activity = 0;
static uint64_t activityUntil = 0;
static std::string speech = "Books, biscuits, and you. My favorite things.";
static std::vector<std::string> pages;
static lv_obj_t *root = nullptr, *scene = nullptr, *speechLabel = nullptr;
static lv_img_dsc_t sceneImage{};
static uint16_t* scenePixels = nullptr;
static lv_disp_draw_buf_t drawBuffer;
static lv_disp_drv_t displayDriver;
static uint32_t flushes = 0, touchReads = 0, physicalTouches = 0;
static bool previousPhysical = false;
static int injectedX = 0, injectedY = 0;
static uint64_t injectedUntil = 0;
static int pendingCommand = 0;
static lv_obj_t* clockFields[5] = {};
static lv_obj_t* clockError = nullptr;
static lv_color_t paper() { return lv_color_hex(0xfff7e6); }
static lv_color_t ink() { return lv_color_hex(0x42355a); }
static lv_color_t line() { return lv_color_hex(0x9c86ad); }
static lv_color_t purple() { return lv_color_hex(0xe6dfef); }
static lv_color_t peach() { return lv_color_hex(0xefdfce); }
static lv_color_t sage() { return lv_color_hex(0xe6ebd6); }
static void render();
static void command(int code);
static int pack(int op, int arg = 0) { return op * 256 + arg + 1; }
static int mod(int n, int d) { return (n % d + d) % d; }
enum Op { GoHome=1, GoWorld, Feed, Cuddle, Play, Catch, Nap, GoLibrary, OpenStory, Previous,
  Next, Choose, GoDiscoveries, GoTopics, OpenTopic, OpenFact, KeepFact, ShowSource,
  BackFact, GoToday, GoWord, GoTricks, OpenTrick, StartLesson, CueTap, GoProfile, GoStickers,
  GoSettings, Dimmer, Brighter, PrevList, NextList, BrowseFacts, GoClock, SaveClock };

static lv_obj_t* label(const std::string& text, int x, int y, int w, const lv_font_t* font = &lv_font_montserrat_24, lv_text_align_t align = LV_TEXT_ALIGN_CENTER) {
  lv_obj_t* o = lv_label_create(root);
  lv_obj_set_style_text_font(o, font, 0); lv_obj_set_style_text_color(o, ink(), 0);
  lv_obj_set_style_text_align(o, align, 0); lv_obj_set_style_text_line_space(o,4,0); lv_label_set_long_mode(o, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(o, w); lv_obj_set_pos(o, x, y); lv_label_set_text(o, text.c_str());
  return o;
}
static void clicked(lv_event_t* e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    pendingCommand = static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(e)));
}
static lv_obj_t* button(const std::string& text, int x, int y, int w, int h, int code, bool enabled = true, const lv_font_t* font = &lv_font_montserrat_20) {
  lv_obj_t* b = lv_btn_create(root);
  lv_obj_set_pos(b,x,y); lv_obj_set_size(b,w,h);
  lv_obj_set_style_radius(b,7,0); lv_obj_set_style_bg_color(b,purple(),0);
  lv_obj_set_style_border_color(b,line(),0); lv_obj_set_style_border_width(b,1,0);
  lv_obj_set_style_shadow_width(b,0,0); lv_obj_set_style_pad_all(b,5,0);
  lv_obj_set_style_bg_color(b,purple(),LV_STATE_PRESSED);
  lv_obj_set_style_bg_color(b,lv_color_hex(0xe6d2d6),LV_STATE_DISABLED);
  lv_obj_add_event_cb(b,clicked,LV_EVENT_CLICKED,reinterpret_cast<void*>(static_cast<intptr_t>(code)));
  if (!enabled) lv_obj_add_state(b,LV_STATE_DISABLED);
  lv_obj_t* l = lv_label_create(b);
  lv_label_set_text(l,text.c_str()); lv_obj_set_width(l,w-14);
  lv_obj_set_style_text_font(l,font,0); lv_obj_set_style_text_color(l,ink(),0);
  lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0); lv_obj_center(l);
  return b;
}

enum class Icon { Bowl, Heart, Moon, Book, Ball, Paw, Star };
static lv_color_t iconPixels[7][24*24];
static const char* const iconPatterns[][12] = {
  {"...##.......","...##..##...","..####.##...","............","############",".##########.","..########..","...######...","...######...","............","............","............"},
  {"............","..###..###..",".##########.","############","############",".##########.","..########..","...######...","....####....",".....##.....","............","............"},
  {"......####..","....######..","...#####....","..####......","..###.......","..###.......","..####......","...#####....","....######..","......####..","............","............"},
  {"............",".####..####.",".####..####.",".#..#..#..#.",".#..#..#..#.",".#..#..#..#.",".#..#..#..#.",".####..####.","..###..###..","...##..##...","............","............"},
  {"....####....","..##....##..",".##......##.","##..##....##","##...##...##","##....##..##",".##....##.#.","..##....##..","....####....","............","............","............"},
  {"..##....##..",".####..####.",".####..####.","..##....##..","............","##..####..##","###########.",".##########.","..########..","...######...","............","............"},
  {".....##.....",".....##.....","..########..","...######...","....####....","..########..",".###.##.###.","##...##...##",".....##.....",".....##.....","............","............"},
};
static lv_obj_t* icon(lv_obj_t* parent, Icon value, int slot, int scale, lv_color_t color) {
  const int size=12*scale;
  lv_obj_t* canvas=lv_canvas_create(parent);
  lv_canvas_set_buffer(canvas,iconPixels[slot],size,size,LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED);
  lv_canvas_fill_bg(canvas,LV_COLOR_CHROMA_KEY,LV_OPA_COVER);
  for(int y=0;y<12;y++) for(int x=0;x<12;x++) if(iconPatterns[static_cast<int>(value)][y][x]=='#')
    for(int yy=0;yy<scale;yy++) for(int xx=0;xx<scale;xx++) lv_canvas_set_px_color(canvas,x*scale+xx,y*scale+yy,color);
  return canvas;
}
static lv_obj_t* iconButton(const char* text, Icon glyph, int slot, int x, int code, lv_color_t color, bool enabled=true) {
  lv_obj_t* b=button(text,x,355,68,72,code,enabled);
  lv_obj_set_style_bg_color(b,color,LV_STATE_DEFAULT);
  lv_obj_t* l=lv_obj_get_child(b,0); lv_obj_align(l,LV_ALIGN_BOTTOM_MID,0,-5);
  lv_obj_t* image=icon(b,glyph,slot,2,ink()); lv_obj_align(image,LV_ALIGN_TOP_MID,0,7);
  return b;
}
static lv_obj_t* worldButton(const char* title, const std::string& subtitle, Icon glyph, int slot,
                             int x, int y, int code, lv_color_t color) {
  lv_obj_t* b=button("",x,y,170,96,code);
  lv_obj_set_style_bg_color(b,color,LV_STATE_DEFAULT); lv_obj_set_style_pad_all(b,0,0);
  lv_obj_t* heading=lv_obj_get_child(b,0); lv_label_set_text(heading,title);
  lv_obj_set_style_text_font(heading,&lv_font_montserrat_20,0);
  lv_obj_set_style_text_align(heading,LV_TEXT_ALIGN_LEFT,0);
  lv_label_set_long_mode(heading,LV_LABEL_LONG_WRAP); lv_obj_set_width(heading,118);
  lv_obj_align(heading,LV_ALIGN_TOP_LEFT,42,10);
  lv_obj_t* image=icon(b,glyph,slot,2,ink()); lv_obj_set_pos(image,10,13);
  lv_obj_t* detail=lv_label_create(b); lv_label_set_text(detail,subtitle.c_str());
  lv_obj_set_style_text_font(detail,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(detail,lv_color_hex(0x7b6984),0);
  lv_obj_set_style_text_align(detail,LV_TEXT_ALIGN_LEFT,0);
  lv_label_set_long_mode(detail,LV_LABEL_LONG_WRAP); lv_obj_set_pos(detail,10,58); lv_obj_set_width(detail,150);
  return b;
}
static void need(Icon glyph, int slot, int x, int value, lv_color_t color) {
  lv_obj_t* card=lv_obj_create(root); lv_obj_clear_flag(card,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(card,x,86); lv_obj_set_size(card,104,34);
  lv_obj_set_style_radius(card,5,0); lv_obj_set_style_bg_color(card,paper(),0);
  lv_obj_set_style_border_color(card,line(),0); lv_obj_set_style_border_width(card,1,0);
  lv_obj_set_style_pad_all(card,0,0);
  lv_obj_t* image=icon(card,glyph,slot,1,color); lv_obj_set_pos(image,6,10);
  lv_obj_t* track=lv_obj_create(card); lv_obj_clear_flag(track,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(track,23,14); lv_obj_set_size(track,28,7); lv_obj_set_style_pad_all(track,0,0);
  lv_obj_set_style_radius(track,1,0); lv_obj_set_style_border_width(track,0,0);
  lv_obj_set_style_bg_color(track,peach(),0);
  lv_obj_t* fill=lv_obj_create(track); lv_obj_clear_flag(fill,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(fill,0,0); lv_obj_set_size(fill,std::max(1,value*28/100),7);
  lv_obj_set_style_radius(fill,0,0); lv_obj_set_style_border_width(fill,0,0);
  lv_obj_set_style_pad_all(fill,0,0); lv_obj_set_style_bg_color(fill,color,0);
  lv_obj_t* number=lv_label_create(card); lv_label_set_text(number,std::to_string(value).c_str());
  lv_obj_set_style_text_font(number,&lv_font_montserrat_16,0); lv_obj_set_style_text_color(number,ink(),0);
  lv_obj_set_pos(number,58,6); lv_obj_set_width(number,40); lv_obj_set_style_text_align(number,LV_TEXT_ALIGN_CENTER,0);
}
static void speechBubble() {
  lv_obj_t* bubble=lv_obj_create(root); lv_obj_clear_flag(bubble,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(bubble,72,307); lv_obj_set_size(bubble,336,40);
  lv_obj_set_style_radius(bubble,7,0); lv_obj_set_style_bg_color(bubble,paper(),0);
  lv_obj_set_style_border_color(bubble,lv_color_hex(0xbdaa84),0); lv_obj_set_style_border_width(bubble,1,0);
  lv_obj_set_style_pad_all(bubble,0,0);
  speechLabel=lv_label_create(bubble); lv_label_set_text(speechLabel,speech.c_str());
  lv_obj_set_width(speechLabel,322); lv_obj_set_style_text_font(speechLabel,&lv_font_montserrat_16,0);
  lv_obj_set_style_text_color(speechLabel,ink(),0); lv_obj_set_style_text_align(speechLabel,LV_TEXT_ALIGN_CENTER,0);
  lv_obj_center(speechLabel);
}
static void top(const char* title, int back = 0) {
  button("Back",115,34,86,56,back ? back : pack(GoHome));
  label(std::to_string(pet::stars(puppy))+" stars",246,50,120,&lv_font_montserrat_20);
  label(title,70,100,340,&lv_font_montserrat_24);
}
static void nav(int current, int count, int prev = 0, int next = 0) {
  label(std::to_string(current+1)+" / "+std::to_string(std::max(1,count)),180,350,120,&lv_font_montserrat_16);
  button("Previous",104,374,128,56,prev ? prev : pack(Previous),current>0);
  button("Next",248,374,128,56,next ? next : pack(Next),current+1<count);
}
static std::vector<std::string> paginate(const char* text) {
  std::vector<std::string> result;
  std::string page, word;
  const char* cursor = text;
  while (*cursor) {
    while (*cursor && isspace(static_cast<unsigned char>(*cursor))) ++cursor;
    const char* begin = cursor;
    while (*cursor && !isspace(static_cast<unsigned char>(*cursor))) ++cursor;
    word.assign(begin,cursor-begin);
    if (word.empty()) break;
    std::string trial = page.empty() ? word : page+" "+word;
    lv_point_t size;
    lv_txt_get_size(&size,trial.c_str(),&lv_font_montserrat_24,0,4,352,LV_TEXT_FLAG_NONE);
    if (size.y > 176 && !page.empty()) { result.push_back(page); page = word; }
    else page = trial;
  }
  if (!page.empty()) result.push_back(page);
  if (result.empty()) result.emplace_back("");
  return result;
}
static lv_img_dsc_t pictures[4];
static int pictureCount=0;
static void picture(int index,int x,int y,int zoom) {
  if(pictureCount>=4)return;
  auto& descriptor=pictures[pictureCount++];
  const auto& src = a::discoveryImages[index];
  lv_img_cache_invalidate_src(&descriptor);
  descriptor = {};
  descriptor.header.cf = LV_IMG_CF_TRUE_COLOR;
  descriptor.header.w = src.width; descriptor.header.h = src.height;
  descriptor.data_size = src.width*src.height*2;
  descriptor.data = reinterpret_cast<const uint8_t*>(src.pixels);
  lv_obj_t* image = lv_img_create(root); lv_img_set_src(image,&descriptor);
  lv_img_set_pivot(image,0,0); lv_img_set_zoom(image,zoom*256);
  lv_img_set_antialias(image,false); lv_obj_set_pos(image,x,y);
}
static int currentStage() { return static_cast<int>(pet::stage(puppy)); }
static void animate() {
  if (!scene) return;
  time_t t=nowSeconds(); tm local; localtime_r(&t,&local);
  const int tod = local.tm_hour>=20 || local.tm_hour<6 ? 2 : local.tm_hour>=17 ? 1 : 0;
  int animation = puppy.sleeping ? 5 : activity;
  if (!a::decodeScene(a::scenes[currentStage()][tod][animation][(millis()/250)%4],scenePixels,160*160)) return;
  lv_obj_invalidate(scene);
}
static void say(const char* text, int nextActivity=0) {
  speech=text; activity=nextActivity; activityUntil=nowSeconds()+4;
}
static std::vector<int> factList() {
  std::vector<int> result;
  if (factMode==0) {
    int day=puppy.lastVisitDay;
    for(int family=0;family<3;family++) {
      int topic=family*4+mod(day,4), number=mod(day/4,8);
      for(int i=0;i<96;i++) if(std::string(a::discoveries[i].topic)==a::topics[topic].id && number--==0) { result.push_back(i); break; }
    }
  } else for(int i=0;i<96;i++) {
    if(factMode==2 && !pet::discovered(puppy,i)) continue;
    if(topicIndex>=0 && std::string(a::discoveries[i].topic)!=a::topics[topicIndex].id) continue;
    result.push_back(i);
  }
  return result;
}
static int adventureIndex() { return mod(puppy.lastVisitDay,7); }
static const char* actionName(int mask) {
  switch(mask) { case 1:return "A little snack"; case 2:return "Play fetch"; case 4:return "A cuddle";
    case 8:return "Read together"; case 16:return "Try a trick"; default:return "A cozy nap"; }
}
static int actionCommand(int mask) {
  switch(mask) { case 1:return pack(Feed); case 2:return pack(Play); case 4:return pack(Cuddle);
    case 8:return pack(GoLibrary); case 16:return pack(GoTricks); default:return pack(Nap); }
}
static const char* stageName() {
  return currentStage()==2 ? "Story dog" : currentStage()==1 ? "Young pup" : "Puppy";
}
static int masteredTricks() {
  int count=0; for(uint8_t progress:puppy.tricks) if(progress>=3) ++count; return count;
}
static void render() {
  scene=nullptr; speechLabel=nullptr; pictureCount=0;
  lv_obj_clean(root); lv_obj_set_style_bg_color(root,paper(),0);
  if(view==Home) {
    sceneImage={}; sceneImage.header.cf=LV_IMG_CF_TRUE_COLOR;
    sceneImage.header.w=160; sceneImage.header.h=160;
    sceneImage.data_size=160*160*2; sceneImage.data=reinterpret_cast<uint8_t*>(scenePixels);
    scene=lv_img_create(root); lv_img_set_src(scene,&sceneImage); lv_img_set_pivot(scene,0,0);
    lv_img_set_zoom(scene,768); lv_img_set_antialias(scene,false); lv_obj_set_pos(scene,0,0); animate();
    auto name=label("BISCUIT",80,58,130,&lv_font_montserrat_20,LV_TEXT_ALIGN_LEFT);
    const char* homeStage=currentStage()==2?"story dog":currentStage()==1?"young pup":"puppy";
    auto mood=label(puppy.sleeping ? "dreaming of stories" : "Day "+std::to_string(puppy.daysTogether)+" · "+homeStage,220,60,180,&lv_font_montserrat_16,LV_TEXT_ALIGN_RIGHT);
    if(puppy.sleeping) { lv_obj_set_style_text_color(name,paper(),0); lv_obj_set_style_text_color(mood,paper(),0); }
    need(Icon::Bowl,0,72,int(std::round(puppy.fullness)),lv_color_hex(0xc5965a));
    need(Icon::Heart,1,188,int(std::round(puppy.happiness)),lv_color_hex(0xc58a94));
    need(Icon::Moon,2,304,int(std::round(puppy.energy)),lv_color_hex(0x9ca66b));
    if(fetchScore>=0) {
      label("Catch the ball!  "+std::to_string(fetchScore)+" / 5",65,296,350,&lv_font_montserrat_20);
      static const int positions[5][2]={{102,174},{302,200},{200,272},{98,280},{294,282}};
      button("Ball",positions[fetchScore%5][0],positions[fetchScore%5][1],80,64,pack(Catch));
      button("All done",150,382,180,56,pack(GoHome));
    } else {
      // The puppy itself is a generous touch target, independent of its visible pixels.
      auto hit=button("",162,169,160,116,pack(Cuddle),!puppy.sleeping);
      lv_obj_set_style_bg_opa(hit,LV_OPA_TRANSP,LV_PART_MAIN|LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(hit,LV_OPA_TRANSP,LV_PART_MAIN|LV_STATE_DISABLED);
      lv_obj_set_style_border_width(hit,0,0);
      speechBubble();
      iconButton("Feed",Icon::Bowl,3,96,pack(Feed),peach(),!puppy.sleeping);
      iconButton("Read",Icon::Book,4,169,pack(GoLibrary),purple(),!puppy.sleeping);
      iconButton("Play",Icon::Ball,5,242,pack(Play),sage(),!puppy.sleeping);
      iconButton(puppy.sleeping ? "Wake" : "More",Icon::Paw,6,315,pack(puppy.sleeping?Nap:GoWorld),purple());
    }
    return;
  }
  switch(view) {
    case World:
      { auto back=button("<",86,52,56,56,pack(GoHome),true,&lv_font_montserrat_28); lv_obj_set_style_radius(back,28,0); }
      label("BISCUIT & YOU",150,68,180,&lv_font_montserrat_20,LV_TEXT_ALIGN_LEFT);
      label(std::to_string(pet::stars(puppy))+" stars",330,70,82,&lv_font_montserrat_16,LV_TEXT_ALIGN_RIGHT);
      label("Day "+std::to_string(puppy.daysTogether)+" · "+stageName()+" · "+std::to_string(puppy.friendship)+" friendship",
            64,108,352,&lv_font_montserrat_16);
      worldButton("Today's adventure",puppy.dailyClaimed?"Sticker earned!":"Something to discover",Icon::Star,0,66,145,pack(GoToday),purple());
      worldButton("Learn tricks",std::to_string(masteredTricks())+" of 6 mastered",Icon::Paw,1,244,145,pack(GoTricks),sage());
      worldButton("Our scrapbook","Growing up, page by page",Icon::Book,2,66,249,pack(GoProfile),peach());
      worldButton(puppy.sleeping?"Wake up":"Cozy nap","A lovely place to pause",Icon::Moon,3,244,249,pack(Nap),purple());
      button("Settings · brightness & clock",110,367,260,56,pack(GoSettings),true,&lv_font_montserrat_16); break;
    case Library: {
      top("Our bookshelf");
      button("Discoveries",76,145,156,56,pack(GoDiscoveries)); button("Notebook",248,145,156,56,pack(BrowseFacts,2));
      for(int j=0;j<2;j++) { int i=listPage*2+j; if(i>=7)break;
        bool unlocked=puppy.daysTogether>=a::stories[i].unlockDay;
        std::string text=unlocked ? a::stories[i].title : "Day "+std::to_string(a::stories[i].unlockDay)+": "+a::stories[i].title;
        if(puppy.stories&(1<<i)) text+=" *";
        button(text,60,213+j*66,360,60,pack(OpenStory,i),unlocked);
      }
      nav(listPage,4,pack(PrevList),pack(NextList)); break;
    }
    case Story: case Ending:
      top(view==Ending ? "The story continues" : "Story time",pack(GoLibrary));
      label(pages[storyPage],64,146,352,&lv_font_montserrat_24,LV_TEXT_ALIGN_LEFT);
      label(std::to_string(storyPage+1)+" / "+std::to_string(pages.size()),180,337,120,&lv_font_montserrat_16);
      button("Previous",104,374,128,56,pack(Previous),storyPage>0 || view==Ending);
      button(storyPage+1<int(pages.size())?"Next":view==Story?"Choose":"The end",248,374,128,56,pack(Next)); break;
    case Choice:
      top("What shall we do?",pack(Previous)); label(a::stories[storyIndex].prompt,64,148,352);
      for(int i=0;i<2;i++)button(a::stories[storyIndex].choices[i].label,78,256+i*80,324,68,pack(Choose,i)); break;
    case Discoveries: {
      top(factMode==2 ? "Our little notebook" : topicIndex>=0 ? a::topics[topicIndex].name : "Little discoveries",pack(GoLibrary));
      button("Topics",76,145,156,56,pack(GoTopics)); button("Today's three",248,145,156,56,pack(GoDiscoveries));
      auto facts=factList(); int count=std::max(1,int((facts.size()+1)/2)); listPage=std::min(listPage,count-1);
      if(facts.empty())label("A place for all the things we find together.",70,226,340);
      for(int j=0;j<2;j++){int n=listPage*2+j;if(n>=int(facts.size()))break;int i=facts[n];
        lv_point_t titleSize;lv_txt_get_size(&titleSize,a::discoveries[i].title,&lv_font_montserrat_20,0,0,230,LV_TEXT_FLAG_NONE);
        auto b=button(a::discoveries[i].title,66,210+j*70,348,64,pack(OpenFact,i),true,titleSize.y>52?&lv_font_montserrat_16:&lv_font_montserrat_20);
        auto text=lv_obj_get_child(b,0);lv_obj_set_width(text,230);lv_obj_align(text,LV_ALIGN_RIGHT_MID,0,0);
        picture(i,73,218+j*70,1);
      }
      nav(listPage,count,pack(PrevList),pack(NextList));break;
    }
    case Topics:
      top("So much to explore",pack(GoDiscoveries));
      for(int j=0;j<3;j++){int i=listPage*3+j;if(i<12){auto b=button(a::topics[i].name,82,153+j*61,316,56,pack(OpenTopic,i));
        auto text=lv_obj_get_child(b,0);lv_obj_set_width(text,200);lv_obj_align(text,LV_ALIGN_RIGHT_MID,0,0);
        for(int n=0;n<96;n++)if(std::string(a::discoveries[n].topic)==a::topics[i].id){picture(n,88,157+j*61,1);break;}}}
      nav(listPage,4,pack(PrevList),pack(NextList));break;
    case Discovery:
      top(a::topics[std::find_if(a::topics,a::topics+12,[](const a::Topic& t){return std::string(t.id)==a::discoveries[factIndex].topic;})-a::topics].name,pack(BrowseFacts,factMode));
      if(factPage<0){
        lv_point_t titleSize;lv_txt_get_size(&titleSize,a::discoveries[factIndex].title,&lv_font_montserrat_24,0,4,348,LV_TEXT_FLAG_NONE);
        label(a::discoveries[factIndex].title,66,146,348,titleSize.y>76?&lv_font_montserrat_20:&lv_font_montserrat_24);
        picture(factIndex,96,228,3);button("Let's find out",110,384,260,56,pack(Next));
      }else if(factPage<int(pages.size())){
        label(pages[factPage],64,146,352,&lv_font_montserrat_24,LV_TEXT_ALIGN_LEFT);
        label(std::to_string(factPage+1)+" / "+std::to_string(pages.size()),180,337,120,&lv_font_montserrat_16);
        button("Previous",104,374,128,56,pack(Previous));button("Next",248,374,128,56,pack(Next));
      }else{
        label("I wonder...",76,152,328,&lv_font_montserrat_28);
        label(a::discoveries[factIndex].wonder,64,194,352);
        button("Source",104,374,128,56,pack(ShowSource));button("Keep",248,374,128,56,pack(KeepFact));
      }break;
    case Source:
      top("Where we found it",pack(BackFact));
      label(a::discoveries[factIndex].sourceName,64,156,352);
      label(a::discoveries[factIndex].sourceUrl,74,220,332,&lv_font_montserrat_16,LV_TEXT_ALIGN_LEFT);
      button("Back to our book",110,374,260,56,pack(BackFact));break;
    case Today: {
      const auto& adv=a::adventures[adventureIndex()];top(adv.title,pack(GoWorld));
      label(adv.description,65,147,350,&lv_font_montserrat_16);
      int row=0;for(int bit=1;bit<=32;bit<<=1)if(adv.actions&bit){
        button(std::string(puppy.dailyCompleted&bit?"* ":"")+actionName(bit),84,228+row*57,312,56,actionCommand(bit));++row;
      }
      button("A lovely word",142,404,196,56,pack(GoWord));break;
    }
    case Word: {
      const auto& adv=a::adventures[adventureIndex()];top(adv.word,pack(GoToday));
      label(adv.meaning,64,156,352,&lv_font_montserrat_24,LV_TEXT_ALIGN_LEFT);
      button("Back to today",110,374,260,56,pack(GoToday));break;
    }
    case Tricks:
      top("Little paws, big ideas",pack(GoWorld));
      for(int j=0;j<3;j++){int i=listPage*3+j;if(i>=6)break;
        bool unlocked=puppy.daysTogether>=a::tricks[i].unlockDay;
        std::string text=a::tricks[i].name;
        text+=unlocked ? "   "+std::to_string(puppy.tricks[i])+"/3" : "   Day "+std::to_string(a::tricks[i].unlockDay);
        button(text,82,153+j*61,316,56,pack(OpenTrick,i),unlocked);
      }nav(listPage,2,pack(PrevList),pack(NextList));break;
    case Training: {
      top(a::tricks[trickIndex].name,pack(GoTricks));
      const auto& trick=a::tricks[trickIndex];int progress=std::min(2,int(puppy.tricks[trickIndex]));
      const char* cues[]={"Up","Down","Left","Right","Paw"};
      if(trainWatching){
        label("Have a look, then try with me.",64,152,352);
        std::string pattern;for(int i=0;i<trick.lessonLengths[progress];i++){if(i)pattern+=" - ";pattern+=cues[static_cast<int>(trick.lessons[progress][i])];}
        label(pattern,66,236,348,&lv_font_montserrat_28);
        button("My turn",110,374,260,56,pack(StartLesson));
      }else{
        label("Your turn!  "+std::to_string(trainStep)+" / "+std::to_string(trick.lessonLengths[progress]),75,148,330);
        button("Up",196,199,88,56,pack(CueTap,0));
        button("Left",90,263,88,56,pack(CueTap,2));button("Paw",196,263,88,56,pack(CueTap,4));button("Right",302,263,88,56,pack(CueTap,3));
        button("Down",196,327,88,56,pack(CueTap,1));
        button("Peek again",128,393,224,56,pack(OpenTrick,trickIndex));
      }break;
    }
    case Profile:
      top("Biscuit & You",pack(GoWorld));
      label(std::string(stageName())+" - "+(puppy.stories && (puppy.stories&(puppy.stories-1))?"Bookworm":"Cuddlebug"),75,152,330);
      label("Day "+std::to_string(puppy.daysTogether)+" together\n"+std::to_string(puppy.friendship)+" friendship\n"+std::to_string(pet::stars(puppy))+" story stars",70,211,340);
      button("Our stickers",110,328,260,56,pack(GoStickers));
      label(savedOK?"Progress saved on Biscuit":"Saving failed. Keep power on.",90,408,300,&lv_font_montserrat_16);break;
    case Stickers:
      top("Our sticker album",pack(GoProfile));
      for(int j=0;j<3;j++){int i=listPage*3+j;label(puppy.stickers&(1<<i)?a::stickers[i]:"A little surprise awaits",80,164+j*56,320);}
      nav(listPage,4,pack(PrevList),pack(NextList));break;
    case Settings: {
      top("Make it cozy",pack(GoWorld));
      label("Brightness "+std::to_string(brightness)+"%",90,151,300);
      button("Dimmer",90,188,140,56,pack(Dimmer));button("Brighter",250,188,140,56,pack(Brighter));
      time_t t=nowSeconds();tm local;localtime_r(&t,&local);char date[50];strftime(date,sizeof(date),"%b %d, %Y   %H:%M",&local);
      label(date,65,261,350,&lv_font_montserrat_20);
      button("Set date & time",110,304,260,56,pack(GoClock));
      label(clockOK?"Clock remembers while powered.\nA backup battery keeps it unplugged.":"Clock needs setting after power loss.",75,381,330,&lv_font_montserrat_16);break;
    }
    case Clock: {
      top("Our little clock",pack(GoSettings));
      time_t t=nowSeconds();tm local;localtime_r(&t,&local);
      const char* names[]={"Year","Month","Day","Hour","Min"};
      const int x[]={46,149,225,293,367},width[]={96,70,62,68,68};
      const int start[]={2000,1,1,0,0},end[]={2099,12,31,23,59};
      const int selected[]={local.tm_year+1900,local.tm_mon+1,local.tm_mday,local.tm_hour,local.tm_min};
      for(int i=0;i<5;i++){
        label(names[i],x[i],150,width[i],&lv_font_montserrat_16);
        auto r=lv_roller_create(root);clockFields[i]=r;
        std::string options;for(int n=start[i];n<=end[i];n++){if(n>start[i])options+='\n';options+=std::to_string(n);}
        lv_roller_set_options(r,options.c_str(),LV_ROLLER_MODE_NORMAL);
        lv_obj_set_style_text_font(r,&lv_font_montserrat_20,LV_PART_MAIN);
        lv_obj_set_style_text_font(r,&lv_font_montserrat_20,LV_PART_SELECTED);
        lv_obj_set_style_bg_color(r,purple(),LV_PART_SELECTED);
        lv_obj_set_style_text_color(r,ink(),LV_PART_MAIN);lv_obj_set_style_text_color(r,ink(),LV_PART_SELECTED);
        lv_roller_set_visible_row_count(r,3);lv_obj_set_width(r,width[i]);lv_obj_set_pos(r,x[i],180);
        lv_roller_set_selected(r,std::clamp(selected[i]-start[i],0,end[i]-start[i]),LV_ANIM_OFF);
      }
      clockError=label("Swipe each number up or down.",65,323,350,&lv_font_montserrat_16);
      button("Save time",110,374,260,56,pack(SaveClock));break;
    }
    default:break;
  }
}
static void command(int code) {
  int op=code/256,arg=(code%256)-1;
  uint64_t now=nowSeconds();int32_t day=dayOrdinal();pet::tick(puppy,now,day);
  switch(op){
    case GoHome: view=Home;fetchScore=-1;say("Books, biscuits, and you. My favorite things.");break;
    case GoWorld:view=World;break;
    case Feed:if(!puppy.sleeping){pet::act(puppy,pet::Action::Feed,now,day);say("Happy tummy, happy tail. Thank you, friend!",1);view=Home;savePet();}break;
    case Cuddle:if(!puppy.sleeping){pet::act(puppy,pet::Action::Petting,now,day);say("Your hand is my favorite place to put my head.",2);view=Home;savePet();}break;
    case Play:if(!puppy.sleeping){view=Home;activity=4;fetchScore=0;}break;
    case Catch:if(fetchScore>=0){if(++fetchScore>=5){pet::act(puppy,pet::Action::Play,now,day);fetchScore=-1;say("Five catches! My tail would like to keep playing.",4);savePet();}}break;
    case Nap:pet::toggleSleep(puppy,now,day);view=Home;fetchScore=-1;say(puppy.sleeping?"One little yawn... Night, friend.":"I dreamed we could fly. You brought snacks.");savePet();break;
    case GoLibrary:if(!puppy.sleeping){view=Library;listPage=0;}break;
    case OpenStory:if(arg>=0&&arg<7&&puppy.daysTogether>=a::stories[arg].unlockDay&&!puppy.sleeping){storyIndex=arg;storyPage=0;pages=paginate(a::stories[arg].opening);view=Story;}break;
    case Previous:
      if(view==Choice){view=Story;pages=paginate(a::stories[storyIndex].opening);storyPage=pages.size()-1;}
      else if(view==Ending&&storyPage==0)view=Choice;
      else if(view==Story||view==Ending)storyPage=std::max(0,storyPage-1);
      else if(view==Discovery)factPage=std::max(-1,factPage-1);
      break;
    case Next:
      if(view==Story||view==Ending){if(storyPage+1<int(pages.size()))++storyPage;else if(view==Story)view=Choice;else{pet::finishStory(puppy,storyIndex,now,day);savePet();view=Home;say("A story shared. Shall we try the other ending sometime?",3);}}
      else if(view==Discovery)factPage=std::min(int(pages.size()),factPage+1);
      break;
    case Choose:if(arg>=0&&arg<2){pages=paginate(a::stories[storyIndex].choices[arg].ending);storyPage=0;view=Ending;}break;
    case GoDiscoveries:factMode=0;topicIndex=-1;listPage=0;view=Discoveries;break;
    case BrowseFacts:factMode=arg;listPage=0;view=Discoveries;if(arg==2)topicIndex=-1;break;
    case GoTopics:topicIndex=-1;listPage=0;view=Topics;break;
    case OpenTopic:topicIndex=arg;factMode=1;listPage=0;view=Discoveries;break;
    case OpenFact:if(arg>=0&&arg<96&&!puppy.sleeping){factIndex=arg;factPage=-1;pages=paginate(a::discoveries[arg].text);view=Discovery;}break;
    case KeepFact:pet::discover(puppy,factIndex,now,day);savePet();factMode=2;topicIndex=-1;listPage=0;view=Discoveries;break;
    case ShowSource:view=Source;break;
    case BackFact:view=Discovery;break;
    case GoToday:view=Today;break;
    case GoWord:view=Word;break;
    case GoTricks:view=Tricks;listPage=0;break;
    case OpenTrick:if(arg>=0&&arg<6&&puppy.daysTogether>=a::tricks[arg].unlockDay){trickIndex=arg;trainStep=0;trainWatching=true;if(puppy.tricks[arg]>=3){pet::practice(puppy,arg,now,day);savePet();view=Home;say("Hey, look! I've been practicing.",6+arg);}else view=Training;}break;
    case StartLesson:trainWatching=false;trainStep=0;break;
    case CueTap:{const auto& trick=a::tricks[trickIndex];int progress=std::min(2,int(puppy.tricks[trickIndex]));
      if(arg!=static_cast<int>(trick.lessons[progress][trainStep])){trainWatching=true;trainStep=0;}
      else if(++trainStep>=trick.lessonLengths[progress]){pet::practice(puppy,trickIndex,now,day);savePet();view=Home;say(puppy.tricks[trickIndex]>=3?"We did it! Watch my little paws.":"We've got that bit! A little wag for both of us.",6+trickIndex);}break;}
    case GoProfile:view=Profile;break;
    case GoStickers:view=Stickers;listPage=0;break;
    case GoSettings:view=Settings;break;
    case GoClock:view=Clock;break;
    case SaveClock:{
      tm local{};local.tm_year=100+lv_roller_get_selected(clockFields[0]);local.tm_mon=lv_roller_get_selected(clockFields[1]);
      local.tm_mday=1+lv_roller_get_selected(clockFields[2]);local.tm_hour=lv_roller_get_selected(clockFields[3]);local.tm_min=lv_roller_get_selected(clockFields[4]);local.tm_isdst=-1;
      const int month=local.tm_mon,date=local.tm_mday;time_t t=mktime(&local);
      if(t<946684800||local.tm_mon!=month||local.tm_mday!=date){lv_label_set_text(clockError,"That month has fewer days.");return;}
      setClock(t);view=Settings;break;
    }
    case Dimmer:brightness=std::max(10,int(brightness)-15);boardBrightness(brightness);if(!testing)prefs.putUChar("brightness",brightness);break;
    case Brighter:brightness=std::min(100,int(brightness)+15);boardBrightness(brightness);if(!testing)prefs.putUChar("brightness",brightness);break;
    case PrevList:listPage=std::max(0,listPage-1);break;
    case NextList:++listPage;break;
    default:break;
  }
  render();
}
static void flush(lv_disp_drv_t* drv,const lv_area_t* area,lv_color_t* colors){
  boardFlush(area->x1,area->y1,area->x2,area->y2,reinterpret_cast<uint16_t*>(colors));++flushes;lv_disp_flush_ready(drv);
}
static void touch(lv_indev_drv_t*,lv_indev_data_t* data){
  ++touchReads; uint16_t x,y; bool down=boardTouch(x,y);
  if(down&&!previousPhysical)++physicalTouches;previousPhysical=down;
  if(injectedUntil>uint64_t(esp_timer_get_time())){down=true;x=injectedX;y=injectedY;}
  data->state=down?LV_INDEV_STATE_PR:LV_INDEV_STATE_REL;
  if(down){data->point.x=x;data->point.y=y;}
}
static void status(){
  int count=0;for(int i=0;i<96;i++)count+=pet::discovered(puppy,i);
  Serial.printf("STATUS {\"view\":%d,\"stars\":%u,\"discoveries\":%d,\"days\":%lu,\"friendship\":%lu,\"sleeping\":%u,\"saved\":%s,\"clock\":%s,\"epoch\":%llu,\"freeHeap\":%u,\"freePSRAM\":%u,\"flushes\":%lu,\"touchReads\":%lu,\"physicalTouches\":%lu,\"testing\":%s,\"checksum\":%lu,\"tricks\":[%u,%u,%u,%u,%u,%u],\"stories\":%u,\"daily\":%u,\"claimed\":%u,\"stickers\":%u,\"brightness\":%u}\n",view,pet::stars(puppy),count,(unsigned long)puppy.daysTogether,(unsigned long)puppy.friendship,puppy.sleeping,savedOK?"true":"false",clockOK?"true":"false",nowSeconds(),ESP.getFreeHeap(),ESP.getFreePsram(),(unsigned long)flushes,(unsigned long)touchReads,(unsigned long)physicalTouches,testing?"true":"false",(unsigned long)checksum(&puppy,sizeof(puppy)),puppy.tricks[0],puppy.tricks[1],puppy.tricks[2],puppy.tricks[3],puppy.tricks[4],puppy.tricks[5],puppy.stories,puppy.dailyCompleted,puppy.dailyClaimed,puppy.stickers,brightness);
}
static void tree(lv_obj_t* object){
  uint32_t n=lv_obj_get_child_cnt(object);
  if(lv_obj_check_type(object,&lv_btn_class)||lv_obj_check_type(object,&lv_label_class)){
    lv_area_t r;lv_obj_get_coords(object,&r);
    const char* text=lv_obj_check_type(object,&lv_label_class)?lv_label_get_text(object):"";
    Serial.printf("UI %s %d %d %d %d %s %s\n",lv_obj_check_type(object,&lv_btn_class)?"button":"text",r.x1,r.y1,r.x2,r.y2,lv_obj_has_state(object,LV_STATE_DISABLED)?"disabled":"enabled",text);
  }
  for(uint32_t i=0;i<n;i++)tree(lv_obj_get_child(object,i));
}
static void screenshot(){
  const uint16_t* frame=boardFrameBuffer();if(!frame){Serial.println("ERROR no frame");return;}
  // RGB565 RLE readback of the exact framebuffer handed to the LCD peripheral.
  uint32_t runs=0;for(int i=0;i<480*480;){uint16_t c=frame[i];int n=1;while(i+n<480*480&&frame[i+n]==c&&n<65535)++n;++runs;i+=n;}
  Serial.printf("FRAME %lu\n",(unsigned long)runs);
  for(int i=0;i<480*480;){uint16_t c=frame[i];uint16_t n=1;while(i+n<480*480&&frame[i+n]==c&&n<65535)++n;uint16_t pair[]={n,c};Serial.write(reinterpret_cast<uint8_t*>(pair),4);i+=n;}
  Serial.println("\nEND_FRAME");
}
static void serialCommands(){
  static char input[128];static size_t length=0;
  while(Serial.available()){
    char c=Serial.read();if(c=='\r')continue;
    if(c!='\n'){if(length<sizeof(input)-1)input[length++]=c;continue;}
    input[length]=0;length=0;
    if(strcmp(input,"STATUS")==0)status();
    else if(strcmp(input,"TREE")==0){lv_obj_update_layout(root);tree(root);Serial.println("END_TREE");}
    else if(strcmp(input,"SHOT")==0)screenshot();
    else if(strncmp(input,"TAP ",4)==0){int x,y;if(sscanf(input+4,"%d %d",&x,&y)==2&&x>=0&&x<480&&y>=0&&y<480){injectedX=x;injectedY=y;injectedUntil=esp_timer_get_time()+120000;Serial.println("OK TAP");}}
    else if(strncmp(input,"TIME ",5)==0){uint64_t t=strtoull(input+5,nullptr,10);setClock(t);render();status();}
    else if(strcmp(input,"TEST BEGIN")==0&&!testing){testBackup=puppy;testEpochBase=epochBase;testEpochMicros=epochMicros;testClockOK=clockOK;testBrightness=brightness;testing=true;Serial.println("OK TEST BEGIN (saving suspended)");}
    else if(strcmp(input,"TEST END")==0&&testing){puppy=testBackup;epochBase=testEpochBase;epochMicros=testEpochMicros;clockOK=testClockOK;brightness=testBrightness;boardBrightness(brightness);testing=false;view=Home;fetchScore=-1;activity=0;render();Serial.println("OK TEST END (progress restored)");}
    else if(strcmp(input,"SAVE")==0){savePet();status();}
    else Serial.println("ERROR unknown command");
  }
}
void setup(){
  Serial.begin(115200);delay(350);
  Serial.printf("BISCUIT boot flash=%u psram=%u reset=%d\n",ESP.getFlashChipSize(),ESP.getPsramSize(),esp_reset_reason());
  if(ESP.getFlashChipSize()<16*1024*1024||ESP.getPsramSize()<8*1024*1024||!boardBegin()){
    Serial.println("FATAL board initialization failed");while(true)delay(1000);
  }
  setenv("TZ",timezoneRule,1);tzset();
  savedOK=prefs.begin("biscuit",false);
  Save s{};bool loaded=savedOK&&prefs.isKey("pet1")&&prefs.getBytesLength("pet1")==sizeof(s)&&prefs.getBytes("pet1",&s,sizeof(s))==sizeof(s)&&s.magic==0x5a4f4531&&s.hash==checksum(&s.value,sizeof(s.value))&&pet::valid(s.value);
  epochBase=boardClockRead();clockOK=epochBase>=946684800ULL && epochBase<4102444800ULL;
  if(!clockOK)epochBase=loaded?s.value.updatedAt:1767225600ULL;
  epochMicros=esp_timer_get_time();
  puppy=loaded?s.value:pet::create(nowSeconds(),dayOrdinal());pet::tick(puppy,nowSeconds(),dayOrdinal());
  if(!loaded && savedOK)savePet();
  brightness=std::min(100,std::max(10,int(prefs.getUChar("brightness",75))));boardBrightness(brightness);
  lv_init();scenePixels=static_cast<uint16_t*>(heap_caps_malloc(160*160*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  auto buffer=static_cast<lv_color_t*>(heap_caps_malloc(480*40*sizeof(lv_color_t),MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
  if(!scenePixels||!buffer){Serial.println("FATAL display memory allocation failed");while(true)delay(1000);}
  lv_disp_draw_buf_init(&drawBuffer,buffer,nullptr,480*40);
  lv_disp_drv_init(&displayDriver);displayDriver.hor_res=480;displayDriver.ver_res=480;
  displayDriver.flush_cb=flush;displayDriver.draw_buf=&drawBuffer;lv_disp_drv_register(&displayDriver);
  static lv_indev_drv_t input;lv_indev_drv_init(&input);input.type=LV_INDEV_TYPE_POINTER;input.read_cb=touch;lv_indev_drv_register(&input);
  root=lv_scr_act();lv_obj_clear_flag(root,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_pad_all(root,0,0);
  lv_obj_set_style_border_width(root,0,0);lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);
  render();Serial.printf("READY stories=7 discoveries=96 loaded=%d\n",loaded);status();
}
void loop(){
  static uint32_t lastTick=millis(),lastFrame=0,lastNeeds=0,lastSave=0,lastSurprise=0;
  uint32_t ms=millis();lv_tick_inc(ms-lastTick);lastTick=ms;
  lv_timer_handler();
  if(pendingCommand){int c=pendingCommand;pendingCommand=0;command(c);}
  serialCommands();
  if(ms-lastFrame>=250){lastFrame=ms;if(activity&&fetchScore<0&&nowSeconds()>activityUntil)activity=0;animate();}
  if(ms-lastSurprise>=90000 && view==Home && !puppy.sleeping && fetchScore<0 && activity==0){
    lastSurprise=ms;
    for(int n=0;n<6;n++){int id=(ms/90000+n)%6;if(puppy.tricks[id]>=3){say("Hey, look! I've been practicing.",6+id);render();break;}}
  }
  if(ms-lastNeeds>=30000){lastNeeds=ms;pet::tick(puppy,nowSeconds(),dayOrdinal());if(view==Home)render();}
  if(ms-lastSave>=300000){lastSave=ms;savePet();}
  delay(3);
}
