// Game content: stories, spelling words, trick/hat/sticker names. Plain data.
#pragma once
#include <stdint.h>
#include "palette.h"

struct Book {
  const char* title;
  uint8_t cover;            // palette color of the cover
  uint8_t icon;             // 0 dog 1 moon 2 star 3 heart 4 leaf 5 bone 6 book 7 paw
  uint8_t level;            // 1 easy (age 6-7), 2 middle (7-8), 3 harder (8-10)
  const char* pages[12];    // nullptr-terminated list, max 11 pages
  const char* question;
  const char* answers[3];   // up to 3, nullptr-terminated
  uint8_t correct;
};

// Reading level: grade 2-3. Short sentences, ~20-30 words per page.
static const Book BOOKS[] = {
  {"The Lost Ball", C_RED, 0, 1, {
    "Pip was a small brown dog with one white ear. Pip loved one thing more than treats. Pip loved his red ball.",
    "One windy day the ball rolled away. It rolled down the hill. It rolled past the pond. Pip ran and ran.",
    "At the bottom of the hill sat a frog. \"Have you seen a red ball?\" asked Pip. The frog pointed with one green toe.",
    "The ball was stuck in the reeds. Pip splashed in. The water was cold! But Pip did not stop.",
    "Pip carried the ball all the way home. He was wet and muddy and very proud. \"Good dog,\" said Grandma. Pip agreed.",
    nullptr}, "Who helped Pip find the ball?", {"A frog", "A cat", "A duck"}, 0},
  {"Moon Puppy", C_NAVY, 1, 1, {
    "Every night Luna looked up at the moon. It was round and bright, like a big silver cookie.",
    "\"I will visit the moon,\" said Luna. She packed a bone, a blanket, and her favorite book.",
    "Luna climbed the tallest hill. She jumped as high as she could. She did not reach the moon. But she got very close!",
    "Luna lay on her blanket and read her book to the stars. The stars twinkled. They liked the story.",
    "When Luna woke up, the moon was gone. But the sun was up, and it was warm. \"Good morning, sun,\" said Luna.",
    nullptr}, "What did Luna pack for her trip?", {"A ball and a hat", "A bone and a book", "Just a bone"}, 1},
  {"Biscuit's Big Day", C_ORANGE, 5, 2, {
    "Today was Biscuit's first day at Puppy School. Biscuit was nervous. What if the other puppies did not like him?",
    "At school there was a big dog named Duke. Duke barked very loud. Biscuit hid behind a bush.",
    "Then Biscuit saw a tiny puppy who was scared too. Her name was Bean. \"Want to sit together?\" asked Biscuit.",
    "Biscuit and Bean learned to sit. They learned to stay. Duke tried to learn too, but he kept rolling over.",
    "At the end of the day everyone got a gold star. Even Duke. Biscuit was not nervous anymore. He had a friend.",
    nullptr}, "Why did Biscuit stop feeling nervous?", {"He got a treat", "He made a friend", "School ended"}, 1},
  {"The Rainy Day", C_BLUE, 4, 1, {
    "Drip, drop, drip. It rained all morning. Mochi pressed her nose on the window. No walk today.",
    "\"Rain is boring,\" sighed Mochi. Then she saw something. A tiny snail was crossing the porch.",
    "The snail was slow. Very slow. Mochi watched it for a whole hour. The snail did not seem bored at all.",
    "When the rain stopped, the snail reached a leaf. It had a little home under there. The snail was happy.",
    "Mochi went for her walk at last. She walked slowly, like the snail. She saw so many new things.",
    nullptr}, "What did Mochi watch on the porch?", {"A snail", "A bird", "A cat"}, 0},
  {"Where Is Bear?", C_PLUM, 3, 1, {
    "Ziggy had a toy bear. It was old and soft and smelled like home. One morning, Bear was gone!",
    "Ziggy looked under the bed. Only dust bunnies. Ziggy looked in the garden. Only real bunnies.",
    "\"Did you take Bear?\" Ziggy asked the cat. The cat yawned. Cats never tell you anything.",
    "Then Ziggy heard a giggle. Baby Sam was hugging Bear in his crib. Sam looked so happy.",
    "Ziggy thought about it. Then he curled up next to the crib. Bear could have two friends now.",
    nullptr}, "Who had Ziggy's toy bear?", {"The cat", "Baby Sam", "A bunny"}, 1},
  {"The Library Dog", C_LEAF, 6, 2, {
    "Olive worked at the library. She did not sort books. She could not read. But she had a very important job.",
    "Every Saturday, children came to read to Olive. Olive listened with her head on her paws.",
    "One boy named Theo read very quietly. He was afraid of making mistakes. Olive did not mind mistakes.",
    "Week after week, Theo read to Olive. His voice got bigger. His stories got longer. Olive wagged and wagged.",
    "One day Theo read a whole book out loud. Everyone clapped. Olive got a biscuit. It was her best day ever.",
    nullptr}, "What was Olive's job at the library?", {"Sorting books", "Listening to kids", "Guarding the door"}, 1},
  {"Snow Paws", C_LTGRAY, 2, 1, {
    "Cocoa had never seen snow. One morning the whole yard was white. Cocoa put one paw out. Cold!",
    "She put out another paw. Then she jumped in. Snow flew everywhere. It was the best thing ever.",
    "Cocoa made paw prints in a circle. Then a zigzag. Then a giant letter C, for Cocoa.",
    "A little bird landed on the fence. It was shivering. Cocoa dug a soft spot for it, out of the wind.",
    "That night the snow sparkled under the moon. Cocoa dreamed of a hundred birds and a thousand paw prints.",
    nullptr}, "What letter did Cocoa make in the snow?", {"B", "C", "S"}, 1},
  {"The Brave Little Bark", C_GOLD, 7, 2, {
    "Peanut was the smallest dog on the street. Her bark was small too. \"Yip!\" That was all.",
    "The big dogs laughed. \"You cannot scare anyone with a yip!\" Peanut felt very small indeed.",
    "One evening a raccoon climbed into the trash. It made a huge mess. The big dogs slept right through it.",
    "Peanut did not sleep. \"YIP! YIP! YIP!\" she barked, until the lights came on and the raccoon ran away.",
    "The next day the big dogs did not laugh. \"You have a very brave bark,\" they said. Peanut agreed.",
    nullptr}, "What did Peanut chase away?", {"A cat", "A raccoon", "A fox"}, 1},
  {"Grandpa's Garden", C_GREEN, 4, 2, {
    "Grandpa grew tomatoes, beans, and one enormous pumpkin. Waffles the dog guarded them all.",
    "Waffles had a rule. No rabbits. No crows. No squirrels. Especially no squirrels.",
    "One day Waffles found a squirrel eating a bean. Waffles took a deep breath. The squirrel looked hungry.",
    "Waffles rolled one bean toward the squirrel with his nose. \"Just one,\" he said. The squirrel took it and bowed.",
    "After that, the squirrel visited every day. It always took one bean. And it never touched the pumpkin.",
    nullptr}, "What did Waffles share?", {"A tomato", "A bean", "The pumpkin"}, 1},
  {"Night Light", C_LAVENDER, 1, 1, {
    "Poppy did not like the dark. At bedtime she hid under the blanket and shivered.",
    "\"Come outside,\" said Grandma one night. Poppy peeked out. The garden was full of tiny lights.",
    "Fireflies! Hundreds of them. They blinked on and off, like little stars that had come down to play.",
    "Poppy followed one firefly all around the yard. It never once flew away from her.",
    "Now, when it gets dark, Poppy remembers the fireflies. The dark is where the little lights live.",
    nullptr}, "What did Poppy see in the garden?", {"Fireflies", "Bats", "The moon"}, 0},
  {"The Sock Thief", C_PINK, 0, 2, {
    "Someone was stealing socks. One sock every day. Detective Noodle was on the case.",
    "Noodle sniffed the hall. Noodle sniffed the stairs. The trail led straight to... the laundry basket.",
    "Inside the basket, on a pile of socks, sat a very small kitten. It was asleep. It looked cozy.",
    "Noodle did not bark. He fetched his own blanket and tucked it around the kitten.",
    "Case closed, thought Noodle. The socks were not stolen. They were just being used as a bed.",
    nullptr}, "Who was taking the socks?", {"A kitten", "A mouse", "Noodle"}, 0},
  {"Race to the Tree", C_WATER, 2, 2, {
    "Dash was fast. Dash was the fastest dog at the park. Everyone knew it, mostly because Dash said so.",
    "One day a slow old dog named Turtle said, \"Race you to the big tree.\" Everyone laughed. Dash grinned.",
    "Dash zoomed off. Halfway there he stopped to sniff a stick. Then a butterfly. Then a very good puddle.",
    "When Dash looked up, Turtle was sitting under the tree. Turtle had not stopped once.",
    "\"Slow and steady,\" said Turtle. Dash flopped in the shade beside her. Sniffing, he decided, was also winning.",
    nullptr}, "Why did Dash lose the race?", {"He fell down", "He kept stopping", "Turtle cheated"}, 1},
  {"The Birthday Bone", C_ROSE, 3, 2, {
    "It was Maple's birthday. She got a bone with a bow on it. The best bone in the world.",
    "Maple wanted to keep it forever. She dug a hole and buried it deep. Now it was safe.",
    "But Maple could not stop thinking about the bone. Was it lonely? Was it cold? She dug it up to check.",
    "Her friend Rocket came by. Rocket had no bone at all. Maple looked at her bone. Then she looked at Rocket.",
    "They chewed it together on the porch until the sun went down. It was the best birthday in the world.",
    nullptr}, "What did Maple do with the bone?", {"Buried it forever", "Shared with Rocket", "Lost it"}, 1},
  {"Captain Fluff", C_SKY, 2, 2, {
    "Captain Fluff sailed the bathtub sea. Her ship was a plastic bowl. Her crew was one rubber duck.",
    "\"Storm ahead!\" cried Fluff, as the tap turned on. Waves crashed. The duck was very brave.",
    "A giant sea monster appeared. It had five long arms. It was, in fact, a hand holding soap.",
    "Fluff fought the monster with all her might. There were bubbles everywhere. The duck cheered.",
    "At last the sea grew calm. Fluff was clean, fluffy, and victorious. She was wrapped in a towel, like a hero.",
    nullptr}, "What was the sea monster really?", {"A whale", "A hand with soap", "The duck"}, 1},
  {"The Very Long Nap", C_MINT, 1, 1, {
    "Clover was a sleepy dog. She napped in the sun. She napped in the shade. She napped in the middle of breakfast.",
    "One afternoon Clover fell asleep under the apple tree. She dreamed she was a bird, flying over the hills.",
    "In her dream she flew past the school, past the pond, all the way to the sea. The sea sang a quiet song.",
    "When Clover woke up, an apple had landed right beside her nose. \"Thank you, tree,\" she said, and ate it.",
    "Clover decided that a nap with a dream in it was the best kind of nap. Then she yawned, and took another one.",
    nullptr}, "What did Clover dream she was?", {"A fish", "A bird", "A cloud"}, 1},
  {"Bingo and the Bees", C_YELLOW, 4, 2, {
    "Bingo loved honey. He loved it on toast. He loved it on his paws. He even loved it on his nose.",
    "The honey came from the bees at the end of the garden. Bingo was a little afraid of bees. They buzzed so loudly.",
    "One day Bingo saw a bee stuck in a spider web. It was buzzing for help. Bingo took a deep breath.",
    "Very gently, he pulled the web away with his teeth. The bee flew free. It circled his head three times, like a thank-you.",
    "That summer the bees made more honey than ever. And one small bee always landed on Bingo's nose to say hello.",
    nullptr}, "Where was the bee stuck?", {"In a jar", "In a spider web", "In a flower"}, 1},
  {"Pickle Learns to Swim", C_WATER, 2, 1, {
    "All the dogs at the lake loved to swim. All except Pickle. Pickle sat on the shore and watched.",
    "\"The water is too wet,\" said Pickle. \"And too deep. And too splashy.\" Her friend Otto just laughed and dived in.",
    "Then a wave took Otto's favorite stick. It floated away, farther and farther. Otto could not reach it.",
    "Pickle did not think. She jumped in. Splash! Her paws paddled all by themselves. She grabbed the stick.",
    "\"You can swim!\" cried Otto. Pickle shook the water from her ears. \"I suppose I can,\" she said, and swam back for more.",
    nullptr}, "Why did Pickle jump in?", {"To save the stick", "To cool off", "Otto pushed her"}, 0},
  {"The Midnight Snack", C_PLUM, 5, 2, {
    "Late at night, when the house was dark, Tater heard a sound. Crunch. Crunch. Someone was in the kitchen.",
    "Tater crept down the hall. His nails went click, click on the floor. The crunching stopped. Then started again.",
    "He peeked around the corner. There, in the light of the fridge, stood Grandpa, eating cookies from the jar.",
    "Grandpa looked at Tater. Tater looked at Grandpa. \"Our secret?\" whispered Grandpa. He broke a cookie in half.",
    "Every night after that, Tater and Grandpa shared one cookie. Nobody ever found out. Except you, of course.",
    nullptr}, "Who was in the kitchen?", {"A mouse", "Grandpa", "A burglar"}, 1},
  {"Juniper's Map", C_LEAF, 7, 2, {
    "Juniper found a piece of paper in the park. It had lines and an X. \"A treasure map!\" she thought.",
    "She followed the first line to the big oak. She followed the next one to the fountain. Then to the red bench.",
    "At the X, Juniper dug and dug. She found a rock. A very ordinary rock. She was a little disappointed.",
    "Then a girl came running. \"You found my rock! It is my lucky rock. I hid it so no one would take it.\"",
    "The girl gave Juniper a biscuit and a hug. Juniper decided the map had led to treasure after all.",
    nullptr}, "What did Juniper find at the X?", {"Gold coins", "A bone", "A lucky rock"}, 2},
  {"Winter Coat", C_SKY, 2, 1, {
    "Nutmeg had short fur. When winter came, she shivered on every walk. Brr, brr, brr.",
    "Grandma got out her knitting needles. Click, clack, click. Day after day, a little sweater grew.",
    "It was green with yellow stripes. Nutmeg was not sure. Dogs do not wear sweaters, do they?",
    "On the first snowy day, Nutmeg put it on. She was warm! She ran through the snow like a fuzzy green rocket.",
    "The other dogs stared. Then they went home and asked for sweaters too. By spring, the whole street was stripy.",
    nullptr}, "What color was Nutmeg's sweater?", {"Green and yellow", "Red", "Blue with dots"}, 0},
#if __has_include("content_level3_books.h")
#include "content_level3_books.h"
#endif
};
static const int NUM_BOOKS = (int)(sizeof(BOOKS) / sizeof(BOOKS[0]));

