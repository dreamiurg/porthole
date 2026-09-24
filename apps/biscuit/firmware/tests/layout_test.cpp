// Native content-fit check using the device's LVGL font metrics and paginator.
// Layout budgets below mirror render() in src/main.cpp. Keep them in sync when
// moving controls; the next control/image is a hard boundary, not extra room.
#include <lvgl.h>
#include "assets.h"
#include "fonts.h"
#include "pet.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// main.cpp uses these aliases for the generated fonts, including the paginator.
#define lv_font_montserrat_16 biscuit_font_16
#define lv_font_montserrat_20 biscuit_font_20
#define lv_font_montserrat_24 biscuit_font_24
#define lv_font_montserrat_28 biscuit_font_28
#include "paginate.inc"

namespace a = biscuitassets;
static unsigned failures = 0, measured = 0, pageCount = 0;

static void require(bool condition, const std::string& name, const std::string& reason) {
  if (!condition) {
    ++failures;
    std::fprintf(stderr, "FAIL %s: %s\n", name.c_str(), reason.c_str());
  }
}

static lv_point_t measure(const std::string& text, const lv_font_t& font, int width, int spacing = 4) {
  lv_point_t size{};
  lv_txt_get_size(&size, text.c_str(), &font, 0, spacing, width, LV_TEXT_FLAG_NONE);
  return size;
}

static bool inCircle(int x, int y) {
  // Pixel centers against the physical circular 480 x 480 display.
  const double dx = x + .5 - 240, dy = y + .5 - 240;
  return dx * dx + dy * dy <= 240 * 240;
}

static void fit(const std::string& name, const std::string& text, const lv_font_t& font,
                int x, int y, int width, int height, int spacing = 4, bool centered = true) {
  ++measured;
  const auto size = measure(text, font, width, spacing);
  require(size.x <= width && size.y <= height, name,
          std::to_string(size.x) + "x" + std::to_string(size.y) + "px exceeds " +
          std::to_string(width) + "x" + std::to_string(height) + "px: " + text);

  // Check glyph boxes, not the full label rectangle: centered text may safely
  // occupy a wide label near the circle's poles. Use LVGL's own line breaking,
  // UTF-8 decoding, kerning and baseline offsets for each rendered glyph.
  bool clipped = false, missing = false;
  uint32_t start = 0;
  int lineY = y;
  while (start < text.size()) {
    const uint32_t length = _lv_txt_get_next_line(text.c_str() + start, &font, 0, width, nullptr, LV_TEXT_FLAG_NONE);
    require(length > 0, name, "LVGL made no line-breaking progress");
    if (!length) break;
    const int lineWidth = lv_txt_get_width(text.c_str() + start, length, &font, 0, LV_TEXT_FLAG_NONE);
    int pen = x + (centered ? (width - lineWidth) / 2 : 0);
    uint32_t cursor = start;
    while (cursor < start + length) {
      const uint32_t code = _lv_txt_encoded_next(text.c_str(), &cursor);
      uint32_t nextCursor = cursor;
      const uint32_t next = _lv_txt_encoded_next(text.c_str(), &nextCursor);
      if (code == '\n' || code == '\r') continue;
      lv_font_glyph_dsc_t glyph{};
      if (!font.get_glyph_dsc(&font, &glyph, code, next) || glyph.is_placeholder) {
        missing = true;
        continue;
      }
      if (glyph.box_w && glyph.box_h) {
        const int left = pen + glyph.ofs_x;
        const int top = lineY + font.line_height - font.base_line - glyph.box_h - glyph.ofs_y;
        const int right = left + glyph.box_w - 1, bottom = top + glyph.box_h - 1;
        clipped |= !inCircle(left, top) || !inCircle(right, top) || !inCircle(left, bottom) || !inCircle(right, bottom);
      }
      pen += glyph.adv_w;
    }
    start += length;
    lineY += font.line_height + spacing;
  }
  require(!clipped, name, "glyphs reach outside the round display: " + text);
  require(!missing, name, "font is missing a character: " + text);
}

