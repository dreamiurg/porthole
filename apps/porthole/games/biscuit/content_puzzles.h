// Biscuit's puzzles content: spelling words (one letter goes missing) with a clue each, the things in the times
// stories, the Puzzles buttons and what the pup does after an answer. Shown as written, except CHEERS and NUDGES, which
// take {pet}. Three-letter and four-letter short words are for early readers (start of first grade), long ones for
// older kids (up to third grade); words are 2 to 11 lowercase letters (check_content.py). The review queue keeps a
// missed word by its place in its list and a story by its place in GROUP_THINGS, so append to the lists: a word moved
// or removed only changes which one comes back (puzzles::playable drops a place past the end).
#pragma once

namespace biscuit {
struct SpellWord { const char* word; const char* clue; };

inline constexpr SpellWord SHORT_WORDS[] = {
  {"dog", "A pup like me!"},
  {"cat", "A pet that says meow."},
  {"sun", "It keeps us warm in the sky."},
  {"bed", "Where you sleep at night."},
  {"hat", "You wear it on your head."},
  {"cup", "You drink from it."},
  {"pig", "A pink farm animal that oinks."},
  {"bus", "A big ride to school."},
  {"egg", "A hen lays it."},
  {"fox", "A wild animal with a bushy tail."},
  {"hen", "A bird that lays eggs."},
  {"jam", "Sweet and sticky, on toast."},
  {"map", "It shows you the way."},
  {"box", "You can keep toys in it."},
  {"leg", "You walk on it."},
  {"bone", "My favorite thing to chew!"},
  {"ball", "I love to chase it."},
  {"fish", "It swims in the water."},
  {"frog", "A green hopper by the pond."},
  {"duck", "A bird that says quack."},
  {"cake", "A treat for a birthday."},
  {"kite", "It flies on a windy day."},
  {"tree", "It has a trunk and leaves."},
  {"moon", "It shines in the night sky."},
  {"star", "A tiny light in the night sky."},
  {"boat", "It floats on the water."},
  {"milk", "A white drink."},
  {"book", "We read it together."},
  {"nest", "A home for baby birds."},
  {"hand", "It has five fingers."},
  {"sock", "It keeps your foot warm."},
  {"rain", "Water falling from clouds."},
  {"tail", "I wag it when I'm happy."},
  {"nose", "I sniff with it."},
};
inline constexpr SpellWord LONG_WORDS[] = {
  {"puppy", "A baby dog."},
  {"friend", "Someone you love to play with."},
  {"school", "Where you learn every day."},
  {"garden", "Flowers and carrots grow there."},
  {"rabbit", "A hopper with long ears."},
  {"pocket", "A little bag sewn in your coat."},
  {"basket", "You carry apples in it."},
  {"winter", "The cold, snowy time of year."},
  {"summer", "The hot, sunny time of year."},
  {"yellow", "The color of a banana."},
  {"purple", "The color of grapes."},
  {"kitten", "A baby cat."},
  {"button", "It keeps your shirt closed."},
  {"pencil", "You write and draw with it."},
  {"window", "You look outside through it."},
  {"bridge", "It takes a road over a river."},
  {"castle", "A home for a king and queen."},
  {"rocket", "It flies up into space."},
  {"planet", "Earth is one of these."},
  {"turtle", "It carries its shell on its back."},
  {"monkey", "It swings from tree to tree."},
  {"cookie", "A sweet treat from the oven."},
  {"pillow", "Soft, under your head at night."},
  {"jacket", "A coat you zip up."},
  {"ladder", "You climb it to reach up high."},
  {"orange", "A fruit, and a color too."},
  {"banana", "A long yellow fruit."},
  {"number", "1, 2 and 3 are each one of these."},
  {"animal", "A dog, a cow or a bird."},
  {"birthday", "The day you turn a year older."},
  {"elephant", "A big gray animal with a trunk."},
  {"dinosaur", "A huge animal from long ago."},
  {"umbrella", "It keeps you dry in the rain."},
  {"snowman", "You build it in the winter."},
  {"rainbow", "Colors in the sky after rain."},
  {"blanket", "It keeps you warm in bed."},
  {"chicken", "A farm bird that clucks."},
  {"kitchen", "Where we cook dinner."},
  {"giraffe", "The animal with the longest neck."},
  {"penguin", "A bird that swims but can't fly."},
  {"teacher", "Who helps you learn at school."},
  {"picture", "You draw or paint it."},
  {"pumpkin", "A big orange fruit in the fall."},
  {"dolphin", "A smart sea animal that jumps."},
  {"octopus", "A sea animal with eight arms."},
  {"sandwich", "Bread with a filling inside."},
  {"mountain", "A very tall hill."},
  {"airplane", "It flies people far away."},
};
inline constexpr int SHORT_WORD_COUNT = sizeof SHORT_WORDS / sizeof SHORT_WORDS[0];
inline constexpr int LONG_WORD_COUNT = sizeof LONG_WORDS / sizeof LONG_WORDS[0];
// Words a wrong letter must not spell, so no wrong answer is also right (b_g: bag, big, bug) or rude. A gap is asked
// only where at least two wrong letters spell none of these and none of the words above.
inline constexpr const char* REAL_WORDS[] = {
  "bag", "bog", "cog", "fog", "hog", "log", "dig", "dug", "doc", "don", "dot", "bat", "fat", "mat", "pat", "rat", "sat",
  "cot", "cut", "cab", "cad", "cam", "can", "cap", "car", "bun", "fun", "gun", "nun", "pun", "run", "sin", "son",
  "sub", "sum", "sup", "fed", "led", "red", "wed", "bad", "bid", "bud", "beg", "bet", "had", "hag", "ham", "has",
  "hit", "hot", "hut", "pup", "cop", "cub", "cum", "big", "fig", "gig", "rig", "wig", "peg", "pug", "pit", "pin",
  "pip", "pic", "nig", "bug", "bum", "but", "pox", "fax", "fix", "for", "ben", "den", "men", "pen", "ten", "her",
  "hem", "hum", "dam", "ram", "jab", "jag", "jar", "jaw", "gap", "lap", "nap", "rap", "sap", "tap", "mop", "mad",
  "man", "bob", "bow", "keg", "lag", "lug", "let", "fag", "cone", "done", "gone", "lone", "none", "tone", "bane",
  "bore", "bode", "call", "fall", "hall", "mall", "tall", "wall", "bell", "bill", "bull", "bald", "balm", "dish",
  "wish", "fist", "from", "buck", "luck", "muck", "puck", "suck", "tuck", "fuck", "deck", "dick", "dock", "dusk",
  "dunk", "bake", "fake", "lake", "make", "rake", "sake", "take", "wake", "coke", "cafe", "cage", "came", "cane",
  "cape", "care", "case", "bite", "site", "mite", "free", "thee", "true", "boon", "noon", "soon", "moan", "mood",
  "moor", "scar", "spar", "stir", "stab", "stag", "coat", "goat", "moat", "beat", "boot", "bout", "boar", "silk",
  "mink", "mild", "mill", "cook", "hook", "look", "took", "boom", "best", "pest", "rest", "test", "west", "newt",
  "band", "land", "sand", "wand", "hind", "hard", "hang", "lock", "mock", "rock", "sack", "sick", "cock", "gain",
  "main", "pain", "ruin", "rail", "raid", "fail", "hail", "mail", "nail", "pail", "sail", "toil", "dose", "hose",
  "lose", "pose", "rose", "node", "nope", "note", "mitten", "socket", "locket", "mutton", "simmer", "dimmer",
};
inline constexpr int REAL_WORD_COUNT = sizeof REAL_WORDS / sizeof REAL_WORDS[0];

// The times stories: "3 bowls, 4 biscuits in each." and "12 bones shared by 3 pups." (puzzles::build).
struct GroupThing { const char* groups; const char* things; };
inline constexpr GroupThing GROUP_THINGS[] = {
  {"bowls", "biscuits"}, {"bags", "bones"}, {"boxes", "balls"}, {"plates", "treats"},
};
inline constexpr int GROUP_THING_COUNT = sizeof GROUP_THINGS / sizeof GROUP_THINGS[0];
inline constexpr const char* GROUP_ASK = "How many?";
inline constexpr const char* SHARE_ASK = "How many each?";

// The Puzzles screen's buttons (puzzles::Strand order), shown as written.
inline constexpr const char* STRAND_LABELS[3] = {"Plus and minus", "Times", "Words"};
// The round's end, when the round moved the kid's level up.
inline constexpr const char* LEVEL_UP = "Next time: harder ones!";
// What the pup does after an answer, a different one each question of a round. {pet} is filled in.
inline constexpr const char* CHEERS[5] = {"{pet} wags his tail.", "{pet} does a happy spin.",
                                          "{pet} gives you a high paw.", "{pet} barks, just once.",
                                          "{pet} leans on your knee."};
inline constexpr const char* STAR_CHEER = "+1 star for you and {pet}!";   // instead of a cheer, when a star comes
inline constexpr const char* NUDGES[5] = {"{pet} nudges your hand.", "{pet} thinks that one was tricky.",
                                          "{pet} tilts his head.", "{pet} wags anyway.", "{pet} sniffs the next one."};
}  // namespace biscuit
