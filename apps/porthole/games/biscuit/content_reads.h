// Biscuit's little reads: a short passage the kid reads alone, then one question with three answers to pick from
// (learning-principles.md, principle 11; the plan is adaptive-practice.md, piece E). Five levels, picked for each kid
// by pet.h's answerRead (three right in a row: up one, a miss: down one, which settles near 80% right):
//   1  first-grade start: short-vowel words of one syllable with single consonants (cat, sit, hop, bed; -ll -ss -ff
//      allowed) and a few sight words; 12 to 30 words. The answer is said in the text.
//   2  digraphs and blends (ship, chin, that, duck, ring, frog, stop, hand); 25 to 45 words. Said in the text, or
//      one plain step from it.
//   3  long vowels: silent e and vowel teams (cake, bike, rope, tree, rain, play, boat, snow); 40 to 65 words. Order
//      of events, or putting two sentences together.
//   4  two syllables, -ed and -ing, ar/or/er (garden, jumped, running, storm, under); 60 to 90 words. Why something
//      happened, or how someone felt, worked out from what they did.
//   5  third-grade start: any words; 75 to 120 words. Inference, what a word means here, the big idea.
// tools/check_content.py holds levels 1 and 2 to their letter patterns (a short list of sight words aside), every
// level to its word count, and checks the fields; host/test_biscuit_content.cpp measures every string in its box.
// Nothing is persisted by index here (a save keeps only the level and the count of right answers), so entries may be
// added, reordered or changed freely. text, question, answers and why take {name} and {pet}; the title is drawn
// as written. why is the "how" shown after a miss (principle 9): the sentence that holds the answer, or the two clues
// that lead to it.
#pragma once
#include <stdint.h>