static void button(const std::string& name, const std::string& text,
                   int x, int y, int width, int height, const lv_font_t& font = biscuit_font_20,
                   int labelWidth = 0, int radius = 7) {
  const int textWidth = labelWidth ? labelWidth : width - 14;
  const auto size = measure(text, font, textWidth, 0);
  // Two pixels of border and five of padding on each side. Thumbnail labels
  // align to the content area's right edge; ordinary labels center in it.
  // Padding positions children but is not a clip boundary in LVGL: a centered
  // 51px three-line label fits a 64px button without touching its 2px border.
  const int textX = labelWidth ? x + width - 7 - textWidth : x + 7;
  fit(name, text, font, textX, y + (height - size.y) / 2, textWidth, height - 4, 0);
  bool clipped = false;
  for (int row = 0; row < height; ++row) {
    int inset = 0;
    if (row < radius || row >= height - radius) {
      const double dy = row < radius ? radius - row - .5 : row - (height - radius) + .5;
      inset = static_cast<int>(std::ceil(radius - std::sqrt(radius * radius - dy * dy) - .5));
    }
    clipped |= !inCircle(x + inset, y + row) || !inCircle(x + width - 1 - inset, y + row);
  }
  require(!clipped, name, "rounded button boundary reaches outside the round display");
}

static void top(const std::string& name, const std::string& text, int nextY = 145) {
  fit(name, text, biscuit_font_24, 70, 100, 340, nextY - 100);
}

static void prose(const std::string& name, const char* text) {
  const auto pages = paginate(text);
  require(!pages.empty(), name, "paginator returned no pages");
  std::string joined;
  for (size_t i = 0; i < pages.size(); ++i) {
    ++pageCount;
    fit(name + " page " + std::to_string(i + 1), pages[i], biscuit_font_24, 64, 146, 352, 176, 4, false);
    if (!joined.empty()) joined += ' ';
    joined += pages[i];
  }
  std::string normalized;
  bool space = false;
  for (const unsigned char ch : std::string(text)) {
    if (std::isspace(ch)) { space = !normalized.empty(); continue; }
    if (space) normalized += ' ';
    normalized += static_cast<char>(ch); space = false;
  }
  require(joined == normalized, name, "pagination lost or duplicated content");
}

static const char* cue(a::Cue value) {
  switch (value) {
    case a::Cue::Up: return "Up"; case a::Cue::Down: return "Down";
    case a::Cue::Left: return "Left"; case a::Cue::Right: return "Right";
    case a::Cue::Paw: return "Paw";
  }
  return "INVALID";
}

static const char* cue(pet::Cue value) {
  switch (value) {
    case pet::Cue::Up: return "Up"; case pet::Cue::Down: return "Down";
    case pet::Cue::Left: return "Left"; case pet::Cue::Right: return "Right";
    case pet::Cue::Paw: return "Paw";
  }
  return "INVALID";
}

