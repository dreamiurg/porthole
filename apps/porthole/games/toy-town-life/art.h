#pragma once
#include "font.h"
#include "gfx.h"
#include "gfx565.h"
#include "state.h"

namespace toytown { namespace art {
extern const font::Font BODY, TITLE, ACTION;
// Original scenes: street, home, school, cafe, park; each fills the 480px panel.
void background(int scene);
void placePicture(int scene, const gfx565::Rect& box);
// Original anime/plush/clay/keychain dolls, transparent 96x160 base, integer scale.
// All saved cosmetic fields change visible materials, shapes or accessories.
void doll(const State& state, int x, int y, int scale);
// Cropped or scaled views of the same production doll: full, head, fabric, shoes.
void detail(const State& state, const gfx565::Rect& box, int part);
enum Npc { MINA, THEO, NORI, SERVER, NPC_COUNT };
enum Prop { PICTURES, CUE_CARDS, PAPER, NOTEBOOK, RIBBON, SANDWICH, FRUIT, WATER, COOKIE, BAG, PENCIL, CAMERA, BOOK, TRAY, HUNG_PICTURES, PICNIC_CLOTH, MAGNIFIER, FLOWER_PICTURE, ROCKET_PICTURE, RABBIT_PICTURE, PROP_COUNT };
// Mina: 0 card, 1 waiting, 2 rehearsing. Theo: 0 picture, 1 planning, 2 hanging.
void npc(int id, const gfx565::Rect& box, int pose = 0);
// Upper 55% of the actual production NPC, for expressive pictured controls.
void npcPortrait(int id, const gfx565::Rect& box, int pose = 0);
void prop(int id, const gfx565::Rect& box);
// phase in milliseconds: subtle alternating footfall, same saved cosmetics.
void walkingDoll(const State& state, const gfx565::Rect& box, int phase);
uint16_t swatch(const State& state, int category);
const gfx::Sprite& icon();
}}  // namespace toytown::art