namespace biscuit {
constexpr int READ_ANSWERS = 3;   // the levels are pet.h's READ_LEVELS
struct Read {
  const char* id;                       // unique, lowercase with hyphens
  uint8_t level;                        // 1..READ_LEVELS
  const char* title;                    // the header while it is read, one line
  const char* text;                     // the passage
  const char* question;                 // two lines at most
  const char* answers[READ_ANSWERS];    // one line each; all three plausible, only one right
  uint8_t right;                        // the index of the right answer
  const char* why;                      // shown after a miss, with the right answer
};

inline constexpr Read READS[] = {
  // ---- level 1
  {"red-cap", 1, "The red cap",
   "{pet} has a red cap. {name} has a tan cap. A big bug sits on the tan cap. The bug is not mad. It can sit.",
   "What cap has a bug on it?", {"The red cap", "The tan cap", "No cap"}, 1,
   "It says: A big bug sits on the tan cap."},
  {"mud-tub", 1, "Mud off",
   "{pet} ran in the mud. Mud is on his legs. {name} fills the tub. {pet} hops in. Fun, fun, fun! The mud is off. "
   "{pet} is wet.",
   "What is off the dog?", {"The mud", "The tub", "His legs"}, 0,
   "It says: The mud is off."},
  {"hot-day", 1, "So hot",
   "It is hot. {pet} did not sit on the hill. {pet} did not sit on the rug. He sits in the tub. Yes! It is wet.",
   "Where did the dog sit?", {"On the hill", "On the rug", "In the tub"}, 2,
   "It says: He sits in the tub."},
  {"big-bun", 1, "Two buns",
   "{pet} has a bun. {name} has a bun. {name} gets the big bun. {pet} gets a bun, but it is not big. {pet} is not mad. He has a nap.",
   "What bun is for {pet}?", {"The big bun", "A bun not big", "No bun"}, 1,
   "It says: {pet} gets a bun, but it is not big."},
  {"ham-pit", 1, "The ham",
   "{pet} has a ham. He digs a pit. The ham is in the pit. {pet} sits on the pit. A hen has a look. The hen can not get it.",
   "What is in the pit?", {"The ham", "The hen", "The mud"}, 0,
   "It says: The ham is in the pit."},
  {"no-hat", 1, "The hat",
   "{name} has no hat. {pet} digs in the bin. He gets a lid. He gets a pen. He gets a hat! It is the hat of {name}. "
   "{name} hugs {pet}.",
   "What did the kid not have?", {"A lid", "A pen", "A hat"}, 2,
   "It says: {name} has no hat."},
  {"box-bag", 1, "Box, bag, tub",
   "A hen is in the box. A fox is in the bag. A pig is in the tub. {pet} had a look at all. He did not yap.",
   "What is in the tub?", {"A pig", "A fox", "A hen"}, 0,
   "It says: A pig is in the tub."},
  {"up-hill", 1, "Up the hill",
   "{pet} runs up the hill. {name} runs up the hill. {pet} gets to the top. {name} is hot. {name} sits on a log. "
   "{pet} wags at the top.",
   "Is {name} on top?", {"Yes, on top", "No, on a log", "No, in a pit"}, 1,
   "It says: {name} sits on a log."},
  // ---- level 2
  {"hot-fish", 2, "Hot fish",
   "{pet} sniffs a dish. It is fish! He wants it fast. {name} said, \"Not yet. It is hot.\" {pet} sits still and licks his lips. Then {name} said, \"Yes!\"",
   "Why can the dog not get it yet?", {"It is hot", "It is a dish", "He is full"}, 0,
   "It says: Not yet. It is hot."},
  {"tent-drops", 2, "The tent",
   "{name} and {pet} sit in a tent. Drops splash on the top. {pet} sniffs at the flap. Then the drops stop. {pet} "
   "runs out to the wet grass. {name} grins.",
   "When did the dog run out?", {"When it is hot", "When {name} sits", "When the drops stop"}, 2,
   "It says: Then the drops stop. {pet} runs out."},
  {"stuck-stick", 2, "Stuck stick",
   "{pet} finds a stick. It is stuck in a bush. {pet} tugs and tugs. It will not pop out. {name} grabs the end and "
   "pulls. Pop! {pet} grins.",
   "Who got the stick out?", {"{pet}", "{name}", "No one"}, 1,
   "It says: {name} grabs the end and pulls."},
  {"frog-pond", 2, "The frog",
   "{name} and {pet} go to the pond. A frog sits on a rock. {pet} sniffs the frog. The frog jumps. Splash! {pet} "
   "gets wet. {name} grins.",
   "Why did the dog get wet?", {"A frog splash", "He fell in", "A fast wind"}, 0,
   "It says: The frog jumps. Splash! {pet} gets wet."},
  {"sand-flag", 2, "The flag",
   "{name} puts a red flag on the sand hill. {pet} sniffs the sand. {pet} digs next to the flag. The sand slips. The flag drops. {name} grins.",
   "Why did the flag drop?", {"{name} pulls it", "{pet} digs next to it", "A fast wind"}, 1,
   "It says: {pet} digs next to it. The sand slips."},
  {"sock-hunt", 2, "The next sock",
   "{name} has one sock. {pet} sniffs the bed. No sock. He sniffs the shelf. No sock. He digs in his bed. Yes! The "
   "sock is with a stick.",
   "Where was the sock?", {"On the shelf", "In the bin", "In his bed"}, 2,
   "It says: He digs in his bed. Yes! The sock is with a stick."},
  {"bun-plum", 2, "Bun or plum",
   "{name} has a bun and a plum on a dish. {pet} did not sniff the plum. He sniffs the bun and then he licks his "
   "lips. {name} grins.",
   "What does the dog want?", {"The plum", "The dish", "The bun"}, 2,
   "It says: He sniffs the bun and then he licks his lips."},
  {"cat-track", 2, "A thin track",
   "{pet} sniffs a track in the mud. It is thin and small. It is not a frog track. It is a cat track. It ends at the "
   "shed. {pet} sits still next to the shed.",
   "Where did the cat go?", {"To the shed", "To the pond", "Back to bed"}, 0,
   "It says: It ends at the shed."},
  // ---- level 3
  {"lost-kite", 3, "The lost kite",
   "{name} and {pet} take the kite to the hill. The wind is strong. The kite goes up, up, up. Then the string slips! "
   "The kite flies into a tall tree. {pet} sits at the trunk and waits. A gust of wind shakes the tree, and the kite "
   "floats down to {name}.",
   "How did they get the kite back?", {"{pet} got it", "The wind shook it down", "{name} got up the tree"}, 1,
   "It says: A gust of wind shakes the tree, and the kite floats down."},
  {"warm-bath", 3, "Bath time",
   "{name} runs the bath. The water is too hot, so {name} adds cold. {pet} puts one paw in and pulls it back out. "
   "{name} adds a little more cold and waits. This time {pet} steps in and sits down with a happy sigh.",
   "What made the dog step in?", {"A treat", "A toy", "More cold water"}, 2,
   "It says: {name} adds a little more cold and waits."},
  {"snow-bone", 3, "Snow bone",
   "Snow fell all night. In the morning, {pet} pushes the door open with his nose. The yard is white, and he cannot "
   "see his bone. He digs in the snow until he finds a lump. It is only a rock. He digs one more time by the tree "
   "and finds the bone.",
   "Where was the bone?", {"By the tree", "Under the rock", "By the door"}, 0,
   "It says: He digs one more time by the tree and finds the bone."},
  {"cup-trick", 3, "Three cups",
   "It is {pet}'s birthday, and {name} plays a game. A treat goes under the blue cup, a toy under the green cup, and a bone under the red cup. {pet} sniffs all three cups slowly. Then he sits down by the red cup and will not move, even when {name} calls him.",
   "What is under the cup he chose?", {"A treat", "A bone", "A toy"}, 1,
   "It says: a bone under the red cup. {pet} sits by the red cup."},
  {"library-step", 3, "At the library",
   "{name} walks {pet} to the library. A sign on the door says no dogs inside. {pet} yawns and lies down on the "
   "mat, with the wet grass behind him. {name} picks a book about boats and sits next to him on the step to read it.",
   "Why does the dog stay outside?", {"He is sleepy", "The grass is wet", "Dogs cannot go in"}, 2,
   "It says: A sign on the door says no dogs inside."},
  {"garden-row", 3, "The garden row",
   "{name} plants beans in the first hole, corn in the next, and peas in the last. Then {name} waters every hole and goes inside. Days later, {pet} sniffs the last hole and wags. A tiny green sprout peeks out, with a curly vine reaching for a stick.",
   "What grows where the dog sniffs?", {"Peas", "Beans", "Corn"}, 0,
   "It says: peas in the last hole. {pet} sniffs the last hole."},
  {"bus-morning", 3, "School morning",
   "The alarm rings. {name} wakes at six, and {pet} licks a sleepy face. {name} gets dressed, eats toast, and brushes teeth. Then the bus honks outside. {pet} walks {name} to the gate, wags goodbye, and waits there until the bus is gone.",
   "What comes right before the bus honks?", {"Eats toast", "Brushes teeth", "Gets dressed"}, 1,
   "It says: gets dressed, eats toast, and brushes teeth. Then the bus honks."},
  {"fence-cat", 3, "Fence cat",
   "A gray cat sits on the fence. {pet} stares at her, and she stares back. {pet} barks once, but the cat does not "
   "move. He barks again, louder. The cat yawns, stretches, and hops down on the other side. {pet} wags, because he "
   "thinks he won.",
   "What does the dog think?", {"He was bored", "He had lost", "He had won"}, 2,
   "It says: {pet} wags, because he thinks he won."},
  // ---- level 4
  {"supper-storm", 4, "Thunder at supper",
   "The thunder started just before supper, and the lights flickered. {pet} crawled under the table and pressed against {name}'s legs. He did not even sniff his dinner bowl, which was full of his favorite food. {name} stopped eating, sat on the floor, and rubbed his ears until the rumbling faded. By dessert, {pet} was snoring, and {name} finished the cold soup alone.",
   "How did the dog feel in the storm?", {"Scared", "Hungry", "Bored"}, 0,
   "He hid under the table and pressed close to {name}."},
  {"orange-cat", 4, "The orange cat",
   "Every afternoon, {pet} stood at the fence, staring at the neighbor's orange cat. She licked her paw and ignored him. One day {pet} whined softly, then picked up his tennis ball and dropped it at the gap in the boards. The ball rolled through. For a long moment nothing happened. Then the cat batted it back, and {pet} wiggled all over.",
   "What does the dog want from her?", {"Her food", "To play", "To chase her"}, 1,
   "He dropped his ball by the gap, and she batted it back."},
  {"blue-sock", 4, "The blue sock",
   "{name} searched everywhere for one blue sock: under the bed, in the laundry basket, behind the couch. {pet} followed along, sniffing every spot and wagging his tail helpfully. Finally {name} gave up and put on a green sock instead, even though it did not match. Later, passing {pet}'s bed, {name} noticed a blue corner under the blanket. {pet} looked away and pretended to nap.",
   "Who took the blue sock?", {"The cat", "{name}", "{pet}"}, 2,
   "A blue corner in his bed, and he looked away."},
  {"park-crackers", 4, "Park crackers",
   "At the park, {name} opened a bag of crackers on the bench. Two little squirrels crept closer, and {pet} growled softly, his fur rising. {name} tossed one cracker far across the grass. The squirrels scurried after it, and {pet} relaxed on the grass with a loud sigh and a big yawn. Then {name} gave {pet} the last cracker, crumbs and all.",
   "Why did the kid toss a cracker?", {"To feed {pet}", "To send them away", "To save it"}, 1,
   "Squirrels crept close and {pet} growled; then they ran after it."},
  {"deep-snow", 4, "Deep snow",
   "Snow covered the yard overnight, even the porch steps. {pet} leaped off the porch and vanished up to his ears. He bounced out, sneezing, with a white nose, and dove right back in. {name} laughed, grabbed a shovel, and cleared a narrow path to the gate. {pet} still preferred bouncing through the deep part, so {name} left that part alone for him.",
   "What does the dog think of snow?", {"He loves it", "He hates it", "He ignores it"}, 0,
   "He leaped in, bounced, and still preferred the deep part."},
  {"school-bus", 4, "Waiting at the window",
   "The house sounded quiet after {name} left for school. {pet} lay by the front window, ears twitching at every engine that passed. He sniffed the empty backpack hook by the door, then dozed through lunchtime, twitching in his dreams. Hours later, the yellow bus finally rumbled around the corner. He jumped to his feet and wiggled in circles by the door, whining.",
   "How did the dog feel about the bus?", {"Afraid", "Annoyed", "Excited"}, 2,
   "He jumped up and wiggled in circles by the door."},
  {"loose-ribbon", 4, "The present",
   "{name} spent all morning wrapping a surprise for Grandma and hid it behind the couch. {pet} sniffed the paper, tilted his head, and tugged one corner. The ribbon slid loose and the paper wrinkled. {name} sighed, then grinned, and wrapped the box again with an extra big bow and a fresh sheet of paper. This time the box went on the high shelf.",
   "Why did the kid wrap it again?", {"The paper was torn", "{pet} undid the ribbon", "It was the wrong gift"}, 1,
   "{pet} tugged a corner and the ribbon came loose."},
  {"blanket-tent", 4, "The blanket tent",
   "On a rainy afternoon, {name} spread blankets over the chairs and built a tent. {pet} crawled inside first and claimed the best pillow, circling twice before flopping down. {name} brought a flashlight, a book, and some crackers, but only {pet} fit on the pillow, so {name} sat on the cold floor and shared the crackers. The rain tapped on the roof.",
   "Why did the kid sit on the floor?", {"{pet} took the pillow", "The floor was warm", "The tent was small"}, 0,
   "He claimed the pillow, and only he fit on it."},
  // ---- level 5
  {"night-sounds", 5, "Night sounds",
   "{pet} could not sleep. Something outside the window went tap, tap, tap. He pressed his nose to the glass, but the "
   "yard was too dark to see. The tapping stopped, then started again, a little faster. {pet} remembered the "
   "afternoon: gray clouds, a cool wind, and the smell that always comes before a storm. He lay back down on his "
   "blanket. The tapping grew into a soft roar on the roof, and he was asleep before it ended.",
   "What was tapping on the window?", {"A bird", "Rain", "A friend"}, 1,
   "Gray clouds and a storm smell, then a soft roar on the roof: that is rain."},
  {"hide-seek", 5, "Hide and seek",
   "Every Saturday, {pet} and {name} played hide and seek in the hall. {pet} was a champion, though only at being found, mostly because he was terrible at hiding. This time he squeezed behind the curtain, but his tail stuck out and thumped the wall. {name} walked straight past the curtain and pretended not to hear. \"Where could he be?\" {name} said loudly. The thumping got faster. Finally {name} lifted the curtain, and {pet} burst out, delighted to be found.",
   "Why did the kid pretend not to notice?", {"To keep the game fun", "To scare {pet}", "To find another spot"}, 0,
   "The tail thumped, so {name} played along."},
  {"one-shoe", 5, "One shoe",
   "Every evening, {pet} brought {name} one shoe from the pile by the door. It was never a pair, and it was usually soggy. {name} grumbled about it for weeks, then noticed something odd: it was always the very shoe worn that day. One evening, after a long, hard afternoon at school, {name} knelt, held the damp shoe close, and finally understood why it arrived at the same time every day, right when the front door opened.",
   "What does the shoe show about the dog?", {"He likes chewing", "He is tidying up", "He missed {name}"}, 2,
   "It is the shoe {name} wore that day."},
  {"vet-room", 5, "The waiting room",
   "The vet's waiting room was crowded and warm. A beagle yapped at the ceiling, a kitten hissed in her carrier, and a parrot shouted a name nobody knew. A man dropped his keys, and three dogs barked at the clatter. {pet} sat perfectly still, pressed against {name}'s knee, and watched the commotion with wide eyes. When the nurse called his name, he stood up slowly, took one long breath, and walked toward the door, ears low but steady.",
   "What does \"commotion\" mean here?", {"A quiet nap", "Loud, busy noise", "A long line"}, 1,
   "A beagle yapped, a kitten hissed, a parrot shouted: lots of noise."},
  {"porch-weather", 5, "Staying on the porch",
   "On a bright, breezy morning, {pet} refused to leave the porch. {name} tugged the leash and offered a biscuit, but {pet} would not budge, and he kept sniffing toward the hills. \"The sky is clear,\" {name} said, and finally gave up and sat down beside him. An hour later, a wall of dark clouds rolled over the ridge, and rain thundered on the roof. {name} looked down at {pet}, then gave him the whole biscuit.",
   "What did the dog know first?", {"Rain was coming", "Dinner was late", "The leash was broken"}, 0,
   "He would not go out, then clouds rolled in: he sensed the storm."},
  {"shy-neighbor", 5, "The new boy",
   "A new boy moved in next door, and he was so shy that he stayed inside all week. {pet} sat at the fence every afternoon and said nothing, which is a dog's quiet way of being polite and kind. After a week, the boy slipped a biscuit through the boards. After two weeks, he waved. By the third week, he was tossing a ball with {name} while {pet} ran back and forth between them, tail swinging.",
   "How did the boy change?", {"He grew angry", "He grew tired", "He grew friendly"}, 2,
   "A biscuit, then a wave, then playing ball: shy to friendly."},
  {"vacuum-hall", 5, "The vacuum",
   "The vacuum cleaner roared in the hallway, and {pet} had to pass it to reach his water bowl. He was reluctant. He crept forward one paw at a time, backed up twice, and finally dashed past with his tail tucked, as if the machine might bite. He lapped up half the bowl in a hurry, then glanced back with one ear up. Behind it, {name} switched it off and laughed. \"It only eats dust,\" {name} said.",
   "What does \"reluctant\" mean here?", {"Unwilling", "Excited", "Sleepy"}, 0,
   "He crept, backed up, and dashed past: he did not want to go."},
  {"fridge-drawing", 5, "The fridge picture",
   "On a slow Sunday, {name} drew the family and taped it to the fridge: four stick people and one scribbly dog with long ears. {pet} sat below it, looking up. Later, {name} added a second dog, smaller, with a tiny red collar, waiting at the edge of the page. \"Whose dog is that?\" asked Dad. \"Nobody's yet,\" {name} said. \"I am just leaving room.\" Dad smiled and put a magnet on each corner of the page.",
   "What is the kid hoping for?", {"A bigger fridge", "Another dog someday", "A new picture"}, 1,
   "A smaller dog waits at the edge: {name} is leaving room for one."},
};
constexpr int READ_COUNT = sizeof READS / sizeof *READS;
}  // namespace biscuit
