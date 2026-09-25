// Biscuit's daily adventures, tricks and stickers. Plain data. Run adventure descriptions and meanings through
// personalize() before drawing. Order is stable: append, never reorder.
#pragma once
#include <stdint.h>

namespace biscuit {
// Display text only: which activities a day asks for (DAILY_MASKS), trick lessons and unlock days are rules, in pet.h.
// ADVENTURES[local day ordinal % 7] is today's. Finishing its three actions earns STICKERS[ordinal % 12].
struct Adventure {
  const char* title;
  const char* description;
  const char* word;               // the pocket word
  const char* meaning;
};
inline constexpr Adventure ADVENTURES[] = {
  {"A picnic for two",
   "A blanket, a book, and a biscuit each. {pet} has already chosen the sunny spot.", "Conjecture",
   "An idea you think might be true, before you can prove it. {pet} thinks every pocket contains a snack."},
  {"Puddles and pawprints",
   "{pet} found a puddle exactly his size. Now the rug has a little trail of flowers. Or pawprints.", "Corroborate",
   "To support a claim with more evidence. The muddy pawprints corroborate {pet}'s story about splashing in puddles."},
  {"The hidden biscuit",
   "{pet} tucked away a biscuit for later. His nose remembers. The rest of him is still thinking.", "Paradox",
   "Something that seems to contradict itself. The better {pet} hides his snack, the worse his chances of eating it."},
  {"The book on the top shelf",
   "{pet} can almost reach the book. Perhaps standing on tip-paws will help.", "Ingenuity",
   "Cleverness at finding an inventive solution. {pet} cannot reach the shelf, but he can bring {name} a sturdy step."},
  {"Under the sofa",
   "A sock looks like a sleeping dragon from down here. {pet} gives it a gentle nudge.", "Perspective",
   "A way of seeing something, shaped by where you stand or what you know. From {pet}'s perspective, the sofa is a mountain."},
  {"Look what I found!",
   "{pet} went looking for his ball and found a book instead. There is room for both on the blanket.", "Serendipity",
   "Finding something good by happy accident. {pet} hunted for a tennis ball and discovered the perfect story instead."},
  {"One more little try",
   "The ball rolled under the sofa again. {pet} has a wag, a wiggle, and another idea.", "Tenacity",
   "Determination to keep trying when something is difficult. {pet} tries a new way to reach his ball under the sofa."},
};
constexpr int ADVENTURE_COUNT = sizeof ADVENTURES / sizeof ADVENTURES[0];

struct Trick { const char* id; const char* name; };   // lessons and unlock days: biscuit::lesson(), trickUnlockDay()
inline constexpr Trick TRICKS[] = {
  {"sit", "Sit"},
  {"paw", "Shake a paw"},
  {"spin", "Twirl"},
  {"bow", "Take a bow"},
  {"jump", "Happy hop"},
  {"roll", "Roll over"},
};
constexpr int TRICK_COUNT = sizeof TRICKS / sizeof TRICKS[0];

inline constexpr const char* STICKERS[] = {
  "Moonbeam",
  "Little library",
  "Golden paw",
  "Daisy chain",
  "Cloud castle",
  "Brave dragon",
  "Rainbow scarf",
  "Comet tail",
  "Tiny lighthouse",
  "Magic acorn",
  "Cozy teacup",
  "Best friends",
};
constexpr int STICKER_COUNT = sizeof STICKERS / sizeof STICKERS[0];
}  // namespace biscuit
