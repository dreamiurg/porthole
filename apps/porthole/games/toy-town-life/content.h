#pragma once
#include "state.h"
namespace toytown {
struct Scene { const char* title; const char* words; const char* actions[3]; };
inline const Scene SCENES[PAGE_COUNT] = {
 {"Toy Town Life", "A tiny town. A story of your own.", {"Walk into town", "Doll & wardrobe", "Scrapbook"}},
 {"Meet your doll", "Four materials. Any style is yours.", {"Next doll", "Open wardrobe", "Home"}},
 {"Your wardrobe", "Everything here is free to try.", {"Change this", "Next category", "Done"}},
 {"Where shall we go?", "School, Cafe or Park? Pack a little bag before you go.", {"Next place", "Pack my bag", "Home"}},
 {"My three-slot bag", "Tap to pack or unpack. You can borrow things, too.", {"Pack / unpack", "Next item", "Let's go"}},
 {"School | Mina", "Will you sit with me? My art talk makes me nervous!", {"Ask what worries her", "Make a plan", "Recall my promise"}},
 {"Listen to Mina", "I forget my first words! A little note would help me begin.", {"Plan with Mina", "Read that again", "My promise"}},
 {"Two promises", "Theo needs help with displays, too. What can we manage?", {"Sit beside Mina", "Make cue cards", "Ask Theo"}},
 {"School | Theo", "I need help hanging pictures. I can help Mina after we hang them.", {"Plan together", "Listen to Mina", "Our promise"}},
 {"Paper to borrow", "No notebook? Our teacher has spare paper for cue cards.", {"Borrow paper", "Choose another plan", "Read again"}},
 {"The art talk", "", {"Talk after class", "Remember this", "Read again"}},
 {"After class", "", {"Help finish displays", "Talk with Mina", "Remember"}},
 {"Cafe | Lunch", "Ten pretend coins for lunch. Mina says: I'd like water, please!", {"See the menu", "Compare lunch ideas", "Ask Mina again"}},
 {"Cafe | Menu", "", {"Change quantity", "Next food", "See my tray"}},
 {"Cafe | My tray", "", {"Change my tray", "Compare trays", "Order lunch"}},
 {"Compare two trays", "", {"Keep this idea", "Change my tray", "My tray"}},
 {"Lunch together", "", {"Remember this", "Read again", "See our lunch"}},
 {"Park | Nori", "My sketchbook is missing! Can we look together?", {"Ask where it was", "Look around", "Recall the clues"}},
 {"Park | Look", "", {"Look here", "Next place", "My idea"}},
 {"What we noticed", "", {"Look elsewhere", "Try my idea", "Read again"}},
 {"Where could it be?", "", {"Search here", "Change my idea", "Look for clues"}},
 {"A new idea", "No notebook here. That's useful to know. Let's revise our idea.", {"Look again", "Change my idea", "Tell Nori"}},
 {"Found it!", "", {"Give it to Nori", "Look at our clues", "Read again"}},
 {"A small memory", "", {"Keep memory", "Add details", "Skip"}},
 {"A lovely stopping spot", "Your town is saved. You can leave now, or keep exploring.", {"Save & leave", "Keep exploring", "Scrapbook"}},
 {"Our scrapbook", "", {"Next memory", "Read caption", "Home"}},
 {"Make it yours", "", {"Change caption", "Change sticker", "Back"}},
 {"Quantity", "", {"Remove", "Add", "Done"}},
 {"Art display", "Theo: Choose a picture.\nThen tap a frame.", {"Flowers", "Rocket", "Rabbit"}},
};
inline const char* PLACES[] = {"Home", "School", "Cafe", "Park"};
inline const char* DOLLS[] = {"Fashion doll", "Plush rabbit", "Clay doll", "Keychain doll"};
inline const char* NAMES[] = {"Emi", "Pip", "Kai", "Lulu"};
inline const char* ITEMS[] = {"Notebook", "Pencil", "Water bottle", "Picnic cloth", "Magnifier"};
inline const char* FOOD[] = {"Sandwich", "Fruit bowl", "Water", "Cookie"};
inline const int PRICES[] = {5, 3, 1, 2};
inline const char* SPOTS[] = {"Bench", "Fountain", "Path"};
inline const char* LAST_SEEN[] = {
 "I drew at the bench. Then I ran to the swings. It has a pink ribbon.",
 "I drew by the fountain. Then I went to the swings. It has a pink ribbon.",
 "I drew on the path by the tree. Then I ran to the swings. Pink ribbon!"
};
inline const char* CLUES[] = {
 "A pink ribbon peeks under the bench. The wind slid something below it.",
 "A pink ribbon is behind the fountain pot. A dry ledge sheltered it.",
 "A pink ribbon is beside the tree roots. Something slipped off the path."
};
inline const char* EMPTY_CLUES[] = {
 "Only crumbs under this bench. No pink ribbon. Let's look elsewhere.",
 "Just a blue leaf by the fountain. No pink ribbon. Another place?",
 "Only pebbles on the path. No pink ribbon. We can change our idea."
};
inline const char* SCHOOL_ENDINGS[] = {
 "", "Mina feels less alone beside you. She pauses at her first words. Theo's display waits.",
 "Your cue cards help Mina start. Theo hangs fewer pictures while you rehearse.",
 "You make picture cards. Mina still forgets her first words. Ask what would help.",
 "Theo helps after hanging pictures. Mina waits, then practices her first words with him."
};
inline const char* CAPTIONS[] = {"We tried an idea.", "We changed our plan.", "We worked together."};
inline const char* STICKERS[] = {"Flower", "Star", "Leaf"};
} // namespace toytown