int main() {
  static_assert(a::StoryCount == 7 && a::DiscoveryCount == 96 && a::TrickCount == 6 && a::AdventureCount == 7,
                "Exported collection sizes must match the fixed pet save model");
  static_assert(static_cast<unsigned>(pet::Action::Feed) == a::Feed);
  static_assert(static_cast<unsigned>(pet::Action::Play) == a::Play);
  static_assert(static_cast<unsigned>(pet::Action::Petting) == a::Pet);
  static_assert(static_cast<unsigned>(pet::Action::Read) == a::Read);
  static_assert(static_cast<unsigned>(pet::Action::Train) == a::Train);
  static_assert(static_cast<unsigned>(pet::Action::Rest) == a::Rest);
  lv_mem_init();
  fit("home name", "BISCUIT", biscuit_font_20, 80, 58, 130, 24, 0, false);
  fit("home mood", "Day 1 · puppy", biscuit_font_16, 220, 60, 180, 20, 0);
  for (int column = 0; column < 4; ++column)
    button("home action", column == 0 ? "Feed" : column == 1 ? "Read" : column == 2 ? "Play" : "More",
           96 + column * 73, 355, 68, 72);
  button("world back", "<", 86, 52, 56, 56, biscuit_font_28, 0, 28);
  fit("world heading", "BISCUIT & YOU", biscuit_font_20, 150, 68, 180, 24, 0, false);
  fit("world summary", "Day 1 · Puppy · 6 friendship", biscuit_font_16, 64, 108, 352, 22, 0);
  for (int row = 0; row < 2; ++row) for (int column = 0; column < 2; ++column)
    button("world card", "", 66 + column * 178, 145 + row * 104, 170, 96);
  fit("world today title", "Today's adventure", biscuit_font_20, 108, 155, 118, 44, 0, false);
  fit("world tricks title", "Learn tricks", biscuit_font_20, 286, 155, 118, 44, 0, false);
  fit("world scrapbook title", "Our scrapbook", biscuit_font_20, 108, 259, 118, 44, 0, false);
  fit("world nap title", "Cozy nap", biscuit_font_20, 286, 259, 118, 44, 0, false);
  for (int row = 0; row < 2; ++row) for (int column = 0; column < 2; ++column)
    fit("world card detail", "Something to discover", biscuit_font_16,
        76 + column * 178, 203 + row * 104, 150, 34, 0, false);
  button("world settings", "Settings · brightness & clock", 110, 367, 260, 56, biscuit_font_16);
  for (const auto& story : a::stories) {
    const std::string id = story.id;
    for (int row = 0; row < 2; ++row) {
      button(id + " bookshelf", std::string(story.title) + " *", 60, 213 + row * 66, 360, 60);
      button(id + " locked bookshelf", "Day " + std::to_string(story.unlockDay) + ": " + story.title, 60, 213 + row * 66, 360, 60);
    }
    prose(id, story.opening);
    fit(id + " prompt", story.prompt, biscuit_font_24, 64, 148, 352, 256 - 148);
    for (int choice = 0; choice < 2; ++choice) {
      button(id + " choice " + std::to_string(choice), story.choices[choice].label, 78, 256 + choice * 80, 324, 68);
      prose(id + " ending " + std::to_string(choice), story.choices[choice].ending);
    }
  }
  for (const auto& fact : a::discoveries) {
    const std::string id = fact.id;
    const auto& listFont = measure(fact.title, biscuit_font_20, 230, 0).y > 52 ? biscuit_font_16 : biscuit_font_20;
    for (int row = 0; row < 2; ++row)
      button(id + " list title", fact.title, 66, 210 + row * 70, 348, 64, listFont, 230);
    const auto& coverFont = measure(fact.title, biscuit_font_24, 348).y > 76 ? biscuit_font_20 : biscuit_font_24;
    fit(id + " cover title", fact.title, coverFont, 66, 146, 348, 228 - 146);
    prose(id, fact.text);
    fit(id + " wonder", fact.wonder, biscuit_font_24, 64, 194, 352, 374 - 194);
    fit(id + " source name", fact.sourceName, biscuit_font_24, 64, 156, 352, 220 - 156);
    fit(id + " source URL", fact.sourceUrl, biscuit_font_16, 74, 220, 332, 374 - 220, 4, false);
  }
  for (const auto& topic : a::topics) {
    top(std::string(topic.id) + " header", topic.name);
    for (int row = 0; row < 3; ++row)
      button(std::string(topic.id) + " topic", topic.name, 82, 153 + row * 61, 316, 56, biscuit_font_20, 200);
  }
  const char* actionNames[] = {"A little snack", "Play fetch", "A cuddle", "Read together", "Try a trick", "A cozy nap"};
  for (size_t day = 0; day < a::AdventureCount; ++day) {
    const auto& adventure = a::adventures[day];
    const std::string id = "daily " + std::to_string(day);
    top(id + " title", adventure.title, 147);
    fit(id + " description", adventure.description, biscuit_font_16, 65, 147, 350, 228 - 147);
    top(id + " word", adventure.word, 156);
    fit(id + " meaning", adventure.meaning, biscuit_font_24, 64, 156, 352, 374 - 156, 4, false);
    int row = 0;
    for (int bit = 0; bit < 6; ++bit) if (adventure.actions & (1 << bit)) {
      button(id + " action", std::string("* ") + actionNames[bit], 84, 228 + row * 57, 312, 56);
      ++row;
    }
    require(row == 3 && 228 + (row - 1) * 57 + 56 <= 404, id, "daily activities overlap the word button");
    require(adventure.actions == pet::dailyMask(static_cast<int>(day)), id, "exported daily actions differ from pet model");
    require(adventure.actions == pet::dailyMask(static_cast<int>(day) - 14), id, "negative day rollover differs");
  }
  button("daily word", "A lovely word", 142, 404, 196, 56);
  const char* trickIds[] = {"sit", "paw", "spin", "bow", "jump", "roll"};
  for (size_t index = 0; index < a::TrickCount; ++index) {
    const auto& trick = a::tricks[index];
    const std::string id = trick.id;
    require(id == trickIds[index], id, "trick index no longer matches the pet model");
    require(trick.unlockDay == pet::trickUnlockDay(index), id, "unlock day differs from the pet model");
    top(id + " header", trick.name, 152);
    for (int row = 0; row < 3; ++row) {
      button(id + " list", std::string(trick.name) + "   3/3", 82, 153 + row * 61, 316, 56);
      button(id + " locked list", std::string(trick.name) + "   Day " + std::to_string(trick.unlockDay), 82, 153 + row * 61, 316, 56);
    }
    for (uint8_t lesson = 0; lesson < 3; ++lesson) {
      const auto expected = pet::lesson(index, lesson);
      require(expected.length == trick.lessonLengths[lesson], id, "lesson length differs from pet model");
      require(trick.lessonLengths[lesson] <= 6, id, "lesson exceeds its six-cue storage");
      std::string pattern;
      for (uint8_t step = 0; step < std::min<uint8_t>(6, trick.lessonLengths[lesson]); ++step) {
        require(step < expected.length && std::strcmp(cue(expected.cues[step]), cue(trick.lessons[lesson][step])) == 0,
                id, "lesson cue differs from pet model");
        if (step) pattern += " - ";
        pattern += cue(trick.lessons[lesson][step]);
      }
      fit(id + " lesson " + std::to_string(lesson), pattern, biscuit_font_28, 66, 236, 348, 374 - 236);
    }
  }
  fit("training prompt", "Have a look, then try with me.", biscuit_font_24, 64, 152, 352, 236 - 152);
  button("training peek", "Peek again", 128, 393, 224, 56);
  for (const char* sticker : a::stickers)
    fit("sticker", sticker, biscuit_font_24, 80, 276, 320, 350 - 276);
  for (const char* text : {"Our bookshelf", "Story time", "The story continues", "What shall we do?",
       "Our little notebook", "Little discoveries", "So much to explore", "Where we found it",
       "Little paws, big ideas", "Our sticker album"}) top("shared header", text);
  button("previous", "Previous", 104, 374, 128, 56);
  button("next", "Next", 248, 374, 128, 56);
  button("discovery start", "Let's find out", 110, 384, 260, 56);
  button("source back", "Back to our book", 110, 374, 260, 56);
  button("word back", "Back to today", 110, 374, 260, 56);
  std::printf("Measured %u text layouts and %u real-paginator pages; %u failures.\n", measured, pageCount, failures);
  lv_mem_buf_free_all();
  return failures ? 1 : 0;
}
