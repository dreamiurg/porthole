// Biscuit's puzzles content: spelling words (one letter goes missing) with a clue each, the Puzzles buttons and what
// the pup does after an answer. Shown as written, except CHEERS and NUDGES, which take {pet}. Short words are for
// early readers (start of first grade), long ones for older kids (up to third grade). Nothing is persisted by index
// here, so the lists may grow or change freely; words are 2 to 11 lowercase letters (check_content.py).
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
};
inline constexpr int SHORT_WORD_COUNT = sizeof SHORT_WORDS / sizeof SHORT_WORDS[0];
inline constexpr int LONG_WORD_COUNT = sizeof LONG_WORDS / sizeof LONG_WORDS[0];

// The Puzzles screens' buttons (puzzles::Kind order; then the times tables 2 to 6 and all of them), shown as written.
inline constexpr const char* KIND_LABELS[6] = {"Adding", "Taking away", "Big numbers", "Times tables", "Short words",
                                               "Long words"};
inline constexpr const char* TABLE_LABELS[6] = {"Twos", "Threes", "Fours", "Fives", "Sixes", "All mixed"};
// What the pup does after an answer, a different one each question of a round. {pet} is filled in.
inline constexpr const char* CHEERS[5] = {"{pet} wags his tail.", "{pet} does a happy spin.",
                                          "{pet} gives you a high paw.", "{pet} barks, just once.",
                                          "{pet} leans on your knee."};
inline constexpr const char* STAR_CHEER = "+1 star for you and {pet}!";   // instead of a cheer, when a star comes
inline constexpr const char* NUDGES[5] = {"{pet} nudges your hand.", "{pet} thinks that one was tricky.",
                                          "{pet} tilts his head.", "{pet} wags anyway.", "{pet} sniffs the next one."};
}  // namespace biscuit