struct WordClue { const char* word; const char* clue; };
static const WordClue WORDS[] = {
  {"DOG", "Your best friend"}, {"BONE", "A dog's favorite chew"}, {"BALL", "Round toy to fetch"}, {"PARK", "Where dogs run and play"},
  {"BOOK", "You read this"}, {"READ", "What you do with a book"}, {"BARK", "A dog's loud voice"}, {"TAIL", "It wags when happy"},
  {"PAW", "A dog's foot"}, {"PUPPY", "A baby dog"}, {"TREAT", "A yummy reward"}, {"LEASH", "Holds a dog on a walk"},
  {"FETCH", "Go get the ball!"}, {"SLEEP", "What you do at night"}, {"HAPPY", "Feeling glad"}, {"JUMP", "Spring up high"},
  {"RUN", "Move very fast"}, {"DIG", "Make a hole"}, {"MUD", "Wet dirt"}, {"BATH", "Getting clean in water"},
  {"SOAP", "Makes bubbles"}, {"SUN", "Warm and bright above"}, {"RAIN", "Drops from the clouds"}, {"STAR", "Twinkles at night"},
  {"MOON", "Shines at night"}, {"CAKE", "Sweet birthday food"}, {"APPLE", "Red or green fruit"}, {"MILK", "White drink from cows"},
  {"STORY", "A tale you read"}, {"PAGE", "One sheet of a book"}, {"HERO", "Someone brave"}, {"BRAVE", "Not afraid"},
  {"KIND", "Nice to others"}, {"SMILE", "A happy face"}, {"NOSE", "A dog sniffs with it"}, {"EARS", "For hearing"},
  {"SNOW", "Cold, white and soft"}, {"WIND", "Air that blows"}, {"TREE", "Has leaves and a trunk"}, {"BIRD", "It sings and flies"},
  {"FROG", "Green and jumps"}, {"DUCK", "Says quack"}, {"FISH", "Swims in water"}, {"CAT", "Says meow"},
  {"HOUSE", "Where you live"}, {"DOOR", "You open it to go in"}, {"BED", "You sleep in it"}, {"LAMP", "Gives light"},
  {"FRIEND", "Someone you like"}, {"FAMILY", "Mom, dad, sister"}, {"GIFT", "A present"}, {"PARTY", "Cake, games, friends"},
  {"TRICK", "Sit! Shake! Roll over!"}, {"SIT", "Bottom on the floor"}, {"SPIN", "Turn around and around"}, {"DANCE", "Move to music"},
  {"WATER", "You drink it"}, {"DREAM", "A story while you sleep"}, {"CLOUD", "Fluffy and in the sky"}, {"GRASS", "Green and soft to lie on"},
  {"BLUE", "Color of the sky"}, {"GREEN", "Color of grass"}, {"PINK", "A light red"}, {"GOLD", "Shiny yellow metal"},
  {"SEVEN", "One more than six"}, {"TEN", "Fingers on two hands"}, {"WINTER", "The coldest season"}, {"SUMMER", "The hottest season"},
  {"BEACH", "Sand by the sea"}, {"BOAT", "Floats on water"}, {"TRAIN", "Runs on tracks"}, {"MAGIC", "Wands and spells"},
  {"QUEEN", "She wears a crown"}, {"DRAGON", "Breathes fire"}, {"CASTLE", "Where a king lives"}, {"PIRATE", "Sails and says arr"},
#if __has_include("content_level3_words.h")
#include "content_level3_words.h"
#endif
};
static const int NUM_WORDS = (int)(sizeof(WORDS) / sizeof(WORDS[0]));

static const char* const TRICK_NAMES[8] = {"Sit", "Shake", "Speak", "Spin", "Roll Over", "Play Dead", "Beg", "Dance"};
static const char* const HAT_NAMES[8] = {"None", "Red Bow", "Party Hat", "Glasses", "Bandana", "Crown", "Wizard Hat", "Flower"};
static const char* const STICKER_NAMES[16] = {
  "First Meal", "First Story", "Bookworm", "Library Card", "Bone Catcher", "Super Speller", "One Week", "Two Weeks",
  "Trick Star", "Show Dog", "Squeaky Clean", "Best Friends", "Early Bird", "Party Time", "All Grown Up", "Hat Collector"};
static const char* const PET_NAME_IDEAS[9] = {"Biscuit", "Pip", "Mochi", "Luna", "Waffles", "Peanut", "Maple", "Noodle", "Cocoa"};
